"""
Unit test for SerialPortSink error handling and permissions guidance.
"""

import unittest
from unittest.mock import MagicMock, patch
from sensor_sim.sinks.serial_sink import SerialPortSink


class TestSerialPortSink(unittest.TestCase):

    @patch("serial.Serial")
    def test_permission_error_guidance(self, mock_serial):
        import serial

        mock_serial.side_effect = serial.SerialException(
            "[Errno 13] could not open port /dev/ttyUSB0: [Errno 13] Permission denied: '/dev/ttyUSB0'"
        )

        sink = SerialPortSink(port="/dev/ttyUSB0", baud_rate=256000)

        with self.assertRaises(PermissionError) as ctx:
            sink.open()

        self.assertIn("Permission denied", str(ctx.exception))
        self.assertIn("sudo usermod -a -G dialout $USER", str(ctx.exception))

    @patch("serial.Serial")
    def test_successful_open_and_write(self, mock_serial):
        mock_instance = MagicMock()
        mock_instance.is_open = True
        mock_serial.return_value = mock_instance

        sink = SerialPortSink(port="/dev/ttyUSB0", baud_rate=256000)
        sink.open()
        mock_serial.assert_called_once()

        sink.write(b"\xaa\xff")
        mock_instance.write.assert_called_once_with(b"\xaa\xff")

        sink.close()
        mock_instance.close.assert_called_once()


if __name__ == "__main__":
    unittest.main()
