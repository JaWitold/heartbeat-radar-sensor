#!/usr/bin/env python3
"""
Hexagonal Sensor Simulator CLI.
Generates binary radar protocol streams for testing without hardware.
"""

import argparse
import sys
import time

from .interfaces import IOutputSink
from .protocols import get_protocol
from .scenarios import get_scenario
from .sinks import PtySink, SerialPortSink, SocketSink


def parse_args():
    parser = argparse.ArgumentParser(
        description="Hexagonal Radar Sensor Simulator CLI"
    )
    parser.add_argument(
        "--sensor",
        type=str,
        default="ld2450",
        help="Sensor model to simulate (ld2450, rd03d, rcwl0516). Default: ld2450",
    )
    parser.add_argument(
        "--scenario",
        type=str,
        default="patrol",
        help="Motion scenario (patrol, multi, interactive). Default: patrol",
    )
    parser.add_argument(
        "--rate",
        type=float,
        default=10.0,
        help="Reporting frame rate in Hz. Default: 10.0",
    )
    parser.add_argument(
        "--count",
        type=int,
        default=0,
        help="Number of frames to emit (0 = infinite). Default: 0",
    )

    # Mutually exclusive sink choices
    sink_group = parser.add_mutually_exclusive_group()
    sink_group.add_argument(
        "--port",
        type=str,
        help="Physical serial port device (e.g. /dev/ttyUSB0)",
    )
    sink_group.add_argument(
        "--pty",
        action="store_true",
        help="Create a Linux virtual pseudo-terminal (/dev/pts/*)",
    )
    sink_group.add_argument(
        "--socket",
        type=int,
        metavar="PORT",
        help="Stream frames over TCP server on specified port",
    )
    parser.add_argument(
        "--baud",
        type=int,
        default=0,
        help="Override sensor default baud rate",
    )
    parser.add_argument(
        "--pty-file",
        type=str,
        default="/tmp/hrs_radar_pty",
        help="File path to store virtual PTY slave node (default: /tmp/hrs_radar_pty)",
    )

    return parser.parse_args()


def main():
    args = parse_args()

    protocol = get_protocol(args.sensor)
    scenario = get_scenario(args.scenario)
    baud_rate = args.baud if args.baud > 0 else protocol.default_baud_rate

    # Select output sink
    sink: IOutputSink
    if args.port:
        sink = SerialPortSink(args.port, baud_rate)
    elif args.socket:
        sink = SocketSink(port=args.socket)
    else:
        # Default to PTY on Linux
        sink = PtySink(pty_file=args.pty_file)

    sink.open()

    interval = 1.0 / max(0.1, args.rate)
    print(f"[*] Simulating: {protocol.name}")
    print(f"[*] Scenario:   {args.scenario}")
    print(f"[*] Frame Rate: {args.rate} Hz ({interval * 1000.0:.1f}ms interval)")
    print(f"[*] Press Ctrl+C to stop.\n")

    frames_sent = 0
    last_time = time.monotonic()

    try:
        while True:
            now = time.monotonic()
            dt = now - last_time
            last_time = now

            targets = scenario.update(dt)
            frame_bytes = protocol.encode_frame(targets)
            sink.write(frame_bytes)
            frames_sent += 1

            if args.count > 0 and frames_sent >= args.count:
                print(f"\n[+] Completed requested {args.count} frames.")
                break

            # Sleep to match desired frame rate
            elapsed = time.monotonic() - now
            sleep_time = interval - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)

    except KeyboardInterrupt:
        print("\n[!] Simulation stopped by user.")
    finally:
        sink.close()


if __name__ == "__main__":
    main()
