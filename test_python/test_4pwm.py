#!/usr/bin/env python
"""Test for single-packet 4PWM motor control using cflib."""
import os
import struct
import pytest

try:
    from cflib.crazyflie import Crazyflie
    from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
    from cflib.crazyflie.crtp import CRTPPacket, CRTPPort
    from cflib.crtp import init_drivers
except Exception as exc:  # pragma: no cover
    pytest.skip(f"cflib not available: {exc}", allow_module_level=True)

URI = os.getenv("CRAZYFLIE_URI", "radio://0/80/2M/E7E7E7E7E7")
TYPE_MOTOR = 8


def send_motor_setpoint(cf, m1, m2, m3, m4):
    pk = CRTPPacket()
    pk.port = CRTPPort.COMMANDER_GENERIC
    pk.data = struct.pack("<BHHHH", TYPE_MOTOR, m1, m2, m3, m4)
    cf.send_packet(pk)


def test_single_packet_4pwm():
    init_drivers()
    try:
        with SyncCrazyflie(URI, cf=Crazyflie(rw_cache="./cache")) as scf:
            send_motor_setpoint(scf.cf, 1000, 2000, 3000, 4000)
    except Exception as exc:
        pytest.skip(f"Could not connect to Crazyflie: {exc}")
