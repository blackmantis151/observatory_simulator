import time

from .base import CommsBackend
from .message import SimMessage


class CilComms(CommsBackend):
    """
    CIL communication backend placeholder.

    Later this will communicate with tcs_sim/Cil/cil_sender.
    For now it proves that YAML backend switching works.
    """

    def __init__(self, bus_config, owner: str, role: str):
        super().__init__(bus_config, owner, role)
        self.cil_cfg = bus_config.cil
        self.running = False

    def start(self) -> None:
        self.running = True
        print(
            f"[CIL] Starting dummy CIL backend: owner={self.owner}, role={self.role}"
        )
        print(f"[CIL] cil_map={self.cil_cfg.get('cil_map')}")
        print(f"[CIL] local_name={self.cil_cfg.get('local_name')}")
        print(
            f"[CIL] bridge={self.cil_cfg.get('bridge_host')}:"
            f"{self.cil_cfg.get('bridge_port')}"
        )

    def stop(self) -> None:
        self.running = False
        print(f"[CIL] Stopping dummy CIL backend: owner={self.owner}, role={self.role}")

    def send_result(self, message: SimMessage) -> None:
        print(f"[CIL:{self.owner}] RESULT {message}")

    def send_status(self, message: SimMessage) -> None:
        print(f"[CIL:{self.owner}] STATUS {message}")

    def send_heartbeat(self, message: SimMessage) -> None:
        print(f"[CIL:{self.owner}] HEARTBEAT {message}")

    def send_summary(self, message: SimMessage) -> None:
        print(f"[CIL:{self.owner}] SUMMARY {message}")

    def receive_command(self, timeout_ms: int | None = None) -> SimMessage | None:
        if timeout_ms:
            time.sleep(timeout_ms / 1000.0)
        else:
            time.sleep(0.1)
        return None

    def receive_heartbeat(self, timeout_ms: int | None = None) -> SimMessage | None:
        if timeout_ms:
            time.sleep(timeout_ms / 1000.0)
        else:
            time.sleep(0.1)
        return None
