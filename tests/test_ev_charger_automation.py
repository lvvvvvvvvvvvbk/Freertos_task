import json
import queue
import threading
import time
import unittest
from dataclasses import dataclass
from http.server import BaseHTTPRequestHandler, HTTPServer
from typing import Dict, List, Optional, Tuple
from urllib import request, error


# -----------------------------
# CAN + GB/T27930 test harness
# -----------------------------
@dataclass
class CANFrame:
    can_id: int
    data: bytes


class FakeCANBus:
    def __init__(self) -> None:
        self._rx = queue.Queue()

    def inject_rx(self, frame: CANFrame) -> None:
        self._rx.put(frame)

    def poll(self, timeout_s: float = 0.0) -> Optional[CANFrame]:
        try:
            return self._rx.get(timeout=timeout_s)
        except queue.Empty:
            return None


class GBT27930Parser:
    MAP: Dict[int, str] = {
        0x1801F456: "EVT_HANDSHAKE_OK",
        0x1802F456: "EVT_PARAM_CONFIG_OK",
        0x1807F456: "EVT_STOP_CHARGE",
    }

    @classmethod
    def map_event(cls, frame: CANFrame) -> str:
        return cls.MAP.get(frame.can_id, "EVT_NONE")


# --------------------------------------
# DL/T645 meter frame + checksum harness
# --------------------------------------
class DLT645:
    @staticmethod
    def checksum(data: bytes) -> int:
        return sum(data) & 0xFF

    @staticmethod
    def verify(frame: bytes) -> bool:
        if len(frame) < 2:
            return False
        return DLT645.checksum(frame[:-1]) == frame[-1]

    @staticmethod
    def build_frame(payload: bytes) -> bytes:
        head = bytes([0x68, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x68])
        body = payload
        raw = head + body
        return raw + bytes([DLT645.checksum(raw)])


class FakeMeter:
    def __init__(self, energy_wh: int = 1234, voltage_mv: int = 380000, current_ma: int = 31500) -> None:
        self.energy_wh = energy_wh
        self.voltage_mv = voltage_mv
        self.current_ma = current_ma

    def read_frame(self) -> bytes:
        payload = f"E={self.energy_wh};V={self.voltage_mv};I={self.current_ma}".encode("ascii")
        return DLT645.build_frame(payload)


# ------------------------
# Cloud HTTP server/client
# ------------------------
class _TelemetryRequestHandler(BaseHTTPRequestHandler):
    store: List[dict] = []
    fail_first_n: int = 0

    def do_POST(self):  # noqa: N802 (BaseHTTPRequestHandler naming)
        if self.path != "/telemetry":
            self.send_response(404)
            self.end_headers()
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)

        if _TelemetryRequestHandler.fail_first_n > 0:
            _TelemetryRequestHandler.fail_first_n -= 1
            self.send_response(503)
            self.end_headers()
            return

        try:
            payload = json.loads(raw.decode("utf-8"))
        except json.JSONDecodeError:
            self.send_response(400)
            self.end_headers()
            return

        _TelemetryRequestHandler.store.append(payload)
        self.send_response(200)
        self.end_headers()

    def log_message(self, fmt, *args):
        return


class FakeCloudServer:
    def __init__(self):
        self.httpd: Optional[HTTPServer] = None
        self.thread: Optional[threading.Thread] = None
        self.port: Optional[int] = None

    def start(self):
        _TelemetryRequestHandler.store = []
        _TelemetryRequestHandler.fail_first_n = 0
        self.httpd = HTTPServer(("127.0.0.1", 0), _TelemetryRequestHandler)
        self.port = self.httpd.server_port
        self.thread = threading.Thread(target=self.httpd.serve_forever, daemon=True)
        self.thread.start()

    def stop(self):
        if self.httpd is not None:
            self.httpd.shutdown()
            self.httpd.server_close()
        if self.thread is not None:
            self.thread.join(timeout=2)

    def set_fail_first(self, n: int):
        _TelemetryRequestHandler.fail_first_n = n

    @property
    def received(self) -> List[dict]:
        return list(_TelemetryRequestHandler.store)


class CloudClient:
    def __init__(self, base_url: str, retries: int = 3, retry_delay_s: float = 0.05) -> None:
        self.base_url = base_url.rstrip("/")
        self.retries = retries
        self.retry_delay_s = retry_delay_s
        self.offline_cache: List[dict] = []

    def _post_json(self, path: str, payload: dict) -> bool:
        req = request.Request(
            self.base_url + path,
            data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        try:
            with request.urlopen(req, timeout=1.0) as resp:
                return 200 <= resp.status < 300
        except (error.URLError, error.HTTPError, TimeoutError):
            return False

    def report_telemetry(self, payload: dict) -> bool:
        for _ in range(self.retries + 1):
            if self._post_json("/telemetry", payload):
                return True
            time.sleep(self.retry_delay_s)

        self.offline_cache.append(payload)
        return False

    def flush_offline_cache(self) -> Tuple[int, int]:
        sent = 0
        kept: List[dict] = []
        for item in self.offline_cache:
            if self._post_json("/telemetry", item):
                sent += 1
            else:
                kept.append(item)
        self.offline_cache = kept
        return sent, len(kept)


# ------------------------
# End-to-end test harness
# ------------------------
class EVChargerAutomationHarness:
    def __init__(self, can_bus: FakeCANBus, meter: FakeMeter, cloud: CloudClient):
        self.can_bus = can_bus
        self.meter = meter
        self.cloud = cloud
        self.events: List[str] = []

    def poll_can_once(self):
        frame = self.can_bus.poll(timeout_s=0.01)
        if frame is None:
            return None
        evt = GBT27930Parser.map_event(frame)
        self.events.append(evt)
        return evt

    def read_meter_once(self) -> dict:
        frame = self.meter.read_frame()
        if not DLT645.verify(frame):
            raise ValueError("DL/T645 checksum verify failed")

        payload_text = frame[8:-1].decode("ascii")
        kv = dict(item.split("=") for item in payload_text.split(";"))
        return {
            "energy_wh": int(kv["E"]),
            "voltage_mv": int(kv["V"]),
            "current_ma": int(kv["I"]),
        }

    def report_cycle(self, state: str) -> bool:
        meter_data = self.read_meter_once()
        data = {
            "ts": int(time.time() * 1000),
            "state": state,
            **meter_data,
            "events": self.events[-5:],
        }
        return self.cloud.report_telemetry(data)


# ------------------------
# Tests
# ------------------------
class TestCANAndGBT27930(unittest.TestCase):
    def test_can_frame_maps_to_expected_event(self):
        bus = FakeCANBus()
        bus.inject_rx(CANFrame(can_id=0x1801F456, data=b"\x01\x02"))
        frame = bus.poll(timeout_s=0.01)
        self.assertIsNotNone(frame)
        self.assertEqual("EVT_HANDSHAKE_OK", GBT27930Parser.map_event(frame))

    def test_unknown_can_id_maps_to_none(self):
        frame = CANFrame(can_id=0x123, data=b"\x00")
        self.assertEqual("EVT_NONE", GBT27930Parser.map_event(frame))


class TestMeterDLT645(unittest.TestCase):
    def test_meter_frame_checksum_ok(self):
        meter = FakeMeter()
        frame = meter.read_frame()
        self.assertTrue(DLT645.verify(frame))

    def test_meter_frame_checksum_fail_when_corrupted(self):
        meter = FakeMeter()
        frame = bytearray(meter.read_frame())
        frame[10] ^= 0xFF
        self.assertFalse(DLT645.verify(bytes(frame)))


class TestCloudReporting(unittest.TestCase):
    def setUp(self):
        self.server = FakeCloudServer()
        self.server.start()
        self.client = CloudClient(f"http://127.0.0.1:{self.server.port}", retries=2, retry_delay_s=0.01)

    def tearDown(self):
        self.server.stop()

    def test_cloud_receives_telemetry(self):
        ok = self.client.report_telemetry({"state": "CHARGING", "energy_wh": 10})
        self.assertTrue(ok)
        self.assertEqual(1, len(self.server.received))
        self.assertEqual("CHARGING", self.server.received[0]["state"])

    def test_retry_then_success(self):
        self.server.set_fail_first(1)
        ok = self.client.report_telemetry({"state": "PARAM_CONFIG", "energy_wh": 20})
        self.assertTrue(ok)
        self.assertEqual(1, len(self.server.received))

    def test_offline_cache_and_flush(self):
        bad_client = CloudClient("http://127.0.0.1:1", retries=1, retry_delay_s=0.01)
        ok = bad_client.report_telemetry({"state": "FAULT", "energy_wh": 0})
        self.assertFalse(ok)
        self.assertEqual(1, len(bad_client.offline_cache))

        good_client = CloudClient(f"http://127.0.0.1:{self.server.port}", retries=1, retry_delay_s=0.01)
        good_client.offline_cache = bad_client.offline_cache.copy()
        sent, remain = good_client.flush_offline_cache()
        self.assertEqual(1, sent)
        self.assertEqual(0, remain)


class TestEndToEndHarness(unittest.TestCase):
    def setUp(self):
        self.server = FakeCloudServer()
        self.server.start()

    def tearDown(self):
        self.server.stop()

    def test_can_meter_cloud_integration(self):
        bus = FakeCANBus()
        meter = FakeMeter(energy_wh=8888, voltage_mv=410000, current_ma=60000)
        cloud = CloudClient(f"http://127.0.0.1:{self.server.port}", retries=1, retry_delay_s=0.01)
        harness = EVChargerAutomationHarness(bus, meter, cloud)

        bus.inject_rx(CANFrame(0x1801F456, b"\x10\x20"))
        bus.inject_rx(CANFrame(0x1802F456, b"\x30\x40"))

        self.assertEqual("EVT_HANDSHAKE_OK", harness.poll_can_once())
        self.assertEqual("EVT_PARAM_CONFIG_OK", harness.poll_can_once())

        ok = harness.report_cycle("CHARGING")
        self.assertTrue(ok)
        self.assertEqual(1, len(self.server.received))

        payload = self.server.received[0]
        self.assertEqual("CHARGING", payload["state"])
        self.assertEqual(8888, payload["energy_wh"])
        self.assertEqual(410000, payload["voltage_mv"])
        self.assertEqual(60000, payload["current_ma"])
        self.assertIn("EVT_HANDSHAKE_OK", payload["events"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
