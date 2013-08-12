import usb.core
import usb.util
import sys
import array

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

# write the data
if sys.argv[1] == '1':
	inp = 1
elif sys.argv[1] == '2':
	inp = 2
elif sys.argv[1] == '3':
	inp = 3
elif sys.argv[1] == '4':
	inp = 4
else:
	inp = 0


#ep_out.write([chr(42), chr(inp), chr(0), chr(4)])
ep_out.write([chr(128), chr(2)])

try:
	ret = ep_in.read(64)
	if ret[0] == 14:
		print ('Ack received')
		txt = map(unichr, ret[2:2+ret[1]])
		print (''.join(txt))
	else:
		print ('No Ack received')
		print ret
except usb.core.USBError:
	print ("USB command did not return status")


