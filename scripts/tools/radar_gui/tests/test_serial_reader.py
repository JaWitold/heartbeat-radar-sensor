"""
Unit tests for open_serial() transports used by both the desktop and web HUDs.
"""

import socket
import threading
import unittest

from radar_gui.protocol_decoders import FrameDecoder
from radar_gui.serial_reader import open_serial

FRAME = (
    b'{"type":"radar_frame","sweep_deg":0.0,"alert":"Danger","severity":0.80,"targets":'
    b'[{"id":1,"x":0.500,"y":1.000,"speed":-0.25,"distance":1.118,"threat_zone":"Danger"}]}\n'
)


class TestOpenSerialSocket(unittest.TestCase):

    def test_socket_url_reads_wifi_telemetry(self):
        """socket://host:port (WifiDisplayAdapter / simulator --socket) works like a serial port."""
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(("127.0.0.1", 0))
        server.listen(1)
        port = server.getsockname()[1]

        def serve_one_frame():
            conn, _ = server.accept()
            with conn:
                conn.sendall(FRAME)

        sender = threading.Thread(target=serve_one_frame, daemon=True)
        sender.start()
        try:
            link = open_serial(f"socket://127.0.0.1:{port}", 115200, timeout=1.0)
            try:
                data = link.read(len(FRAME))
            finally:
                link.close()
        finally:
            sender.join(timeout=2.0)
            server.close()

        contacts, _ = FrameDecoder.parse_stream(bytearray(data))
        self.assertEqual(len(contacts), 1)
        self.assertEqual(contacts[0].id, 1)
        self.assertAlmostEqual(contacts[0].y, 1.0, places=3)


if __name__ == "__main__":
    unittest.main()
