# Observatory Stimulator

Simulates the telescope ALT, AZ, CAS, auxiliary-mechanism, enclosure, and SCC
behavior. Communication is selected at startup with `comms.backend` in
`tcs_sim/config/comms.yaml`.

## ZMQ mode

`backend: zmq` retains the existing two-frame topic/JSON protocol. Run from
`tcs_sim` so the simulator CSV files stay together:

```sh
python3 run_all.py
```

## CIL mode

CIL mode uses one native bridge process per configured CIL identity. The
bridge only transports official CIL headers and binary payloads. Python
decodes commands, runs the mechanical simulation, and sends replies created
from actual simulator events. The production path contains no echo mode.

Build the bridge:

```sh
make -C tcs_sim/Cil
sudo install -D -m 0755 tcs_sim/Cil/cil_bridge \
  /opt/observatory-stimulator/bin/cil_bridge
```

Before selecting `backend: cil`:

1. Install the deployment `Cil.map` and set its absolute path.
2. Transcribe each service, response, status, SDB, and heartbeat structure
   from the official subsystem ICD/header into the endpoint profiles.
3. Replace every example service/status value with its official value.
4. Ensure no real process is already bound to an identity/port the simulator
   will occupy.

`tcs_sim/config/cil-profile.example.yaml` documents the field schema. The
supported field types are network-order `i32`, `u32`, `f32`, and `f64`, plus
explicit padding and fixed-length arrays. CIL mode deliberately fails startup
if an identity, service, heartbeat, status, or SCC profile is missing; it never
falls back to ZMQ or mirrored responses.

## Checks

```sh
python3 -m unittest discover -s tests -v
python3 -m compileall -q tcs_sim
make -C tcs_sim/Cil clean all
```
