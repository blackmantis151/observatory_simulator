import time
import threading
import bisect
import csv
import os
from utils import now_ts, log
from comms.message import SimMessage


class SubsystemBase:
    """
    Base class for all subsystems (ALT, AZ, CAS).

    Handles:
      - ZeroMQ socket setup
      - Heartbeat loop
      - Command receive loop
      - Sending ACK / COMPLETED / ERROR / STATUS / HEARTBEAT

    Subclasses must implement:
      - handle_command(self, msg: dict)
    """

    def __init__(self, name: str, comms):
        self.name = name.upper()
        self.comms = comms
        self.running = True
        self.disturbance = None
        
        # --- NEW: File Paths & Thread Safety ---
        self.traj_file = f"trajectory_{self.name.lower()}.csv"
        self.status_file = f"status_{self.name.lower()}.csv"
        self.data_lock = threading.Lock()
        self.traj_timestamps = []
        self.traj_positions = []
        self.trajectory_id = None
        self.trajectory_revision = -1
        self.trajectory_starved = False

        # --- NEW: Initialize Files & Recover State ---
        self._init_files()
        
        # Generic state; subclasses can use/extend
        self.state = "IDLE"
        self.current_pos_deg = self._recover_last_position() # <--- CHANGED FROM 0.0
        self.target_pos_deg = 0.0

    # ---------- Generic send helpers ----------

    def send_result(self, msg_type: str, cmd_id=None, **extra):
        topic = f"result.{self.name}"
        body = {
            "msg_type": msg_type,
            "source": self.name,
            "timestamp": now_ts(),
            "state": self.state,
            "current_pos_deg": self.reported_pos_deg(),
            "target_pos_deg": self.target_pos_deg,
        }

        if cmd_id is not None:
            body["cmd_id"] = cmd_id

        body.update(extra)

        self.comms.send_result(SimMessage(topic=topic, body=body))
        log(self.name, f"SENT on {topic}: {body}")

    def send_status(self, cmd_id=None, **extra):
        topic = f"status.{self.name}"
        body = {
            "msg_type": "STATUS",
            "source": self.name,
            "timestamp": time.time(),
            "state": self.state,
            "current_pos_deg": self.reported_pos_deg(),
            "target_pos_deg": self.target_pos_deg,
        }

        if cmd_id is not None:
            body["cmd_id"] = cmd_id

        body.update(extra)

        self.comms.send_status(SimMessage(topic=topic, body=body))
        log(self.name, f"SENT on {topic}: {body}")

    def send_heartbeat(self):
        topic = f"heartbeat.{self.name}"
        body = {
            "msg_type": "HEARTBEAT",
            "source": self.name,
            "timestamp": now_ts(),
            "state": self.state,
            "current_pos_deg": self.reported_pos_deg(),
        }

        self.comms.send_heartbeat(SimMessage(topic=topic, body=body))

    def reported_pos_deg(self):
        if self.disturbance is None or self.name not in {"ALT", "AZ"}:
            return self.current_pos_deg
        tracking = bool(getattr(self, "tracking_active", False))
        x_arcsec, y_arcsec = self.disturbance.sample(tracking)
        altitude = self.current_pos_deg if self.name == "ALT" else 45.0
        alt_offset, az_offset = self.disturbance.axis_offsets(
            x_arcsec, y_arcsec, altitude
        )
        return self.current_pos_deg + (
            alt_offset if self.name == "ALT" else az_offset
        )
        
        
        
    # ---------- File & Math Helpers (NEW) ----------

    def _init_files(self):
        """Ensures CSV files exist with correct headers."""
        # Trajectory File
        if not os.path.exists(self.traj_file):
            with open(self.traj_file, 'w', newline='') as f:
                csv.writer(f).writerow(['timestamp', 'position'])
        
        # Status File (Append-only)
        if not os.path.exists(self.status_file):
            with open(self.status_file, 'w', newline='') as f:
                csv.writer(f).writerow(['timestamp', 'position'])

    def _recover_last_position(self):
        """Reads the last line of the status CSV to recover state on restart."""
        default_pos = 90.0 if self.name == 'ALT' else 0.0
        
        if not os.path.exists(self.status_file):
            return default_pos

        try:
            with open(self.status_file, "r") as f:
                # Efficiently seek to the end
                f.seek(0, os.SEEK_END)
                if f.tell() < 5: return default_pos # Empty file check
                
                pos = f.tell() - 2
                while pos > 0 and f.read(1) != "\n":
                    pos -= 1
                    f.seek(pos, os.SEEK_SET)
                last_line = f.readline().strip()
                
                parts = last_line.split(',')
                if len(parts) >= 2 and parts[0].replace('.','',1).isdigit():
                    recovered = float(parts[1])
                    log(self.name, f"Recovered Position from Disk: {recovered}")
                    return recovered
        except Exception as e:
            log(self.name, f"Error recovering state: {e}")
        
        return default_pos
    
    
    # Add this inside class SubsystemBase in subsystem_base.py
    def clear_trajectory_data(self):
        """
        Wipes the existing trajectory buffer AND FILE.
        CRITICAL FIX: This writes the HEADER back to the file so pandas can read it.
        """
        with self.data_lock:
            # Wipe memory (if used)
            self.traj_timestamps = []
            self.traj_positions = []
            
            # Wipe File and Re-write Header
            try:
                with open(self.traj_file, 'w', newline='') as f:
                    csv.writer(f).writerow(['timestamp', 'position'])
                log(self.name, "Trajectory file cleared and header reset.")
            except Exception as e:
                log(self.name, f"Error clearing trajectory file: {e}")
            
    

    def append_trajectory_data(self, timestamps, positions):
        """Thread-safe method to append new trajectory packets."""
        try:
            with self.data_lock:
                with open(self.traj_file, 'a', newline='') as f:
                    writer = csv.writer(f)
                    for t, p in zip(timestamps, positions):
                        writer.writerow([t, p])
            return True
        except Exception as e:
            log(self.name, f"Error appending trajectory: {e}")
            return False

    def update_trajectory_data(self, params):
        """Atomically start or replace the future part of a rolling trajectory."""
        timestamps = params.get("timestamp") or []
        positions = params.get("position") or []
        trajectory_id = params.get("trajectory_id")
        revision = params.get("revision", 0)
        mode = str(params.get("mode", "START")).upper()
        effective = float(params.get("effective_from", timestamps[0] if timestamps else 0))
        if (not timestamps or len(timestamps) != len(positions) or
                any(float(timestamps[i]) >= float(timestamps[i + 1])
                    for i in range(len(timestamps) - 1))):
            return False
        try:
            pairs = [(float(t), float(p)) for t, p in zip(timestamps, positions)]
            with self.data_lock:
                if trajectory_id == self.trajectory_id and revision <= self.trajectory_revision:
                    log(self.name, f"Ignoring stale trajectory revision {revision}")
                    return True
                kept = [] if mode == "START" or trajectory_id != self.trajectory_id else [
                    (t, p) for t, p in zip(self.traj_timestamps, self.traj_positions)
                    if t < effective]
                if self.name == "AZ":
                    reference = kept[-1][1] if kept else pairs[0][1]
                    unwrapped = []
                    for timestamp, position in pairs:
                        while position - reference > 180.0:
                            position -= 360.0
                        while position - reference < -180.0:
                            position += 360.0
                        unwrapped.append((timestamp, position))
                        reference = position
                    pairs = unwrapped
                merged = kept + [(t, p) for t, p in pairs if t >= effective]
                merged = [(t, p) for t, p in merged if t >= time.time() - 2.0]
                self.traj_timestamps = [t for t, _ in merged]
                self.traj_positions = [p for _, p in merged]
                self.trajectory_id = trajectory_id
                self.trajectory_revision = revision
                self.trajectory_starved = False
                with open(self.traj_file, 'w', newline='') as f:
                    writer = csv.writer(f)
                    writer.writerow(['timestamp', 'position'])
                    writer.writerows(merged)
            return True
        except (TypeError, ValueError, OSError) as exc:
            log(self.name, f"Error updating trajectory: {exc}")
            return False

    def log_current_state_to_disk(self, timestamp, position):
        """Persist status to CSV (Distinct from network send_status)."""
        try:
            with open(self.status_file, 'a', newline='') as f:
                csv.writer(f).writerow([f"{timestamp:.3f}", f"{position:.4f}"])
        except Exception:
            pass 

    def get_rolling_target(self, query_time):
        """Return an interpolated in-memory target, holding safely on starvation."""
        with self.data_lock:
            times = tuple(self.traj_timestamps)
            positions = tuple(self.traj_positions)
        if not times:
            return None
        idx = bisect.bisect_left(times, query_time)
        if idx == 0:
            return positions[0]
        if idx >= len(times):
            if query_time - times[-1] > 1.0 and not self.trajectory_starved:
                self.trajectory_starved = True
                log(self.name, "Trajectory buffer starved; holding final safe target")
            return positions[-1]
        t1, t2 = times[idx - 1], times[idx]
        p1, p2 = positions[idx - 1], positions[idx]
        return p1 if t2 == t1 else p1 + (p2 - p1) * (query_time - t1) / (t2 - t1)

    # ---------- Background loops ----------

    def heartbeat_loop(self):
        while self.running:
            self.send_heartbeat()
            self.send_status(trajectory_starved=self.trajectory_starved)
            time.sleep(1.0)

    # ---------- To be overridden ----------

    def handle_command(self, msg: dict):
        cmd_id = msg.get("cmd_id")
        command = (msg.get("command") or "").upper()
        params = msg.get("params", {}) or {}

        handler = getattr(self, "command_table", {}).get(command)
        if not handler:
            self.send_result("ERROR", cmd_id=cmd_id, error_code="NETUNK")
            log(self.name, f"Unknown command '{command}'")
            return

        handler(cmd_id, params)

    # ---------- Main loop ----------

    def run(self):
        log(self.name, f"{self.__class__.__name__} starting, listening for commands")

        hb_thread = threading.Thread(target=self.heartbeat_loop, daemon=True)
        hb_thread.start()

        while self.running:
            try:
                message = self.comms.receive_command(timeout_ms=1000)
            except KeyboardInterrupt:
                break

            if message is None:
                continue

            topic = message.topic
            msg = message.body

            log(self.name, f"RECV on {topic}: {msg}")

            if msg.get("msg_type") != "CMD":
                log(self.name, "Ignoring non-CMD message")
                continue

            self.handle_command(msg)

        self.running = False
        log(self.name, f"{self.__class__.__name__} shutting down.")
