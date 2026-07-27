from abc import ABC, abstractmethod
from .message import SimMessage


class CommsBackend(ABC):
    """
    Common communication interface.

    Subsystems and SCC should use only this interface.
    ZMQ and CIL details must stay in concrete backend classes.
    """

    def __init__(self, bus_config, owner: str, role: str):
        self.bus = bus_config
        self.owner = owner.upper()
        self.role = role

    @abstractmethod
    def start(self) -> None:
        pass

    @abstractmethod
    def stop(self) -> None:
        pass

    @abstractmethod
    def send_result(self, message: SimMessage) -> None:
        pass

    @abstractmethod
    def send_status(self, message: SimMessage) -> None:
        pass

    @abstractmethod
    def send_heartbeat(self, message: SimMessage) -> None:
        pass

    @abstractmethod
    def send_summary(self, message: SimMessage) -> None:
        pass

    @abstractmethod
    def receive_command(self, timeout_ms: int | None = None) -> SimMessage | None:
        pass

    @abstractmethod
    def receive_heartbeat(self, timeout_ms: int | None = None) -> SimMessage | None:
        pass
