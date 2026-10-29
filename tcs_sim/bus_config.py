# Observatory Simulator for the 2m Telescope at the IUCAA Girawali Observatory 
# Copyright (C) 2026  Harshit Krishna, Soumadip Bhowmick & Manthan Bhagatfrom dataclasses import dataclass
from config_loader import load_config


@dataclass(frozen=True)
class BusConfig:
    backend: str
    timeout_ms: int

    tcs_ip: str
    scc_ip: str

    cmd_endpoint: str
    result_endpoint: str
    heartbeat_endpoint: str
    scc_to_tcs_endpoint: str
    status_endpoint: str

    cil: dict
    autoguider: dict


def build_bus_config(config_path: str | None = None) -> BusConfig:
    """
    Build all runtime communication endpoints from YAML.

    This file should not contain hard-coded site-specific ports/IPs.
    Hard-coded defaults here are only safety fallbacks matching the old simulator.
    """

    config = load_config(config_path)

    backend = config["comms"]["backend"]
    timeout_ms = int(config["comms"].get("timeout_ms", 5000))

    network_cfg = config.get("network", {})
    zmq_cfg = config.get("zmq", {})
    cil_cfg = config.get("cil", {})

    tcs_ip = network_cfg.get("tcs_ip", "")
    scc_ip = network_cfg.get("scc_ip", "")

    return BusConfig(
        backend=backend,
        timeout_ms=timeout_ms,
        tcs_ip=tcs_ip,
        scc_ip=scc_ip,
        cmd_endpoint=f"tcp://{tcs_ip}:{int(zmq_cfg.get('cmd_port', 5555))}",
        result_endpoint=f"tcp://{tcs_ip}:{int(zmq_cfg.get('result_port', 5556))}",
        heartbeat_endpoint=f"tcp://{scc_ip}:{int(zmq_cfg.get('heartbeat_port', 5557))}",
        scc_to_tcs_endpoint=f"tcp://{scc_ip}:{int(zmq_cfg.get('scc_to_tcs_port', 5558))}",
        status_endpoint=f"tcp://{tcs_ip}:{int(zmq_cfg.get('status_port', 5559))}",
        cil=cil_cfg,
        autoguider=config.get("autoguider", {}),
    )
