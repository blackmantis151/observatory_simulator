from pathlib import Path
import os
import yaml


DEFAULT_CONFIG_PATH = Path(__file__).resolve().parent / "config" / "comms.yaml"


def load_config(config_path: str | None = None) -> dict:
    """
    Load simulator communication configuration.

    Priority:
    1. Explicit config_path argument
    2. OBS_SIM_CONFIG environment variable
    3. tcs_sim/config/comms.yaml
    """

    path = (
        Path(config_path)
        if config_path is not None
        else Path(os.environ.get("OBS_SIM_CONFIG", DEFAULT_CONFIG_PATH))
    )

    if not path.exists():
        raise FileNotFoundError(f"Simulator config file not found: {path}")

    with open(path, "r", encoding="utf-8") as f:
        config = yaml.safe_load(f)

    validate_config(config)
    return config


def validate_config(config: dict) -> None:
    if not isinstance(config, dict):
        raise ValueError("Configuration file is empty or invalid.")

    backend = config.get("comms", {}).get("backend")

    if backend not in {"zmq", "cil"}:
        raise ValueError(
            f"Invalid comms backend '{backend}'. Expected 'zmq' or 'cil'."
        )

    if "network" not in config:
        raise ValueError("Missing required 'network' section in YAML config.")

    if backend == "zmq" and "zmq" not in config:
        raise ValueError("backend='zmq' but missing 'zmq' config section.")

    if backend == "cil" and "cil" not in config:
        raise ValueError("backend='cil' but missing 'cil' config section.")
