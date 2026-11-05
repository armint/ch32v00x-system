#!/usr/bin/env python3
"""Host side of the CH32V307 USBHS loopback test (needs pyusb).

Finds the device 1209:0001 and prints the speed. Checks that EP0 accepts
a SETUP packet after a STALL. Sends packets of several sizes to the bulk
OUT endpoint and checks that the bulk IN endpoint sends the same bytes
back, before and after CLEAR_FEATURE(ENDPOINT_HALT) on both endpoints.
Exit code 0: all checks pass.
"""

import errno
import os
import sys

import usb.core
import usb.util

VID, PID = 0x1209, 0x0001
EP_OUT, EP_IN = 0x01, 0x81
ROUNDS = 200
ROUNDS_AFTER_CLEAR = 20


def loopback(dev, mps, rounds, label):
    sizes = [1, 2, mps // 2, mps - 1, mps]
    for i in range(rounds):
        size = sizes[i % len(sizes)]
        data = os.urandom(size)
        dev.write(EP_OUT, data, timeout=1000)
        back = bytes(dev.read(EP_IN, mps, timeout=1000))
        if back != data:
            print(f"FAIL: {label}, round {i}, size {size}: got {len(back)} bytes, data differs")
            return False
    return True


def stall_then_setup(dev):
    # The test program stalls every vendor request. The next SETUP must
    # still get an answer: EP0 must accept a SETUP while it answers STALL.
    try:
        dev.ctrl_transfer(0xC0, 0x01, 0, 0, 1, timeout=1000)
        print("FAIL: vendor request not stalled")
        return False
    except usb.core.USBError as e:
        if e.errno != errno.EPIPE:
            print(f"FAIL: vendor request: expected STALL (EPIPE), got {e}")
            return False
    desc = dev.ctrl_transfer(0x80, 6, 0x0100, 0, 18, timeout=1000)
    if len(desc) != 18 or desc[1] != 1:
        print(f"FAIL: device descriptor after STALL: got {bytes(desc).hex()}")
        return False
    return True


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

    if not stall_then_setup(dev):
        return 1
    if not loopback(dev, mps, ROUNDS, "loopback"):
        return 1
    # CLEAR_FEATURE(ENDPOINT_HALT) resets the data toggle to DATA0 on both
    # sides; a wrong toggle reset makes the next packets disappear.
    dev.clear_halt(EP_OUT)
    dev.clear_halt(EP_IN)
    if not loopback(dev, mps, ROUNDS_AFTER_CLEAR, "after clear_halt"):
        return 1
    print(f"PASS: STALL then SETUP, {ROUNDS} + {ROUNDS_AFTER_CLEAR} packets echoed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
