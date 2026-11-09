"""
TCP/UDP Socket Sink for network or Wokwi socket bridge simulation.
"""

import socket
from ..interfaces import IOutputSink


class SocketSink(IOutputSink):
    """Listens on a TCP port and streams radar frames to connected clients."""

    def __init__(self, host: str = "127.0.0.1", port: int = 9999):
        self._host = host
        self._port = port
        self._server_socket = None
        self._client_socket = None

    def open(self) -> None:
        self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server_socket.bind((self._host, self._port))
        self._server_socket.listen(1)
        self._server_socket.settimeout(0.5)
        print(f"[SocketSink] Listening on {self._host}:{self._port}...")

    def write(self, data: bytes) -> None:
        if self._client_socket is None and self._server_socket:
            try:
                self._client_socket, addr = self._server_socket.accept()
                print(f"[SocketSink] Client connected from {addr}")
            except socket.timeout:
                return

        if self._client_socket:
            try:
                self._client_socket.sendall(data)
            except (BrokenPipeError, ConnectionResetError):
                print("[SocketSink] Client disconnected.")
                self._client_socket = None

    def close(self) -> None:
        if self._client_socket:
            self._client_socket.close()
            self._client_socket = None
        if self._server_socket:
            self._server_socket.close()
            self._server_socket = None
        print("[SocketSink] Socket closed.")
