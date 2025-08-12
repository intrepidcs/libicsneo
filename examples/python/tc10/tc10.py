import icsneopy
import argparse


def main():
    parser = get_parser()
    args = parser.parse_args()
    run_test(args)


def find_device(serial: str) -> icsneopy.Device:
    devices = icsneopy.find_all_devices()
    for d in devices:
        if d.get_serial() == serial:
            print(f"opening device {serial}")
            return d
    return None


def run_test(args):
    # find the device
    d = find_device(args.serial)
    if d is None:
        print(f"error: unable to find device {args.serial}")
        exit(1)

    # open the device
    if not d.open():
        print(f"error: unable to open device {args.serial}")
        exit(1)

    # check if TC10 is supported
    if not d.supports_tc10():
        print(f"error: device does not support TC10 {args.serial}")
        exit(1)

    # send the request on all networks
    for n in args.networks:
        net = getattr(icsneopy.Network.NetID, n)
        if args.send_wake:
            print(f"requesting TC10 wake on network {net}")
            if not d.request_tc10_wake(net):
                print(f"error: unable to send TC10 wake on device {args.serial}")
                exit(1)
        elif args.send_sleep:
            print(f"requesting TC10 sleep on network {net}")
            if not d.request_tc10_sleep(net):
                print(f"error: unable to send TC10 sleep on device {args.serial}")
                exit(1)

    # close the device
    print(f"closing device {args.serial}")
    d.close()


def get_parser():
    parser = argparse.ArgumentParser(description="TC10 wake request")
    parser.add_argument(
        "serial",
        help="The serial number of the device",
    )
    parser.add_argument(
        "--networks",
        nargs="+",
        help="List of icsneopy networks to use.  Multiple networks accepted, e.g. '--networks ETHERNET_01 AE_01'",
        required=True,
    ),
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--send-wake",
        help="Trigger TC10 wake on the selected networks",
        action="store_true",
        default=False,
    )
    group.add_argument(
        "--send-sleep",
        help="Trigger TC10 sleep on the selected networks",
        action="store_true",
        default=False,
    )
    return parser


if __name__ == "__main__":
    main()
