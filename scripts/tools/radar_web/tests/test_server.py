"""
Unit tests for RadarWebServer routes and contact serialization.
"""

import asyncio
import unittest
import aiohttp
from radar_gui.protocol_decoders import RadarContact
from radar_web.server import RadarWebServer


class TestRadarWebServer(unittest.IsolatedAsyncioTestCase):

    async def asyncSetUp(self):
        self.server = RadarWebServer(port=8899, demo_mode=True)
        await self.server.start()

    async def asyncTearDown(self):
        await self.server.stop()

    def test_routes_configured(self):
        routes = [r.resource.canonical for r in self.server.app.router.routes()]
        self.assertIn("/", routes)
        self.assertIn("/ws", routes)
        # Verify static directory route
        static_routes = [
            r.resource.canonical for r in self.server.app.router.routes() if "static" in r.resource.canonical
        ]
        self.assertTrue(len(static_routes) > 0)

    async def test_http_and_websocket_endpoints(self):
        async with aiohttp.ClientSession() as session:
            # Test HTML
            async with session.get("http://localhost:8899/") as resp:
                self.assertEqual(resp.status, 200)
                html = await resp.text()
                self.assertIn("TACTICAL HEARTBEAT RADAR", html)

            # Test CSS
            async with session.get("http://localhost:8899/static/style.css") as resp:
                self.assertEqual(resp.status, 200)
                css = await resp.text()
                self.assertIn("--phosphor-green", css)

            # Test JS
            async with session.get("http://localhost:8899/static/radar.js") as resp:
                self.assertEqual(resp.status, 200)
                js = await resp.text()
                self.assertIn("drawRadar", js)

            # Test WebSocket
            async with session.ws_connect("http://localhost:8899/ws") as ws:
                msg = await ws.receive_json()
                self.assertEqual(msg["type"], "radar_frame")
                self.assertIsInstance(msg["contacts"], list)

    async def test_websocket_rejects_foreign_origin(self):
        async with aiohttp.ClientSession() as session:
            with self.assertRaises(aiohttp.WSServerHandshakeError) as ctx:
                await session.ws_connect("http://localhost:8899/ws", origin="http://evil.example")
            self.assertEqual(ctx.exception.status, 403)

    def test_default_host_is_loopback_only(self):
        self.assertEqual(RadarWebServer().host, "127.0.0.1")

    def test_contact_serialization_and_threat_zones(self):
        # Danger contact (< 1.5m)
        danger_contact = RadarContact(
            id=1, x=0.5, y=1.2, speed=-0.5, distance=1.3, angle_deg=22.6, timestamp=100.0
        )
        d_dict = self.server._contact_to_dict(danger_contact)
        self.assertEqual(d_dict["id"], 1)
        self.assertEqual(d_dict["threat_zone"], "Danger")
        self.assertEqual(d_dict["distance"], 1.3)

        # Caution contact (1.5m to 3.5m)
        caution_contact = RadarContact(
            id=2, x=1.0, y=2.0, speed=0.1, distance=2.236, angle_deg=26.6, timestamp=100.0
        )
        c_dict = self.server._contact_to_dict(caution_contact)
        self.assertEqual(c_dict["id"], 2)
        self.assertEqual(c_dict["threat_zone"], "Caution")

        # Safe contact (> 3.5m)
        safe_contact = RadarContact(
            id=3, x=2.0, y=4.0, speed=0.0, distance=4.472, angle_deg=26.6, timestamp=100.0
        )
        s_dict = self.server._contact_to_dict(safe_contact)
        self.assertEqual(s_dict["id"], 3)
        self.assertEqual(s_dict["threat_zone"], "Safe")


if __name__ == "__main__":
    unittest.main()
