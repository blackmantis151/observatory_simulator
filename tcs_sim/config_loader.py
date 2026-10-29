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
    sidecar = DEFAULT_CONFIG_PATH.with_name("autoguider.yaml")
    if "autoguider" not in config and sidecar.is_file():
        with sidecar.open("r", encoding="utf-8") as stream:
            config["autoguider"] = yaml.safe_load(stream)
    ags_profile = DEFAULT_CONFIG_PATH.with_name("ags-cil-profile.yaml")
    endpoints = config.get("cil", {}).get("endpoints", {})
    if "AGS" not in endpoints and ags_profile.is_file():
        with ags_profile.open("r", encoding="utf-8") as stream:
            endpoints["AGS"] = yaml.safe_load(stream)

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

    if backend == "zmq" and "zmq" not in config:
        raise ValueError("backend='zmq' but missing 'zmq' config section.")

    if backend == "zmq" and "network" not in config:
        raise ValueError("backend='zmq' but missing 'network' config section.")

    if backend == "cil" and "cil" not in config:
        raise ValueError("backend='cil' but missing 'cil' config section.")

    if backend == "cil":
        _validate_cil_config(config["cil"])


def _validate_cil_config(cil: dict) -> None:
    required = ("bridge_executable", "map_file", "endpoints")
    for key in required:
        if not cil.get(key):
            raise ValueError(f"backend='cil' but missing cil.{key}")

    bridge = Path(cil["bridge_executable"])
    map_file = Path(cil["map_file"])
    if not bridge.is_absolute() or not map_file.is_absolute():
        raise ValueError("cil.bridge_executable and cil.map_file must be absolute paths")
    if not bridge.is_file() or not os.access(bridge, os.X_OK):
        raise ValueError(f"CIL bridge is missing or not executable: {bridge}")
    if not map_file.is_file():
        raise ValueError(f"CIL map is missing: {map_file}")

    endpoints = cil["endpoints"]
    for owner in ("ALT", "AZ", "CAS", "AMN", "ENC", "AGS", "SCC"):
        endpoint = endpoints.get(owner)
        if not isinstance(endpoint, dict) or not endpoint.get("identities"):
            raise ValueError(f"Missing CIL endpoint identities for {owner}")
        for identity in endpoint["identities"]:
            if not identity.get("name"):
                raise ValueError(f"CIL endpoint {owner} has an identity without a name")
