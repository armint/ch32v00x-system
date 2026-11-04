#!/usr/bin/env python3
"""Host side of the CH32V307 USBHS loopback test (needs pyusb).

Finds the device 1209:0001, prints the speed, sends packets of several
sizes to the bulk OUT endpoint and checks that the bulk IN endpoint sends
the same bytes back. Exit code 0: all packets match.
"""

import os
import sys

import usb.core
import usb.util

VID, PID = 0x1209, 0x0001
EP_OUT, EP_IN = 0x01, 0x81
ROUNDS = 200


def main():
    dev = usb.core.find(idVendor=VID, idProduct=PID)
    if dev is None:
        print("FAIL: device 1209:0001 not found")
        return 1
    dev.set_configuration()
    intf = dev.get_active_configuration()[(0, 0)]
    name = usb.util.get_string(dev, intf.iInterface)
    mps = usb.util.find_descriptor(intf, bEndpointAddress=EP_OUT).wMaxPacketSize
    speed = {usb.util.SPEED_HIGH: "high", usb.util.SPEED_FULL: "full"}.get(dev.speed, str(dev.speed))
    print(f"interface '{name}', speed {speed}, bulk packet size {mps}")

    sizes = [1, 2, mps // 2, mps - 1, mps]
    for i in range(ROUNDS):
        size = sizes[i % len(sizes)]
        data = os.urandom(size)
        dev.write(EP_OUT, data, timeout=1000)
        back = bytes(dev.read(EP_IN, mps, timeout=1000))
        if back != data:
            print(f"FAIL: round {i}, size {size}: got {len(back)} bytes, data differs")
            return 1
    print(f"PASS: {ROUNDS} packets echoed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
