#!/usr/bin/env python3
"""Safely test 4×PWM transmission to Crazyflie/Bolt via dedicated PWM CRTP port (Route A).
- Sends ONLY 4×uint16 (<HHHH) payload (8 bytes) on port 0x0A, channel 0.
- No TYPE byte. Matches firmware expectation for the PWM port.
- Optionally toggles a known enable/arm param if available (e.g., pwm.enable or motorPowerSet.enable).
"""

import argparse
import struct
import sys
import time


PORT_PWM = 0x0A   # Dedicated 4PWM port in firmware
CHAN_PWM = 0      # Usually 0

# Candidate params to enable/disable PWM control on the firmware
ENABLE_PARAM_CANDIDATES = [
    "pwm.enable",             # preferred if your firmware defines this
    "motorPowerSet.enable",   # common in CF mods
]


def clamp(value, low=0, high=0xFFFF):
    """Clamp *value* to the inclusive range [low, high]."""
    return max(low, min(high, int(value)))


def send_packet_compat(cf, pkt):
    """Send CRTP *pkt* using a link-compatible API across cflib versions."""
    # Try modern path
    try:
        cf.cf.link.send_packet(pkt)
        return
    except AttributeError:
        pass
    # Try legacy/private path
    try:
        cf._link.send_packet(pkt)  # noqa: SLF001 (private attribute access for compatibility)
        return
    except AttributeError:
        pass
    raise RuntimeError("No compatible send_packet on Crazyflie link")


def send_4pwm_packet(cf, m1, m2, m3, m4):
    """Send exactly 8-byte payload: 4×uint16 (little-endian) to the PWM port."""
    from cflib.crtp import CRTPPacket  # local import to keep --dry-run fast

    payload = struct.pack("<HHHH", m1, m2, m3, m4)
    pk = CRTPPacket()
    pk.port = PORT_PWM
    pk.channel = CHAN_PWM
    pk.data = payload
    send_packet_compat(cf, pk)


def try_set_enable(cf, state: int) -> str:
    """
    Try to set one of the known enable params to 0/1.
    Returns the param name used, or '' if none succeeded.
    """
    for pname in ENABLE_PARAM_CANDIDATES:
        try:
            cf.param.set_value(pname, str(int(bool(state))))
            return pname
        except Exception:
            continue
    return ""


def main():
    parser = argparse.ArgumentParser(
        description="Send 4×PWM commands to Crazyflie/Bolt via dedicated PWM CRTP port (no TYPE byte)."
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
        help="Four motor PWM values in [0..65535] (default: %(default)s)",
    )
    parser.add_argument(
        "--hold",
        type=float,
        default=1.0,
        help="Seconds to hold motor values (default: %(default)s)",
    )
    parser.add_argument(
        "--rate",
        type=float,
        default=50.0,
        help="Send frequency (Hz) to avoid firmware timeout (default: %(default)s)",
    )
    parser.add_argument(
        "--no-enable",
        action="store_true",
        help="Do NOT try to toggle any enable/arm param on the firmware.",
    )
    parser.add_argument(
        "--disable-after",
        action="store_true",
        help="After stopping (sending zeros), try to disable/arm-off param.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print actions without connecting.",
    )

    args = parser.parse_args()
    m = [clamp(v) for v in args.m]
    rate = max(1.0, float(args.rate))
    interval = 1.0 / rate

    # Lazy import to allow --dry-run without cflib installed
    global Crazyflie, SyncCrazyflie, init_drivers
    try:
        from cflib.crazyflie import Crazyflie
        from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
        from cflib.crtp import init_drivers
    except ModuleNotFoundError:
        if args.dry_run:
            print(f"[DRY] Would connect to {args.uri}")
            if not args.no_enable:
                print("[DRY] Would try to set an enable param to 1 (pwm.enable or motorPowerSet.enable)")
            print(f"[DRY] Would send 4×PWM {m} at {rate:.1f} Hz for {args.hold:.2f} s")
            print("[DRY] Would then send zeros once")
            if args.disable_after and not args.no_enable:
                print("[DRY] Would try to set the enable param back to 0")
            return 0
        print("cflib not installed. Run: pip install cflib")
        return 1

    if args.dry_run:
        print(f"[DRY] Would connect to {args.uri}")
        if not args.no_enable:
            print("[DRY] Would try to set an enable param to 1 (pwm.enable or motorPowerSet.enable)")
        print(f"[DRY] Would send 4×PWM {m} at {rate:.1f} Hz for {args.hold:.2f} s")
        print("[DRY] Would then send zeros once")
        if args.disable_after and not args.no_enable:
            print("[DRY] Would try to set the enable param back to 0")
        return 0

    init_drivers()
    print(f"Connecting to {args.uri} ...")
    try:
        with SyncCrazyflie(args.uri, cf=Crazyflie(rw_cache="./cache")) as scf:
            cf = scf.cf

            used_param = ""
            if not args.no_enable:
                used_param = try_set_enable(cf, 1)
                if used_param:
                    print(f"[ENABLE] Set {used_param}=1")
                else:
                    print("[WARN] Could not set any known enable param. "
                          "Make sure the firmware-side PWM enable is ON.")

            # Short grace time for params to take effect
            time.sleep(0.1)

            print(f"Sending 4×PWM {m} at {rate:.1f} Hz for {args.hold:.2f} s ...")
            t_end = time.monotonic() + max(0.0, float(args.hold))
            # Stream at fixed rate to satisfy firmware timeout logic
            while time.monotonic() < t_end:
                send_4pwm_packet(cf, *m)
                time.sleep(interval)

            print("Stopping (send zeros once) ...")
            send_4pwm_packet(cf, 0, 0, 0, 0)

            if args.disable_after and used_param:
                try_set_enable(cf, 0)
                print(f"[DISABLE] Set {used_param}=0")

            print("Done.")
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr)
        return 1
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
