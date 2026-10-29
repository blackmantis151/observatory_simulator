import os
import time
from datetime import datetime
from pathlib import Path


LOG_DIR = Path(os.environ.get("OBS_SIM_LOG_DIR", Path(__file__).resolve().parent / "logs")).expanduser()


def now_ts() -> int:
    """Return current Unix timestamp in seconds."""
    return int(time.time())


def log(role: str, msg: str) -> None:
    """Simple timestamped log."""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] [{role}] {msg}", flush=True)
