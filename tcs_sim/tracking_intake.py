"""Coherent Service 8 tracking intake for the ZMQ simulator profile."""

from collections import OrderedDict
import threading
import time

from utils import log

SCHEMA = "tcs.tracking.service8.v1"
RESPONSE_SCHEMA = "tcs.tracking.service8.response.v1"
AXIS_IDS = {"ALT": 40, "AZ": 34, "CAS": 46}
UINT32_MAX = (1 << 32) - 1
MAX_PENDING = 64
MAX_ACCEPTED = 256


def _uint(value, maximum=UINT32_MAX):
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= maximum:
        raise ValueError("expected unsigned integer")
    return value


def signed_mas_degrees(word):
    word = _uint(word)
    signed = word if word < (1 << 31) else word - (1 << 32)
    return signed / 3600000.0


class TrackingIntake:
    def __init__(self):
        self.lock = threading.RLock()
        self.axes = {}
        self.pending = OrderedDict()
        self.pending_identity = {}
        self.accepted = OrderedDict()
        self.retired = {}
        self.active_session = None
        self.active_generation = None
        self.last_sequence = None
        self.rejections = 0
        self.sequence_gaps = 0

    def register(self, subsystem):
        if subsystem.name in AXIS_IDS:
            with self.lock:
                self.axes[subsystem.name] = subsystem

    def reset(self):
        with self.lock:
            self.axes.clear()
            self.pending.clear()
            self.pending_identity.clear()
            self.accepted.clear()
            self.retired.clear()
            self.active_session = None
            self.active_generation = None
            self.last_sequence = None
            self.rejections = 0
            self.sequence_gaps = 0

    @staticmethod
    def _validate(topic, body):
        if not isinstance(body, dict) or body.get("schema") != SCHEMA:
            raise ValueError("bad schema")
        axis = body.get("axis")
        if axis not in AXIS_IDS or topic != f"tracking.{axis}":
            raise ValueError("axis/topic mismatch")
        if not isinstance(body.get("session"), str) or not body["session"]:
            raise ValueError("bad session")
        process_id = _uint(body.get("process_id"), (1 << 53) - 1)
        generation = _uint(body.get("generation"), (1 << 53) - 1)
        sequence = _uint(body.get("demand_set_sequence"), (1 << 53) - 1)
        if (_uint(body.get("DestId")) != AXIS_IDS[axis] or
                _uint(body.get("Class")) != 1 or _uint(body.get("Service")) != 8):
            raise ValueError("bad route or CIL header")
        words = [_uint(body.get(f"param{i}")) for i in range(8)]
        if words[:4] != [0, 0, 4, 0] or words[7] != 0:
            raise ValueError("bad Service 8 constants")
        if words[5] > 0x7FFFFFFF or words[6] > 999999999:
            raise ValueError("bad execution timestamp")
        packet_sequence = body.get("packet_sequence")
        if packet_sequence is not None:
            _uint(packet_sequence, (1 << 53) - 1)
        return {
            "axis": axis, "session": body["session"], "process_id": process_id,
            "generation": generation, "sequence": sequence,
            "words": words, "degrees": signed_mas_degrees(words[4]),
            "packet_sequence": packet_sequence,
        }

    def ingest(self, subsystem, topic, body):
        if topic == "tracking.CANCEL":
            return self.cancel(body)
        try:
            packet = self._validate(topic, body)
        except (TypeError, ValueError) as exc:
            self.rejections += 1
            log("TRACKING", f"Rejected {topic}: {exc}")
            return False
        identity = (packet["session"], packet["process_id"],
                    packet["generation"], packet["sequence"])
        key = identity + (packet["words"][5], packet["words"][6])
        with self.lock:
            if packet["generation"] < self.retired.get(packet["session"], 0):
                self.rejections += 1
                return False
            if self.active_session == packet["session"]:
                if packet["generation"] < self.active_generation:
                    self.rejections += 1
                    return False
                if (packet["generation"] == self.active_generation and
                        self.last_sequence is not None and
                        packet["sequence"] < self.last_sequence):
                    self.rejections += 1
                    return False
            accepted = self.accepted.get(key)
            if accepted is not None:
                if accepted.get(packet["axis"]) == packet:
                    subsystem.queue_tracking_response(self._response(packet))
                    return True
                self.rejections += 1
                return False
            old_key = self.pending_identity.get(identity)
            if old_key is not None and old_key != key:
                self.rejections += 1
                self.pending.pop(old_key, None)
                self.pending_identity.pop(identity, None)
                return False
            group = self.pending.setdefault(key, {})
            old = group.get(packet["axis"])
            if old is not None:
                if old != packet:
                    self.rejections += 1
                    return False
                return True
            group[packet["axis"]] = packet
            self.pending_identity[identity] = key
            while len(self.pending) > MAX_PENDING:
                dropped_key, _ = self.pending.popitem(last=False)
                self.pending_identity.pop(dropped_key[:4], None)
                self.rejections += 1
            if set(group) != set(AXIS_IDS) or set(self.axes) != set(AXIS_IDS):
                return True
            is_start = (self.active_session != packet["session"] or
                        self.active_generation != packet["generation"])
            if self.active_session is not None and self.active_session != packet["session"]:
                self.retired[self.active_session] = (1 << 53) - 1
            if (not is_start and self.last_sequence is not None and
                    packet["sequence"] > self.last_sequence + 1):
                self.sequence_gaps += packet["sequence"] - self.last_sequence - 1
            timestamp = packet["words"][5] + packet["words"][6] / 1_000_000_000.0
            trajectory_id = f"{packet['session']}:{packet['generation']}"
            commit_lock = next(iter(self.axes.values())).trajectory_commit_lock
            with commit_lock:
                for axis_name in ("ALT", "AZ", "CAS"):
                    axis_packet = group[axis_name]
                    if not self.axes[axis_name].accept_tracking_point(
                            timestamp, axis_packet["degrees"], trajectory_id,
                            packet["sequence"], is_start):
                        self.rejections += 1
                        return False
                for axis_obj in self.axes.values():
                    axis_obj.activate_tracking_set(trajectory_id, is_start)
            self.active_session = packet["session"]
            self.active_generation = packet["generation"]
            self.last_sequence = packet["sequence"]
            self.pending.pop(key, None)
            self.pending_identity.pop(identity, None)
            self.accepted[key] = dict(group)
            while len(self.accepted) > MAX_ACCEPTED:
                self.accepted.popitem(last=False)
            for axis_name, axis_packet in group.items():
                self.axes[axis_name].queue_tracking_response(self._response(axis_packet))
            return True

    @staticmethod
    def _response(packet):
        sec, nsec = divmod(time.time_ns(), 1_000_000_000)
        body = {
            "schema": RESPONSE_SCHEMA, "session": packet["session"],
            "process_id": packet["process_id"], "generation": packet["generation"],
            "demand_set_sequence": packet["sequence"], "axis": packet["axis"],
            "SourceId": AXIS_IDS[packet["axis"]], "DestId": 17,
            "Class": 2, "Service": 8, "param0": 0x002A0008,
            "param1": 0x00010000, "param2": 4, "param3": 0,
            "param4": packet["words"][4], "param5": packet["words"][5],
            "param6": packet["words"][6], "param7": 0x18,
            "response_timestamp": {"seconds": sec, "nanoseconds": nsec},
            "accepted": True,
        }
        if packet["packet_sequence"] is not None:
            body["packet_sequence"] = packet["packet_sequence"]
        return body

    def cancel(self, body):
        try:
            if not isinstance(body, dict) or body.get("schema") != "tcs.tracking.cancel.v1":
                raise ValueError("bad cancellation schema")
            session = body.get("session")
            if not isinstance(session, str) or not session:
                raise ValueError("bad cancellation session")
            floor = _uint(body.get("retire_before_generation"), (1 << 53) - 1)
        except (TypeError, ValueError) as exc:
            self.rejections += 1
            log("TRACKING", f"Rejected cancellation: {exc}")
            return False
        with self.lock:
            self.retired[session] = max(floor, self.retired.get(session, 0))
            for key in list(self.pending):
                if key[0] == session and key[2] < floor:
                    self.pending.pop(key, None)
                    self.pending_identity.pop(key[:4], None)
            if self.active_session == session and self.active_generation < floor:
                self._retire_locked()
        return True

    def manual_takeover(self, reason):
        with self.lock:
            if self.active_session is not None:
                self.retired[self.active_session] = max(
                    self.retired.get(self.active_session, 0), self.active_generation + 1)
            self._retire_locked()
            log("TRACKING", f"Tracking retired by {reason}")

    def _retire_locked(self):
        self.pending.clear()
        self.pending_identity.clear()
        if self.axes:
            commit_lock = next(iter(self.axes.values())).trajectory_commit_lock
            with commit_lock:
                for axis_obj in self.axes.values():
                    axis_obj.clear_tracking_input()
        self.active_session = None
        self.active_generation = None
        self.last_sequence = None


_INTAKE = TrackingIntake()


def get_tracking_intake():
    return _INTAKE
