# scc_sim.py
#
# Supervisory Control Computer (SCC) simulator.
#
# - SUB on heartbeat bus: receives JSON heartbeats from ALT, AZ, CAS
# - PUB to TCS bus: sends TELESCOPESUMMARY messages
# - Maintains an in memory dict self.subsystems as the live state
# - Logs snapshots to a CSV file using pandas
#   CSV columns: timestamp, ALT, AZ, CAS, overall
#   Newest snapshot is always the first data row

import time
import threading
import os

import pandas as pd

from utils import now_ts, log
from comms.message import SimMessage

# CSV log path
LOG_CSV = "/home/harshit/Observatory_Stimulator/Observatory_Stimulator/tcs_sim/scc_log.csv"


class SCCSimulator:
    def __init__(self, comms):
        self.comms = comms

        # Live state of subsystems in memory
        # name -> { "state": str, "last_heartbeat": int, "current_pos_deg": float }
        self.subsystems = {}

        self.running = True

        # Ensure CSV exists with correct header
        self._init_csv()

        log("SCC", f"SCC will log CSV snapshots to {LOG_CSV}")

    # ------------- CSV and state helpers -------------

    def _init_csv(self):
        """Create CSV with header if it does not exist."""
        if not os.path.exists(LOG_CSV):
            df = pd.DataFrame(
                columns=["timestamp", "ALT", "AZ", "CAS", "overall"]
            )
            df.to_csv(LOG_CSV, index=False)

    def _get_state(self, name: str) -> str:
        """Return latest state string for the given subsystem."""
        info = self.subsystems.get(name)
        if info is None:
            return "UNKNOWN"
        return info.get("state", "UNKNOWN")

    def compute_overall_state(self) -> str:
        """
        Compute overall telescope state from subsystem states.

        Rule:
        - If any subsystem is FAULT -> overall FAULT
        - Else if any subsystem is not IDLE or UNKNOWN -> BUSY
        - Else -> IDLE (or UNKNOWN if nothing known)
        """
        if not self.subsystems:
            return "UNKNOWN"

        states = [self._get_state(n) for n in self.subsystems.keys()]

        if any(s == "FAULT" for s in states):
            return "FAULT"
        if any(s not in ("IDLE", "UNKNOWN") for s in states):
            return "BUSY"
        return "IDLE"

    def log_snapshot_pandas(self):
        """
        Log one snapshot row at the top of the CSV using pandas.

        Row layout:
            timestamp, ALT, AZ, CAS, overall

        Newest row appears immediately after the header.
        """
        ts = now_ts()
        alt_state = self._get_state("ALT")
        az_state = self._get_state("AZ")
        cas_state = self._get_state("CAS")
        overall = self.compute_overall_state()

        new_row = pd.DataFrame(
            [{
                "timestamp": ts,
                "ALT": alt_state,
                "AZ": az_state,
                "CAS": cas_state,
                "overall": overall,
            }]
        )

        # If file exists, read and prepend
        if os.path.exists(LOG_CSV):
            try:
                old_df = pd.read_csv(LOG_CSV)
            except Exception as e:
                log("SCC", f"ERROR reading {LOG_CSV}, recreating: {e}")
                old_df = pd.DataFrame(
                    columns=["timestamp", "ALT", "AZ", "CAS", "overall"]
                )
        else:
            old_df = pd.DataFrame(
                columns=["timestamp", "ALT", "AZ", "CAS", "overall"]
            )

        updated = pd.concat([new_row, old_df], ignore_index=True)

        try:
            updated.to_csv(LOG_CSV, index=False)
        except Exception as e:
            log("SCC", f"ERROR writing {LOG_CSV}: {e}")

    # ------------- Messaging to TCS -------------

    def send_summary(self):
        """
        Send a TELESCOPESUMMARY message to TCS and log a snapshot.

        Body layout:
          {
            "msg_type": "TELESCOPESUMMARY",
            "source": "SCC",
            "timestamp": ...,
            "overall_state": "...",
            "subsystems": { "ALT": {...}, "AZ": {...}, "CAS": {...} }
          }
        """
        topic = "telestatus.overall"
        body = {
            "msg_type": "TELESCOPESUMMARY",
            "source": "SCC",
            "timestamp": now_ts(),
            "overall_state": self.compute_overall_state(),
            "subsystems": self.subsystems,
        }

        self.comms.send_summary(SimMessage(topic=topic, body=body))

        log("SCC", f"SENT TELESCOPESUMMARY: {body}")

        # Also log a snapshot to the CSV
        self.log_snapshot_pandas()

    def summary_loop(self):
        """
        Background thread that sends summary to TCS every few seconds.
        """
        while self.running:
            time.sleep(5.0)
            if self.subsystems:
                self.send_summary()

    # ------------- Main receive loop -------------

    def run(self):
        """
        Main loop for SCC simulator.

        - Receives heartbeat JSON messages from subsystems
        - Updates self.subsystems in memory
        - Logs each heartbeat snapshot to CSV
        - Background thread sends periodic TELESCOPESUMMARY
        """
        log("SCC", "SCC simulator starting, listening for heartbeat")
        log("SCC", "SCC publishing telescope summaries")

        # Start periodic summary thread
        threading.Thread(target=self.summary_loop, daemon=True).start()

        while self.running:
            try:
                message = self.comms.receive_heartbeat(timeout_ms=1000)
            except KeyboardInterrupt:
                self.running = False
                break

            if message is None:
                continue

            topic = message.topic
            msg = message.body

            source = msg.get("source", "UNKNOWN")
            state = msg.get("state", "UNKNOWN")
            pos = msg.get("current_pos_deg", None)
            ts = msg.get("timestamp", now_ts())

            # Update in memory state
            self.subsystems[source] = {
                "state": state,
                "last_heartbeat": ts,
                "current_pos_deg": pos,
            }

            log("SCC", f"RECV HB from {source}: state={state}, pos={pos}, ts={ts}")

            # Log snapshot for this new heartbeat
            self.log_snapshot_pandas()

        log("SCC", "SCC simulator shutting down.")
