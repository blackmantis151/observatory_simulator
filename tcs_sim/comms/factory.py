from .zmq_comms import ZmqComms
from .cil_comms import CilComms


def create_comms_backend(bus_config, owner: str, role: str):
    """
    Create a communication backend for one simulator component.

    owner examples:
      ALT, AZ, CAS, AMN, ENC, SCC

    role examples:
      subsystem, scc
    """

    if bus_config.backend == "zmq":
        return ZmqComms(bus_config, owner=owner, role=role)

    if bus_config.backend == "cil":
        return CilComms(bus_config, owner=owner, role=role)

    raise ValueError(f"Unknown comms backend: {bus_config.backend}")
