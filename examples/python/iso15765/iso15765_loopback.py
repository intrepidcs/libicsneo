"""
Two-device ISO-TP loopback on CAN1.

Firmware segments the payload. This script sends it once and prints CAN1
frames with the ISO-TP PCI nibble decoded (SF/FF/CF/FC).

Wire CAN1 of two devices together (CANH/CANL, terminated).

  python iso15765_loopback.py --raw
  python iso15765_loopback.py --sf
  python iso15765_loopback.py --both
"""

import argparse
import threading
import time
from datetime import timedelta

import icsneopy


CAN1 = icsneopy.Network.NetID.DWCAN_01


def pci_describe(data):
	if not data:
		return ""
	pci_type = data[0] >> 4
	pci_low = data[0] & 0x0F
	if pci_type == 0x0:
		if pci_low > 7 or len(data) < 1 + pci_low:
			return ""
		return f"SF len={pci_low}"
	if pci_type == 0x1:
		if len(data) < 8:
			return ""
		length = (pci_low << 8) | data[1]
		if length < 8:
			return ""
		return f"FF len={length}"
	if pci_type == 0x2:
		if len(data) < 2:
			return ""
		return f"CF SN={pci_low}"
	if pci_type == 0x3:
		if len(data) < 3 or pci_low > 2:
			return ""
		fs_name = ("CTS", "WAIT", "OVFLW")[pci_low]
		return f"FC FS={fs_name} BS={data[1]} STmin=0x{data[2]:02X}"
	return ""


def print_api_result(what, ok):
	line = f"  {what}: {'OK' if ok else 'FAIL'}"
	if not ok:
		err = icsneopy.get_last_error()
		if err.get_type() != icsneopy.APIEvent.Type.NoErrorFound:
			line += f" [{err.describe()}]"
	print(line)


def is_extended(arb_id):
	return arb_id > 0x7FF


def make_args(tx_id, fc_id, payload, rx):
	msg = icsneopy.Iso15765MessageArgs()
	msg.tx_index = 0
	msg.arb_id = icsneopy.Iso15765MessageArgs.ArbId(tx_id, is_extended(tx_id))
	msg.flow_control_arb_id = icsneopy.Iso15765MessageArgs.ArbId(fc_id, is_extended(fc_id))
	msg.flow_control_arb_id_mask = 0x1FFFFFFF if is_extended(tx_id) else 0x7FF
	msg.is_can_fd = False
	msg.is_brs_enabled = False
	msg.tx_dl = 8
	msg.padding_value = 0x00
	msg.fs_timeout = 75
	msg.fs_wait_timeout = 150
	msg.cf_timeout = 150
	msg.is_flow_control_enabled = rx
	if payload:
		msg.data = payload
	return msg


def transmit_can(device, arb_id, data):
	frame = icsneopy.CANMessage()
	frame.network = icsneopy.Network(CAN1)
	frame.arbid = arb_id
	# RawMessage.data is vector<uint8_t>: a list of ints, not bytes.
	frame.data = list(data)
	frame.isExtended = is_extended(arb_id)
	frame.isCANFD = False
	ok = device.transmit(frame)
	if not ok:
		print_api_result("transmit CAN", ok)
	return ok


class Sniffer:
	def __init__(self, tag):
		self.tag = tag
		self.t0 = None
		self.lock = threading.Lock()

	def on_message(self, message):
		if not isinstance(message, icsneopy.CANMessage):
			return
		if message.network.get_type() != icsneopy.Network.Type.CAN:
			return
		with self.lock:
			if self.t0 is None:
				self.t0 = message.timestamp
			rel_ms = 0.0
			if message.timestamp >= self.t0:
				rel_ms = (message.timestamp - self.t0) / 1e6
			direction = "TX" if message.transmitted else "RX"
			width = 8 if is_extended(message.arbid) else 3
			data = " ".join(f"{b:02X}" for b in message.data)
			line = (
				f"  [{self.tag} {direction}] {rel_ms:8.3f} ms  "
				f"{message.network}  0x{message.arbid:0{width}X} "
				f"[{len(message.data)}]  {data}"
			)
			pci = pci_describe(message.data)
			if pci:
				line += f"  {pci}"
			print(line)


def open_and_online(device, baud, keep_baud):
	print(f"Opening {device.describe()} ...")
	if not device.open():
		print_api_result("open", False)
		return False
	if not keep_baud and device.settings is not None:
		net = icsneopy.Network(CAN1)
		if not device.settings.set_baudrate(net, baud):
			print_api_result("set_baudrate", False)
		elif not device.settings.apply(True):
			print_api_result("settings.apply(temporary)", False)
	if not device.go_online():
		print_api_result("go_online", False)
		device.close()
		return False
	return True


def close_device(device):
	if device is None:
		return
	device.iso15765_disable_all()
	if device.is_online():
		device.go_offline()
	if device.is_open():
		device.close()


def find_by_serial(devices, serial):
	for device in devices:
		if device.get_serial().lower() == serial.lower():
			return device
	return None


def parse_args():
	parser = argparse.ArgumentParser(
		description="Two Intrepid devices with CAN1 wired together (CANH/CANL, terminated)."
	)
	mode = parser.add_mutually_exclusive_group(required=True)
	mode.add_argument("--raw", action="store_true", help="Raw CAN ping-pong (cable check)")
	mode.add_argument("--sf", action="store_true", help="Firmware ISO-TP single-frame TX from tester")
	mode.add_argument("--both", action="store_true", help="Firmware ISO-TP TX on tester, firmware flow-control on ECU")
	parser.add_argument("--tester", default="", help="Tester (sender) serial. Optional if exactly two devices are found.")
	parser.add_argument("--ecu", default="", help="ECU / peer serial. Optional if exactly two devices are found.")
	parser.add_argument("--baud", type=int, default=500000, help="CAN1 baud, applied temporarily (default 500000)")
	parser.add_argument("--keep-baud", action="store_true", help="Do not change device baud")
	parser.add_argument("--tx-id", type=lambda s: int(s, 0), default=0x7E0, help="Tester ISO-TP CAN ID (default 0x7E0)")
	parser.add_argument("--fc-id", type=lambda s: int(s, 0), default=0x7E8, help="Flow-control CAN ID (default 0x7E8)")
	parser.add_argument("--bytes", type=int, default=None, help="ISO-TP payload size (sf default 5, both default 20)")
	parser.add_argument("--listen-ms", type=int, default=2000, help="Time to listen after the action (default 2000)")
	return parser.parse_args()


def main():
	args = parse_args()
	print(f"libicsneo {icsneopy.get_version()}")

	devices = icsneopy.find_all_devices()
	print(f"Found {len(devices)} device(s)")
	for device in devices:
		print(f"  {device.describe()} serial={device.get_serial()}")
	if len(devices) < 2:
		raise SystemExit("Need two devices connected.")

	if not args.tester and not args.ecu:
		if len(devices) != 2:
			raise SystemExit("More than two devices found; pass --tester and --ecu serials.")
		tester, ecu = devices[0], devices[1]
		print("No serials given; using first as tester, second as ECU.")
	else:
		if not args.tester or not args.ecu:
			raise SystemExit("Pass both --tester and --ecu, or neither.")
		tester = find_by_serial(devices, args.tester)
		ecu = find_by_serial(devices, args.ecu)
		if tester is None or ecu is None:
			raise SystemExit("Could not match tester/ECU serials.")

	payload_size = args.bytes if args.bytes is not None else (5 if args.sf else 20)
	payload = [i & 0xFF for i in range(payload_size)]

	print(f"Tester: {tester.describe()}")
	print(f"ECU:    {ecu.describe()}")
	if not args.raw:
		print(f"CAN1 TX 0x{args.tx_id:X}  FC 0x{args.fc_id:X}  payload {payload_size} bytes")
	print()

	network = icsneopy.Network(CAN1)
	try:
		if not open_and_online(tester, args.baud, args.keep_baud):
			return 1
		if not open_and_online(ecu, args.baud, args.keep_baud):
			return 1

		for device, sniffer in ((tester, Sniffer("TESTER")), (ecu, Sniffer("ECU   "))):
			callback = icsneopy.MessageCallback(sniffer.on_message, icsneopy.MessageFilter(CAN1))
			device.add_message_callback(callback)

		if args.raw:
			print("Mode --raw: tester 0x123, then ECU 0x456")
			transmit_can(tester, 0x123, [0x11, 0x22, 0x33, 0x44])
			time.sleep(0.2)
			transmit_can(ecu, 0x456, [0x55, 0x66, 0x77, 0x88])
		elif args.sf:
			print("Mode --sf: firmware ISO-TP single frame from tester")
			if len(payload) > 7:
				print("  warning: payload > 7 bytes will be a First Frame (no flow control in this mode)")
			ok = tester.iso15765_enable(network)
			print_api_result("tester iso15765_enable", ok)
			msg = make_args(args.tx_id, args.fc_id, payload, rx=False)
			ok = tester.iso15765_transmit_message(network, msg, timedelta(0))
			print_api_result("tester iso15765_transmit_message", ok)
		else:
			print("Mode --both: firmware TX on tester, firmware flow-control on ECU")
			ok = tester.iso15765_enable(network)
			print_api_result("tester iso15765_enable", ok)
			ok = ecu.iso15765_enable(network)
			print_api_result("ECU iso15765_enable", ok)
			msg = make_args(args.tx_id, args.fc_id, [], rx=True)
			ok = ecu.iso15765_setup_rx_flow_control(network, msg)
			print_api_result("ECU iso15765_setup_rx_flow_control", ok)
			time.sleep(0.1)
			msg = make_args(args.tx_id, args.fc_id, payload, rx=False)
			ok = tester.iso15765_transmit_message(network, msg, timedelta(0))
			print_api_result("tester iso15765_transmit_message", ok)

		time.sleep(args.listen_ms / 1000.0)
	finally:
		close_device(tester)
		close_device(ecu)
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
