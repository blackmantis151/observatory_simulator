import time
from datetime import datetime


def now_ts() -> int:
    """Return current Unix timestamp in seconds."""
    return int(time.time())


def log(role: str, msg: str) -> None:
    """Simple timestamped log."""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] [{role}] {msg}", flush=True)
