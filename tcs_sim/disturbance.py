"""Shared, configurable optical tracking disturbance."""

from __future__ import annotations

import math
import threading
import time

import numpy as np


class TrackingDisturbance:
    """Correlated tangent-plane error in arcseconds.

    The axis servo remains ideal; this object represents encoder/optical error
    seen by status reporting and the guide camera.
    """

    def __init__(self, config: dict | None = None):
        cfg = config or {}
        seed = cfg.get("seed")
        self.rng = np.random.default_rng(seed)
        fixed = cfg.get("fixed_initial_offset_arcsec")
        if fixed is not None:
            self.initial = np.asarray(fixed, dtype=float)
        else:
            radius = float(cfg.get("initial_offset_radius_arcsec", 1.5))
            angle = self.rng.uniform(0.0, 2.0 * math.pi)
            distance = radius * math.sqrt(self.rng.uniform())
            self.initial = np.array([distance * math.cos(angle), distance * math.sin(angle)])
        self.rms = float(cfg.get("drift_rms_arcsec", 0.35))
        self.tau = max(float(cfg.get("drift_tau_s", 4.0)), 0.01)
        self.noise = float(cfg.get("measurement_noise_arcsec", 0.03))
        self.vibrations = cfg.get("vibrations", [
            {"amplitude_arcsec": 0.12, "frequency_hz": 0.7, "axis_deg": 20.0},
            {"amplitude_arcsec": 0.06, "frequency_hz": 2.1, "axis_deg": 105.0},
        ])
        self.enabled = bool(cfg.get("enabled", True))
        self.started = time.monotonic()
        self.updated = self.started
        self.drift = np.zeros(2)
        self.last = np.zeros(2)
        self.lock = threading.Lock()

    def sample(self, tracking: bool = True, now: float | None = None) -> tuple[float, float]:
        if not self.enabled or not tracking:
            return 0.0, 0.0
        now = time.monotonic() if now is None else now
        with self.lock:
            dt = max(0.0, min(now - self.updated, 1.0))
            if dt:
                decay = math.exp(-dt / self.tau)
                sigma = self.rms * math.sqrt(max(0.0, 1.0 - decay * decay))
                self.drift = decay * self.drift + self.rng.normal(0.0, sigma, 2)
                self.updated = now
            value = self.initial + self.drift
            elapsed = now - self.started
            for item in self.vibrations:
                amplitude = float(item.get("amplitude_arcsec", 0.0))
                phase = 2.0 * math.pi * float(item.get("frequency_hz", 0.0)) * elapsed
                axis = math.radians(float(item.get("axis_deg", 0.0)))
                value = value + amplitude * math.sin(phase) * np.array([math.cos(axis), math.sin(axis)])
            value = value + self.rng.normal(0.0, self.noise, 2)
            self.last = value
            return float(value[0]), float(value[1])

    @staticmethod
    def axis_offsets(x_arcsec: float, y_arcsec: float, altitude_deg: float) -> tuple[float, float]:
        cos_alt = max(abs(math.cos(math.radians(altitude_deg))), 0.05)
        return y_arcsec / 3600.0, x_arcsec / (3600.0 * cos_alt)
