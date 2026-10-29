def create_comms_backend(bus_config, owner: str, role: str):
    """
    Create a communication backend for one simulator component.

    owner examples:
      ALT, AZ, CAS, AMN, ENC, AGS, SCC

    role examples:
      subsystem, scc
    """

    if bus_config.backend == "zmq":
        from .zmq_comms import ZmqComms
        return ZmqComms(bus_config, owner=owner, role=role)

    if bus_config.backend == "cil":
        from .cil_comms import CilComms
        return CilComms(bus_config, owner=owner, role=role)

    raise ValueError(f"Unknown comms backend: {bus_config.backend}")
