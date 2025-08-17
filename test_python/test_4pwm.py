#!/usr/bin/env python3
"""Safely test single-packet 4PWM transmission to Crazyflie/Bolt."""

import argparse
import struct
import sys
import time

from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.crazyflie.crtp import CRTPPacket, CRTPPort
from cflib.crtp import init_drivers

TYPE_MOTOR = 8
TYPE_ARM = 9


def clamp(value, low=0, high=0xFFFF):
    """Clamp *value* to the inclusive range [low, high]."""
    return max(low, min(high, int(value)))


def _send_generic(cf, payload):
    pk = CRTPPacket()
    pk.port = CRTPPort.COMMANDER_GENERIC
    pk.data = payload
    cf.send_packet(pk)


def arm(cf, state):
    _send_generic(cf, struct.pack("<BB>", TYPE_ARM, state))


def send_motor(cf, m1, m2, m3, m4):
    _send_generic(cf, struct.pack("<BHHHH>", TYPE_MOTOR, m1, m2, m3, m4))


def main():
    parser = argparse.ArgumentParser(
        description="Send single-packet 4PWM commands to Crazyflie/Bolt."
    )
    parser.add_argument(
        "--uri",
        default="radio://0/80/2M/E7E7E7E7E7",
        help="Connection URI (default: %(default)s)",
    )
    parser.add_argument(
        "--m",
        nargs=4,
        type=int,
        default=[500, 500, 500, 500],
        metavar=("M1", "M2", "M3", "M4"),
        help="Four motor PWM values (default: %(default)s)",
    )
    parser.add_argument(
        "--hold",
        type=float,
        default=1.0,
        help="Seconds to hold motor values (default: %(default)s)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print actions without connecting",
    )

    args = parser.parse_args()
    motors = [clamp(m) for m in args.m]

    if args.dry_run:
        print(f"Would connect to {args.uri}")
        print(
            "Would ARM, send PWM",
            motors,
            f"for {args.hold} s, then DISARM.",
        )
        return

    init_drivers()
    try:
        with SyncCrazyflie(args.uri, cf=Crazyflie(rw_cache="./cache")) as scf:
            cf = scf.cf
            try:
                arm(cf, 1)
                print("Armed.")
                send_motor(cf, *motors)
                time.sleep(args.hold)
            finally:
                send_motor(cf, 0, 0, 0, 0)
                arm(cf, 0)
                print("Disarmed.")
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr)


if __name__ == "__main__":
    main()
