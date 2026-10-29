import json
import zmq

from .base import CommsBackend
from .message import SimMessage


class ZmqComms(CommsBackend):
    """
    ZMQ communication backend.

    One instance should be created per simulator component.
    Do not share one ZMQ socket object across all subsystem threads.
    """

    WORD_COMMAND_TOPICS = {
        "AMN": (
            "FOCUS", "MIRROR_COVER", "MOVE_FOLD", "DARKSLIDE",
            "AGFILTER", "AGFOCUS", "AGRADIAL",
        ),
        "ALT": ("ALTITUDE",),
    }

    @classmethod
    def command_topics(cls, owner: str):
        owner = owner.upper()
        tracking = (f"tracking.{owner}", "tracking.CANCEL") if owner in {"ALT", "AZ", "CAS"} else ()
        return (f"cmd.{owner}",) + tracking + tuple(
            f"cmd.{command}" for command in cls.WORD_COMMAND_TOPICS.get(owner, ())
        )


    def __init__(self, bus_config, owner: str, role: str):
        super().__init__(bus_config, owner, role)

        self.ctx = None

        self.cmd_sub = None
        self.result_pub = None
        self.status_pub = None
        self.hb_pub = None

        self.scc_hb_sub = None
        self.scc_pub = None

    def start(self) -> None:
        self.ctx = zmq.Context.instance()

        if self.role == "subsystem":
            self._start_subsystem_sockets()
        elif self.role == "scc":
            self._start_scc_sockets()
        else:
            raise ValueError(f"Unsupported ZMQ comms role: {self.role}")

    def _start_subsystem_sockets(self) -> None:
        self.cmd_sub = self.ctx.socket(zmq.SUB)
        self.cmd_sub.connect(self.bus.cmd_endpoint)
        for topic in self.command_topics(self.owner):
            self.cmd_sub.setsockopt_string(zmq.SUBSCRIBE, topic)

        self.result_pub = self.ctx.socket(zmq.PUB)
        self.result_pub.connect(self.bus.result_endpoint)

        self.status_pub = self.ctx.socket(zmq.PUB)
        self.status_pub.connect(self.bus.status_endpoint)

        self.hb_pub = self.ctx.socket(zmq.PUB)
        self.hb_pub.connect(self.bus.heartbeat_endpoint)

    def _start_scc_sockets(self) -> None:
        self.scc_hb_sub = self.ctx.socket(zmq.SUB)
        self.scc_hb_sub.bind(self.bus.heartbeat_endpoint)
        self.scc_hb_sub.setsockopt_string(zmq.SUBSCRIBE, "heartbeat.")

        self.scc_pub = self.ctx.socket(zmq.PUB)
        self.scc_pub.connect(self.bus.scc_to_tcs_endpoint)

    def stop(self) -> None:
        sockets = [
            self.cmd_sub,
            self.result_pub,
            self.status_pub,
            self.hb_pub,
            self.scc_hb_sub,
            self.scc_pub,
        ]

        for sock in sockets:
            if sock is not None:
                sock.close(linger=0)

    def _send_two_frame(self, socket, message: SimMessage) -> None:
        if socket is None:
            raise RuntimeError(
                f"ZMQ socket is not initialized for owner={self.owner}, role={self.role}"
            )

        socket.send_string(message.topic, flags=zmq.SNDMORE)
        socket.send_string(json.dumps(message.body))

    def send_result(self, message: SimMessage) -> None:
        self._send_two_frame(self.result_pub, message)

    def send_status(self, message: SimMessage) -> None:
        self._send_two_frame(self.status_pub, message)

    def send_heartbeat(self, message: SimMessage) -> None:
        self._send_two_frame(self.hb_pub, message)

    def send_summary(self, message: SimMessage) -> None:
        self._send_two_frame(self.scc_pub, message)

    def receive_command(self, timeout_ms: int | None = None) -> SimMessage | None:
        if self.cmd_sub is None:
            raise RuntimeError(
                f"Command socket is not initialized for owner={self.owner}"
            )

        if timeout_ms is not None:
            poller = zmq.Poller()
            poller.register(self.cmd_sub, zmq.POLLIN)
            events = dict(poller.poll(timeout_ms))

            if self.cmd_sub not in events:
                return None

        topic = self.cmd_sub.recv_string()
        body_str = self.cmd_sub.recv_string()
        body = json.loads(body_str)

        return SimMessage(topic=topic, body=body)

    def receive_heartbeat(self, timeout_ms: int | None = None) -> SimMessage | None:
        if self.scc_hb_sub is None:
            raise RuntimeError("SCC heartbeat socket is not initialized.")

        if timeout_ms is not None:
            poller = zmq.Poller()
            poller.register(self.scc_hb_sub, zmq.POLLIN)
            events = dict(poller.poll(timeout_ms))

            if self.scc_hb_sub not in events:
                return None

        topic = self.scc_hb_sub.recv_string()
        body_str = self.scc_hb_sub.recv_string()
        body = json.loads(body_str)

        return SimMessage(topic=topic, body=body)
