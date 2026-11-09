"""
Async aiohttp Server hosting Tactical Radar Web HUD & WebSocket stream.
"""

import asyncio
import json
import os
import time
from typing import Dict, List, Optional, Set
from urllib.parse import urlparse
from aiohttp import web

from radar_gui.protocol_decoders import FrameDecoder, RadarContact, zone_of
from radar_gui.serial_reader import open_serial, print_serial_error, simulated_contacts
from sensor_sim.scenarios import get_scenario


class RadarWebServer:
    """Async web server providing HTTP static HUD assets and WebSocket telemetry stream."""

    def __init__(
        self,
        host: str = "127.0.0.1",
        port: int = 8080,
        demo_mode: bool = False,
        serial_port: Optional[str] = None,
        baud_rate: int = 256000,
    ):
        self.host = host
        self.port = port
        self.demo_mode = demo_mode
        self.serial_port = serial_port
        self.baud_rate = baud_rate

        self.app = web.Application()
        self._sockets: Set[web.WebSocketResponse] = set()
        self._latest_contacts: List[Dict] = []
        self._running = False
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._runner: Optional[web.AppRunner] = None
        self._serial_task: Optional[asyncio.Task] = None
        self._demo_task: Optional[asyncio.Task] = None

        self._setup_routes()

    def _setup_routes(self) -> None:
        static_dir = os.path.join(os.path.dirname(__file__), "static")
        self.app.router.add_get("/", self._handle_index)
        self.app.router.add_get("/ws", self._handle_ws)
        self.app.router.add_static("/static/", path=static_dir, name="static")

    async def _handle_index(self, request: web.Request) -> web.Response:
        index_path = os.path.join(os.path.dirname(__file__), "static", "index.html")
        if os.path.exists(index_path):
            with open(index_path, "r", encoding="utf-8") as f:
                return web.Response(text=f.read(), content_type="text/html")
        return web.Response(text="<h1>Tactical Radar HUD — static/index.html not found</h1>", content_type="text/html")

    async def _handle_ws(self, request: web.Request) -> web.WebSocketResponse:
        # Browsers always send Origin: stop other sites open in the browser from reading the live feed
        origin = request.headers.get("Origin")
        if origin is not None and urlparse(origin).netloc != request.host:
            raise web.HTTPForbidden()

        ws = web.WebSocketResponse(heartbeat=15.0)
        await ws.prepare(request)
        self._sockets.add(ws)

        # Send initial snapshot immediately upon connection
        initial_msg = json.dumps({
            "type": "radar_frame",
            "timestamp": time.monotonic(),
            "contacts": self._latest_contacts,
        })
        await ws.send_str(initial_msg)

        try:
            async for msg in ws:
                # Can receive client control messages (e.g., ping or config)
                pass
        finally:
            self._sockets.discard(ws)

        return ws

    def _contact_to_dict(self, contact: RadarContact) -> Dict:
        """Serializes RadarContact for the browser HUD."""
        return {
            "id": contact.id,
            "x": round(contact.x, 3),
            "y": round(contact.y, 3),
            "speed": round(contact.speed, 2),
            "distance": round(contact.distance, 3),
            "angle_deg": round(contact.angle_deg, 1),
            "threat_zone": zone_of(contact),
        }

    async def broadcast_contacts(self, contacts: List[RadarContact]) -> None:
        """Broadcasts contacts to all connected WebSocket clients."""
        payload = [self._contact_to_dict(c) for c in contacts]
        self._latest_contacts = payload

        if not self._sockets:
            return

        msg = json.dumps({
            "type": "radar_frame",
            "timestamp": time.monotonic(),
            "contacts": payload,
        })

        dead_sockets = set()
        for ws in self._sockets:
            try:
                await ws.send_str(msg)
            except Exception:
                dead_sockets.add(ws)

        self._sockets.difference_update(dead_sockets)

    async def _demo_producer(self) -> None:
        """Streams the sensor_sim 'multi' scenario as live contacts."""
        scenario = get_scenario("multi")
        while self._running:
            await self.broadcast_contacts(simulated_contacts(scenario, 0.05))
            await asyncio.sleep(0.05)  # 20 Hz update rate

    async def _serial_producer(self) -> None:
        """Reads raw binary or JSON frames from physical serial or virtual PTY."""
        while self._running:
            ser = None
            try:
                ser = open_serial(self.serial_port, self.baud_rate, timeout=0.05)
                print(f"[RadarWeb] Connected to {self.serial_port} @ {ser.baudrate}")
                buffer = bytearray()

                while self._running:
                    # Non-blocking read in async loop
                    data = await asyncio.to_thread(ser.read, 64)
                    if data:
                        buffer.extend(data)
                        contacts, buffer = FrameDecoder.parse_stream(buffer)
                        if contacts:
                            await self.broadcast_contacts(contacts)
                    else:
                        await asyncio.sleep(0.01)

            except Exception as e:
                print_serial_error("RadarWeb", self.serial_port, e)
                await asyncio.sleep(1.0)  # retry
            finally:
                if ser:
                    try:
                        ser.close()
                    except Exception:
                        pass

    async def start(self) -> None:
        """Starts web server and data ingestion tasks."""
        self._running = True
        self._loop = asyncio.get_running_loop()

        self._runner = web.AppRunner(self.app)
        await self._runner.setup()

        site = web.TCPSite(self._runner, self.host, self.port)
        try:
            await site.start()
        except OSError as e:
            if getattr(e, "errno", None) == 98:
                fallback_port = self.port + 1 if self.port != 8080 else 8088
                print(f"[RadarWeb] Port {self.port} in use, falling back to {fallback_port}...")
                self.port = fallback_port
                site = web.TCPSite(self._runner, self.host, self.port)
                await site.start()
            else:
                raise

        print("=" * 60)
        print(f"  🎯 Tactical Radar Web HUD active!")
        print(f"  🌐 Browser Display URL: http://localhost:{self.port}")
        if self.host == "0.0.0.0":
            print(f"  📱 Network Access:      http://<your-ip>:{self.port}")
        print("=" * 60)

        if self.demo_mode:
            self._demo_task = asyncio.create_task(self._demo_producer())
        elif self.serial_port:
            self._serial_task = asyncio.create_task(self._serial_producer())

    async def stop(self) -> None:
        """Gracefully shuts down server and background tasks."""
        self._running = False
        if self._demo_task:
            self._demo_task.cancel()
            try:
                await self._demo_task
            except asyncio.CancelledError:
                pass

        if self._serial_task:
            self._serial_task.cancel()
            try:
                await self._serial_task
            except asyncio.CancelledError:
                pass

        # Close all active websockets
        for ws in list(self._sockets):
            await ws.close()
        self._sockets.clear()

        if self._runner:
            await self._runner.cleanup()
        print("[RadarWeb] Server stopped.")
