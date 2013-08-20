import usb.core
import usb.util
import sys
import array
import time

# find our device
dev = usb.core.find(idVendor=0x0483, idProduct=0xffff)

# was it found?
if dev is None:
    raise ValueError('Device not found')

# set the active configuration. With no arguments, the first
# configuration will be the active one
# dev.set_configuration()
      
try:
	cfg = dev.get_active_configuration()
except usb.core.USBError:
	dev.set_configuration()
	cfg = dev.get_active_configuration()
	

interface_number = cfg[(0,0)].bInterfaceNumber
intf = usb.util.find_descriptor(
    cfg, bInterfaceNumber = interface_number,
)

ep_out = usb.util.find_descriptor(
    intf,
    # match the first OUT endpoint
    custom_match = \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_OUT
)

assert ep_out is not None

ep_in = usb.util.find_descriptor(
    intf,
    # match the first OUT endpoint
    custom_match = \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN
)

assert ep_in is not None

dc = int(sys.argv[1])

dc_b1 = (int(dc) >> 24) & 0xFF
dc_b2 = (int(dc) >> 16) & 0xFF
dc_b3 = (int(dc) >> 8) & 0xFF
dc_b4 = int(dc) & 0xFF
	
ep_out.write([chr(0x85), chr(6), chr(dc_b1), chr(dc_b2), chr(dc_b3), chr(dc_b4)])
ret = ep_in.read(64)

