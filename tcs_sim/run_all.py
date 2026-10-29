# Observatory Simulator for the 2m Telescope at the IUCAA Girawali Observatory 
# Copyright (C) 2026  Harshit Krishna, Soumadip Bhowmick & Manthan Bhagat

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
from pathlib import Path
import threading
import time
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import numpy as np

from utils import log
from bus_config import build_bus_config
from comms.factory import create_comms_backend
from alt_sim import ALTSubsystem
from az_sim import AZSubsystem
from cas_sim import CASSubsystem
from scc_sim import SCCSimulator
from amn_sim import AMNSubsystem
from enc_sim import ENCSubsystem
from ags_sim import AGSSubsystem
from disturbance import TrackingDisturbance

def live_sky_plot(alt_obj, az_obj, ags_obj):
    fig = plt.figure(figsize=(14, 10))
    grid = fig.add_gridspec(2, 2, height_ratios=[1.0, 1.15], hspace=0.35, wspace=0.25)
    ax = fig.add_subplot(grid[0, 0], projection='polar')
    camera_ax = fig.add_subplot(grid[0, 1])
    history_ax = fig.add_subplot(grid[1, :], projection='3d')
    
    # --- 1. Setup Standard Sky Chart Orientation ---
    ax.set_theta_zero_location('N')      # 0° azimuth (North) at Top
    ax.set_theta_direction(-1)           # Clockwise (N -> E -> S -> W)
    
    # Zenith (90°) at Center (r=0), Horizon (0°) at Edge (r=90)
    ax.set_rlim(0, 90)                   
    ax.set_yticks([0, 30, 60, 90])       # Draw rings
    ax.set_yticklabels(['90°', '60°', '30°', '0°']) # Label them correctly
    
    ax.set_title("Live Telescope Tracking\n(Center=Zenith, Edge=Horizon)")

    # --- 2. Create Plot Objects ---
    # Blue Line: Trail of where we have been
    trail, = ax.plot([], [], 'b-', linewidth=1, alpha=0.5, label='Trail')
    
    # Red Dot: Where the telescope IS right now
    current_point, = ax.plot([], [], 'ro', markersize=8, label='Actual Pos')
    
    # Green Star: Where the telescope WANTS to be (Target)
    target_point, = ax.plot([], [], 'g*', markersize=12, label='Target Pos')
    
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.1))
    history_ax.set_title("Telescope Trajectory and Achieved Position")
    history_ax.set_xlabel("Elapsed time (s)")
    history_ax.set_ylabel("Altitude (deg)")
    history_ax.set_zlabel("Azimuth (deg, unwrapped)")
    trajectory_history, = history_ax.plot(
        [], [], [], "g--", linewidth=1.5, label="Demanded trajectory"
    )
    achieved_history, = history_ax.plot(
        [], [], [], "r-", linewidth=1.2, label="Disturbed achieved position"
    )
    history_ax.legend(loc="upper left")

    camera_image = camera_ax.imshow(
        np.zeros((ags_obj.window, ags_obj.window)), cmap="gray", origin="lower"
    )
    camera_waiting = camera_ax.text(
        0.5, 0.5, "Waiting for guide acquisition", ha="center", va="center",
        color="white", transform=camera_ax.transAxes,
    )
    button = Button(fig.add_axes([0.81, 0.505, 0.14, 0.035]), "Save Snapshot")

    def save_snapshot(_event):
        with ags_obj.frame_lock:
            frame = None if ags_obj.latest_frame is None else ags_obj.latest_frame.copy()
        if frame is None:
            return
        directory = Path(ags_obj.cfg.get("snapshots_dir", "autoguider_snapshots"))
        directory.mkdir(parents=True, exist_ok=True)
        filename = directory / f"guide_{time.strftime('%Y%m%d_%H%M%S')}.png"
        plt.imsave(filename, frame, cmap="gray")
        log("AGS", f"Saved snapshot {filename}")

    centroid_point, = camera_ax.plot([], [], "r+", markersize=12)
    centroid_point.set_visible(False)
    camera_ax.set_title("Autoguider: OFF")
    button.on_clicked(save_snapshot)


    # Data buffers for the trail
    theta_list = []
    r_list = []
    history_time = []
    achieved_alt = []
    achieved_az = []
    demanded_alt = []
    demanded_az = []
    plot_started = time.monotonic()

    plt.ion()
    plt.show()

    log("PLOT", "Live plot started.")

    while True:
        try:
            # --- 3. Read Data (Thread-Safe enough for plotting) ---
            # Actual Position
            curr_alt = alt_obj.reported_pos_deg()
            curr_az  = az_obj.reported_pos_deg()
            
            # Target Position (The command/trajectory)
            targ_alt = alt_obj.target_pos_deg
            targ_az  = az_obj.target_pos_deg
            elapsed = time.monotonic() - plot_started
            history_time.append(elapsed)
            achieved_alt.append(curr_alt)
            achieved_az.append(curr_az)
            demanded_alt.append(targ_alt)
            demanded_az.append(targ_az)

            # --- 4. Convert to Polar Coordinates ---
            # r = 90 - Altitude (So 90 becomes 0 at center)
            r_curr = 90.0 - curr_alt
            theta_curr = np.radians(curr_az)
            
            r_targ = 90.0 - targ_alt
            theta_targ = np.radians(targ_az)

            # --- 5. Update Trail ---
            # Only add to trail if position changed slightly (optimization)
            if not theta_list or abs(theta_list[-1] - theta_curr) > 0.01 or abs(r_list[-1] - r_curr) > 0.1:
                theta_list.append(theta_curr)
                r_list.append(r_curr)
                # Keep trail short (last 100 points) so it doesn't get messy
                if len(theta_list) > 100:
                    theta_list.pop(0)
                    r_list.pop(0)
            with ags_obj.frame_lock:
                frame = None if ags_obj.latest_frame is None else ags_obj.latest_frame.copy()
                guide = dict(ags_obj.latest)
            if frame is not None:
                camera_waiting.set_visible(False)
                camera_image.set_data(frame)
                camera_image.set_clim(
                    float(np.percentile(frame, 5)),
                    float(np.percentile(frame, 99.5)),
                )
                x0 = guide.get("window_tlx", 0)
                y0 = guide.get("window_tly", 0)
                centroid_point.set_visible(guide.get("guide_status") != "F")
                centroid_point.set_data(
                    [guide.get("centroid_x", 0) - x0],
                    [guide.get("centroid_y", 0) - y0],
                )
            else:
                camera_waiting.set_visible(True)
                centroid_point.set_visible(False)
            camera_ax.set_title(
                f"Autoguider: {ags_obj.mode}  focus={ags_obj.focus_mm:.2f} mm\n"
                f"FWHM={guide.get('fwhm', 0):.2f} px  "
                f"status={guide.get('guide_status', 'F')}"
            )

            # --- 6. Draw ---
            trail.set_data(theta_list, r_list)
            current_point.set_data([theta_curr], [r_curr])
            target_point.set_data([theta_targ], [r_targ])
            trajectory_history.set_data_3d(history_time, demanded_alt, demanded_az)
            achieved_history.set_data_3d(history_time, achieved_alt, achieved_az)
            history_ax.set_xlim(0.0, max(1.0, elapsed))
            all_alt = demanded_alt + achieved_alt
            all_az = demanded_az + achieved_az
            history_ax.set_ylim(min(all_alt) - 1.0, max(all_alt) + 1.0)
            history_ax.set_zlim(min(all_az) - 1.0, max(all_az) + 1.0)

            fig.canvas.draw_idle()
            fig.canvas.flush_events()
            
            time.sleep(0.1) # 10Hz Refresh Rate

        except KeyboardInterrupt:
            print("[Plot] Interrupted")
            break
        except Exception as e:
            print(f"[Plot] Error: {e}")
            break
            
    plt.close(fig)

if __name__ == "__main__":
    log("MAIN", "Starting Simulators...")

    bus_config = build_bus_config()
    disturbance = TrackingDisturbance(bus_config.autoguider.get("disturbance"))
    comms_backends = []

    def make_comms(owner: str, role: str):
        backend = create_comms_backend(bus_config, owner=owner, role=role)
        backend.start()
        comms_backends.append(backend)
        return backend

    # 1. Initialize Subsystems.
    # Each component gets its own comms backend instance.
    alt = ALTSubsystem(make_comms("ALT", "subsystem"))
    az = AZSubsystem(make_comms("AZ", "subsystem"))
    cas = CASSubsystem(make_comms("CAS", "subsystem"))
    amn = AMNSubsystem(make_comms("AMN", "subsystem"))
    enc = ENCSubsystem(make_comms("ENC", "subsystem"))
    alt.disturbance = disturbance
    az.disturbance = disturbance
    ags = AGSSubsystem(
        make_comms("AGS", "subsystem"),
        bus_config.autoguider,
        alt, az, disturbance,
    )


    # SCC has a different communication role.
    scc = SCCSimulator(make_comms("SCC", "scc"))

    # 2. Start listener threads.
    threads = [
        threading.Thread(target=alt.run, daemon=True),
        threading.Thread(target=az.run, daemon=True),
        threading.Thread(target=cas.run, daemon=True),
        threading.Thread(target=scc.run, daemon=True),
        threading.Thread(target=amn.run, daemon=True),
        threading.Thread(target=enc.run, daemon=True),
        threading.Thread(target=ags.run, daemon=True),
    ]

    for t in threads:
        t.start()

    log("MAIN", f"All Systems GO using backend={bus_config.backend}. Launching GUI...")

    try:
        live_sky_plot(alt, az, ags)
    except KeyboardInterrupt:
        log("MAIN", "Shutting down...")
    finally:
        ags.stop()
        for backend in comms_backends:
            backend.stop()
