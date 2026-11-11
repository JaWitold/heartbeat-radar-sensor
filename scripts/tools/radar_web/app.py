"""
Main entrypoint for Tactical Radar Web HUD server.
"""

import argparse
import asyncio
import os
import signal
import sys
from typing import Optional
from .server import RadarWebServer


def parse_args():
    parser = argparse.ArgumentParser(
        description="Tactical Heartbeat Radar Web HUD Server"
    )
    parser.add_argument(
        "--http-port",
        type=int,
        default=8088,
        help="HTTP/WebSocket port to bind web server (default: 8088)",
    )
    parser.add_argument(
        "--host",
        type=str,
        default="127.0.0.1",
        help="Host address to bind (default: 127.0.0.1; use 0.0.0.0 to share the live feed on your network)",
    )
    parser.add_argument(
        "--serial",
        type=str,
        help="Serial port device (e.g. /dev/ttyUSB0 or /dev/pts/17)",
    )
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Expect raw 256000 baud HLK-LD2450 binary frames",
    )
    parser.add_argument(
        "--baud",
        type=int,
        default=0,
        help="Baud rate (default: 256000 if --raw else 921600)",
    )
    parser.add_argument(
        "--demo",
        action="store_true",
        help="Run in standalone demo mode with animated targets",
    )
    parser.add_argument(
        "--pty-file",
        type=str,
        default="/tmp/hrs_radar_pty",
        help="Auto-read serial device from file written by sim-radar (default: /tmp/hrs_radar_pty)",
    )
    return parser.parse_args()


async def run_server(args):
    serial_port = args.serial

    # If neither serial nor demo is specified, check if /tmp/hrs_radar_pty exists
    if not serial_port and not args.demo and os.path.exists(args.pty_file):
        try:
            with open(args.pty_file, "r") as f:
                detected = f.read().strip()
                if detected and (os.path.exists(detected) or detected.startswith("/dev/pts/")):
                    serial_port = detected
                    print(f"[RadarWeb] Auto-detected virtual PTY: {serial_port}")
        except Exception:
            pass

    # If still neither serial nor demo, default to demo mode for instant usability
    demo_mode = args.demo
    if not serial_port and not demo_mode:
        print("[RadarWeb] No serial device specified or detected; launching in DEMO mode.")
        demo_mode = True

    baud_rate = args.baud
    if baud_rate <= 0:
        baud_rate = 256000 if args.raw else 921600

    server = RadarWebServer(
        host=args.host,
        port=args.http_port,
        demo_mode=demo_mode,
        serial_port=serial_port,
        baud_rate=baud_rate,
    )

    await server.start()

    stop_event = asyncio.Event()

    def _sig_handler():
        stop_event.set()

    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, _sig_handler)
        except NotImplementedError:
            pass

    await stop_event.wait()
    await server.stop()


def main():
    args = parse_args()
    try:
        asyncio.run(run_server(args))
    except KeyboardInterrupt:
        print("\n[RadarWeb] Terminated by user.")


if __name__ == "__main__":
    main()
