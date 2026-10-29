# Observatory Simulator

Simulates the TCS motors and subsystems over ZeroMQ.  
When TCS fires a packet, the stimulator dispatches it to a command‐specific handler, simulates the mechanical response over time, and sends back status/feedback.

<!-- ## Structure

- `main.py` – entry point, sets up UDP and dispatcher  
- `config/` – YAML settings (ports, timing, speeds)  
- `state.py` – central simulated state (ALT, AZ, CAS, clock)  
- `udp.py` – low‐level UDP send/receive  
- `dispatcher.py` – routes incoming packets to handlers  
- `handlers/` – one module per command (e.g. `goto`, `focus`)  
- `feedback.py` – common feedback‐formatting utilities  
- `tests/` – pytest stubs -->
Simulates the telescope ALT, AZ, CAS, auxiliary-mechanism, enclosure, and SCC
behavior. Communication is selected at startup with `comms.backend` in
`tcs_sim/config/comms.yaml`.

## ZMQ mode

`backend: zmq` retains the existing two-frame topic/JSON protocol. Run from
`tcs_sim` so the simulator CSV files stay together:

```sh
python3 run_all.py
```

This project uses `uv` for package management
```bash
#clone repo
git clone https://github.com/blackmantis151/Observatory_Stimulator.git

#install uv
curl -LsSf https://astral.sh/uv/install.sh | sh
#sync all packages
uv sync

#run simulator
python tcs_sim/run_all.py
```

## License

This project is licensed under the GPLv3 License. See the LICENSE file for details.
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

Runtime CSV files are stored under `tcs_sim/logs/` regardless of the launch
directory. To place all logs elsewhere without editing the source:

```sh
OBS_SIM_LOG_DIR=/writable/log/directory python3 run_all.py
```


Set `OBS_SIM_SCC_LOG` only when the SCC CSV needs a separate file path.
