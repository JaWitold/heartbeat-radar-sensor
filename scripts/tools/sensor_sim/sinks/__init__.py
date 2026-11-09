"""
Output sinks registry.
"""

from .serial_sink import SerialPortSink
from .pty_sink import PtySink
from .socket_sink import SocketSink

__all__ = ["SerialPortSink", "PtySink", "SocketSink"]
