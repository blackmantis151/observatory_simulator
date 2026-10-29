"""Synthetic autoguider camera and legacy guide-packet transmitter."""

from __future__ import annotations

import math
import os
from pathlib import Path
import socket
import threading
import time

import numpy as np

from comms.message import SimMessage
from utils import log, now_ts


AGS_SERVICE = 0x004F0000
SYS_NOMINAL = 0x00010000
AGS_BAD_CMD = 0x004F0001
AGS_LOOP_ERROR = 0x004F0004

COMMAND_NUMBERS = {
    "GUIDE_ON_BRIGHTEST": 1,
    "GUIDE_ON_RANK": 2,
    "GUIDE_ON_RANGE": 3,
    "GUIDE_ON_PIXEL": 4,
    "GUIDE_OFF": 5,
    "START_SESSION": 10,
    "END_SESSION": 11,
}
NUMBER_COMMANDS = {value: key for key, value in COMMAND_NUMBERS.items()}


def format_guide_packet(x: float, y: float, timecode: float, status: str) -> bytes:
    if not (0.0 <= x <= 9999.99 and 0.0 <= y <= 9999.99 and -999.99 <= timecode <= 9999.99):
        raise ValueError("Guide values are outside the ICD field range")
    prefix = f"{x:08.2f} {y:08.2f} {timecode:08.2f} {status} "
    if len(prefix) != 29 or len(status) != 1:
        raise ValueError("Guide values do not fit the 34-byte ICD packet")
    checksum = sum(prefix.encode("ascii"))
    packet = f"{prefix}{checksum:04d}\r".encode("ascii")
    if len(packet) != 34:
        raise ValueError("Invalid guide packet length")
    return packet


class AGSSubsystem:
    def __init__(self, comms, config: dict, alt, az, disturbance):
        self.name = "AGS"
        self.comms = comms
        self.cfg = config
        self.alt = alt
        self.az = az
        self.disturbance = disturbance
        self.running = True
        self.session_active = False
        self.guiding = False
        self.mode = "OFF"
        self.focus_mm = float(config.get("focus", {}).get("default_mm", 12.5))
        self.frame_lock = threading.Lock()
        self.latest_frame = None
        self.latest = self._empty_status()
        self.rng = np.random.default_rng(config.get("disturbance", {}).get("seed"))
        camera = config.get("camera", {})
        self.width = int(camera.get("width", 1024))
        self.height = int(camera.get("height", 1024))
        self.window = int(camera.get("guide_window", 100))
        margin = self.window
        count = int(camera.get("star_count", 20))
        self.stars = [
            {
                "x": float(self.rng.uniform(margin, self.width - margin)),
                "y": float(self.rng.uniform(margin, self.height - margin)),
                "mag": float(self.rng.uniform(*camera.get("magnitude_range", [8.0, 15.0]))),
            }
            for _ in range(count)
        ]
        self.selected = None
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_target = (
            str(config.get("guide_udp_host", "127.0.0.1")),
            int(config.get("guide_udp_port", 13025)),
        )
        self.command_table = {
            "START_SESSION": self._start_session,
            "END_SESSION": self._end_session,
            "GUIDE_ON_BRIGHTEST": self._guide_brightest,
            "GUIDE_ON_RANK": self._guide_rank,
            "GUIDE_ON_PIXEL": self._guide_pixel,
            "GUIDE_ON_RANGE": self._guide_range,
            "GUIDE_OFF": self._guide_off,
            "SET_FOCUS": self._set_focus,
            "ACTIVATE": self._activate,
            "SAFESTATE": self._safestate,
            "SHUTDOWN": self._shutdown,
        }
        threading.Thread(target=self._camera_loop, daemon=True).start()

    def _empty_status(self):
        return {
            "centroid_x": 0.0, "centroid_y": 0.0, "fwhm": 0.0,
            "magnitude": 0.0, "peak": 0, "guide_status": "F",
            "pointing_error_x_arcsec": 0.0, "pointing_error_y_arcsec": 0.0,
        }

    def _result(self, kind, cmd_id, command, status=SYS_NOMINAL, **extra):
        body = {
            "msg_type": kind, "source": "AGS", "cmd_id": cmd_id,
            "timestamp": now_ts(), "Class": 2 if kind == "COMPLETED" else 6,
            "Service": AGS_SERVICE, "command": COMMAND_NUMBERS.get(command, 0),
            "status": status, "param1": 0, "param2": 0,
            "session_active": self.session_active, "guiding": self.guiding,
        }
        body.update(extra)
        self.comms.send_result(SimMessage("result.AGS", body))

    def _complete(self, cmd_id, command, **extra):
        self._result("COMPLETED", cmd_id, command, **extra)

    def _error(self, cmd_id, command, code=AGS_BAD_CMD):
        self._result("ERROR", cmd_id, command, code, error_code="BADPARAM")

    def _start_session(self, cmd_id, params):
        self.session_active = True
        self.mode = "IDLE"
        self._complete(cmd_id, "START_SESSION")

    def _end_session(self, cmd_id, params):
        self._stop_guiding()
        self.session_active = False
        self.mode = "OFF"
        self._complete(cmd_id, "END_SESSION")

    def _guide_brightest(self, cmd_id, params):
        self._select(min(self.stars, key=lambda star: star["mag"]))
        self._complete(cmd_id, "GUIDE_ON_BRIGHTEST")

    def _guide_rank(self, cmd_id, params):
        try:
            rank = int(params.get("rank", params.get("param1", 0)))
            star = sorted(self.stars, key=lambda item: item["mag"])[rank]
        except (ValueError, TypeError, IndexError):
            self._error(cmd_id, "GUIDE_ON_RANK")
            return
        self._select(star)
        self._complete(cmd_id, "GUIDE_ON_RANK", param1=rank)

    def _guide_pixel(self, cmd_id, params):
        try:
            x = float(params.get("x", params.get("param1", 0) / 1000.0))
            y = float(params.get("y", params.get("param2", 0) / 1000.0))
        except (TypeError, ValueError):
            self._error(cmd_id, "GUIDE_ON_PIXEL")
            return
        if not (0 <= x < self.width and 0 <= y < self.height):
            self._error(cmd_id, "GUIDE_ON_PIXEL")
            return
        self._select(min(self.stars, key=lambda star: (star["x"] - x) ** 2 + (star["y"] - y) ** 2))
        self._complete(cmd_id, "GUIDE_ON_PIXEL", param1=round(x * 1000), param2=round(y * 1000))

    def _guide_range(self, cmd_id, params):
        self._error(cmd_id, "GUIDE_ON_RANGE", AGS_BAD_CMD)

    def _guide_off(self, cmd_id, params):
        self._stop_guiding()
        self._complete(cmd_id, "GUIDE_OFF")

    def _set_focus(self, cmd_id, params):
        focus = self.cfg.get("focus", {})
        try:
            demand = float(params.get("position_mm"))
        except (TypeError, ValueError):
            self._error(cmd_id, "SET_FOCUS")
            return
        if not float(focus.get("minimum_mm", 0.0)) <= demand <= float(focus.get("maximum_mm", 25.0)):
            self._error(cmd_id, "SET_FOCUS")
            return
        self.comms.send_result(SimMessage("result.AGS", {
            "msg_type": "ACK", "source": "AGS", "cmd_id": cmd_id,
            "timestamp": now_ts(), "focus_mm": self.focus_mm,
        }))
        delay = abs(demand - self.focus_mm) / max(float(focus.get("speed_mm_s", 5.0)), 0.01)
        def finish():
            time.sleep(delay)
            self.focus_mm = demand
            self._complete(cmd_id, "SET_FOCUS", focus_mm=self.focus_mm)
        threading.Thread(target=finish, daemon=True).start()

    def _mcp_ack(self, cmd_id):
        self.comms.send_result(SimMessage("result.AGS", {
            "msg_type": "ACK", "source": "AGS", "cmd_id": cmd_id,
            "timestamp": now_ts(), "status": SYS_NOMINAL,
        }))

    def _activate(self, cmd_id, params):
        self._mcp_ack(cmd_id)
        self._complete(cmd_id, "ACTIVATE", mcp_state=1)

    def _safestate(self, cmd_id, params):
        self._mcp_ack(cmd_id)
        self._stop_guiding()
        self._complete(cmd_id, "SAFESTATE", mcp_state=6)

    def _shutdown(self, cmd_id, params):
        self._mcp_ack(cmd_id)
        self._stop_guiding()
        self.session_active = False
        self.mode = "OFF"
        self._complete(cmd_id, "SHUTDOWN", mcp_state=6)

    def _select(self, star):
        self.selected = star
        self.guiding = True
        self.mode = "GUIDING"

    def _stop_guiding(self):
        was_guiding = self.guiding
        self.guiding = False
        self.mode = "IDLE" if self.session_active else "OFF"
        if was_guiding:
            status = self.latest
            self._send_packet(status["centroid_x"], status["centroid_y"], 0.0, "0")

    def handle_command(self, message):
        command = str(message.get("command", "")).upper()
        handler = self.command_table.get(command)
        if handler is None:
            self._error(message.get("cmd_id"), command)
            return
        handler(message.get("cmd_id"), message.get("params") or {})

    def _camera_loop(self):
        cadence = max(float(self.cfg.get("cadence_s", 1.0)), 0.05)
        while self.running:
            started = time.monotonic()
            if self.guiding and self.selected is not None:
                frame, status = self._render()
                with self.frame_lock:
                    self.latest_frame, self.latest = frame, status
                self._write_latest(frame)
                self._send_packet(status["centroid_x"], status["centroid_y"],
                                  cadence * float(self.cfg.get("timecode_scale", 2.0)),
                                  status["guide_status"])
                self._send_status()
            time.sleep(max(0.0, cadence - (time.monotonic() - started)))

    def _render(self):
        camera = self.cfg.get("camera", {})
        focus = self.cfg.get("focus", {})
        tracking = bool(getattr(self.alt, "tracking_active", False) and getattr(self.az, "tracking_active", False))
        dx_arcsec, dy_arcsec = self.disturbance.sample(tracking)
        scale = max(float(self.cfg.get("pixel_scale_arcsec", 1.0)), 1e-6)
        rotation = math.radians(float(self.cfg.get("rotation_deg", 0.0)))
        dx = (dx_arcsec * math.cos(rotation) - dy_arcsec * math.sin(rotation)) / scale
        dy = (dx_arcsec * math.sin(rotation) + dy_arcsec * math.cos(rotation)) / scale
        cx = self.selected["x"] + dx
        cy = self.selected["y"] + dy
        x0 = int(round(self.selected["x"] - self.window / 2))
        y0 = int(round(self.selected["y"] - self.window / 2))
        yy, xx = np.mgrid[0:self.window, 0:self.window]
        optimum = float(focus.get("optimum_mm", 12.5))
        base_fwhm = float(focus.get("in_focus_fwhm_px", 2.0))
        defocus = float(focus.get("defocus_px_per_mm", 0.8)) * (self.focus_mm - optimum)
        true_fwhm = math.sqrt(base_fwhm * base_fwhm + defocus * defocus)
        sigma = true_fwhm / 2.35482
        flux = 10 ** ((float(camera.get("magnitude_zero_point", 24.4)) - self.selected["mag"]) / 2.5)
        image = float(camera.get("background_adu", 100.0)) + flux / (2 * math.pi * sigma ** 2) * np.exp(
            -((xx + x0 + 0.5 - cx) ** 2 + (yy + y0 + 0.5 - cy) ** 2) / (2 * sigma ** 2))
        image = self.rng.poisson(np.maximum(image, 0.0)).astype(float)
        image += self.rng.normal(0.0, float(camera.get("read_noise_adu", 5.0)), image.shape)
        image = np.clip(image, 0, float(camera.get("saturation_adu", 65535.0)))
        background = float(np.median(image))
        noise_sigma = math.sqrt(max(background, 0.0) + float(camera.get("read_noise_adu", 5.0)) ** 2)
        threshold = background + 5.0 * noise_sigma
        weights = np.where(image >= threshold, image - background, 0.0)
        total = float(weights.sum())
        if total <= max(20.0, float(camera.get("detection_threshold", 50.0))):
            measured_x = measured_y = 0.0
            measured_fwhm = 0.0
            guide_status = "F"
        else:
            mx = float((weights * xx).sum() / total)
            my = float((weights * yy).sum() / total)
            variance = float((weights * ((xx - mx) ** 2 + (yy - my) ** 2)).sum() / (2 * total))
            measured_fwhm = 2.35482 * math.sqrt(max(variance, 0.0))
            measured_x, measured_y = mx + x0 + 0.5, my + y0 + 0.5
            edge = min(mx, my, self.window - 1 - mx, self.window - 1 - my) < 10
            snr = total / math.sqrt(max(total + image.size * float(camera.get("read_noise_adu", 5.0)) ** 2, 1.0))
            guide_status = "W" if edge else str(min(7, max(0, int(20 / max(snr, 1.0)))))
        return image, {
            "centroid_x": measured_x, "centroid_y": measured_y,
            "fwhm": measured_fwhm, "magnitude": self.selected["mag"],
            "peak": int(image.max()), "guide_status": guide_status,
            "window_tlx": x0, "window_tly": y0,
            "window_brx": x0 + self.window - 1, "window_bry": y0 + self.window - 1,
            "pointing_error_x_arcsec": dx_arcsec, "pointing_error_y_arcsec": dy_arcsec,
        }

    def _send_packet(self, x, y, timecode, status):
        try:
            self.socket.sendto(format_guide_packet(x, y, timecode, status), self.udp_target)
        except (OSError, ValueError) as exc:
            log("AGS", f"Guide packet failed: {exc}")

    def _send_status(self):
        body = {
            "msg_type": "STATUS", "source": "AGS", "timestamp": now_ts(),
            "session_active": self.session_active, "guiding": self.guiding,
            "activity": self.mode, "focus_mm": self.focus_mm,
        }
        body.update(self.latest)
        self.comms.send_status(SimMessage("status.AGS", body))

    def _write_latest(self, frame):
        path = self.cfg.get("latest_png")
        if not path:
            return
        try:
            from matplotlib import pyplot as plt
            destination = Path(path)
            destination.parent.mkdir(parents=True, exist_ok=True)
            temporary = destination.with_suffix(destination.suffix + ".tmp")
            plt.imsave(temporary, frame, cmap="gray", format="png")
            os.replace(temporary, destination)
        except (OSError, ValueError) as exc:
            log("AGS", f"Latest image write failed: {exc}")

    def send_heartbeat(self):
        self.comms.send_heartbeat(SimMessage("heartbeat.AGS", {
            "msg_type": "HEARTBEAT", "source": "AGS", "timestamp": now_ts(),
            "state": "OK", "session_active": self.session_active, "guiding": self.guiding,
        }))

    def run(self):
        next_heartbeat = 0.0
        while self.running:
            now = time.monotonic()
            if now >= next_heartbeat:
                self.send_heartbeat()
                self._send_status()
                next_heartbeat = now + 1.0
            message = self.comms.receive_command(timeout_ms=200)
            if message is not None:
                body = dict(message.body)
                if body.get("msg_type") == "CMD":
                    self.handle_command(body)
                elif isinstance(body.get("command"), int):
                    body["msg_type"] = "CMD"
                    body["command"] = NUMBER_COMMANDS.get(
                        body["command"], "UNSUPPORTED"
                    )
                    body["params"] = {
                        "param1": body.get("param1", 0),
                        "param2": body.get("param2", 0),
                    }
                    body.setdefault("cmd_id", f"zmq:{time.time_ns()}")
                    self.handle_command(body)

    def stop(self):
        self._stop_guiding()
        self.running = False
        self.socket.close()
