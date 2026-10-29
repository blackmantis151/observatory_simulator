from dataclasses import dataclass, field
from typing import Any


@dataclass
class SimMessage:
    """
    Transport-neutral simulator message.

    The current ZMQ protocol sends two frames:
    1. topic string
    2. JSON body

    We preserve that internally as:
    topic: str
    body: dict
    """

    topic: str
    body: dict[str, Any] = field(default_factory=dict)
