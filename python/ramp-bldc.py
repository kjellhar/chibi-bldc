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


ep_out.write([chr(0x80), chr(2)])

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

ep_out.write([chr(0x82), chr(2)])

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

currentDc = 1500



finalRpm = int(sys.argv[1])
currentRpm = int(1000)

while (currentRpm < finalRpm):
	if (currentRpm > finalRpm):
		currentRpm = finalRpm
	
	currentRpm += 10
	currentDc += 1.5
	
	if (currentDc > 5000):
		currentDc = 5000
		
	dc_b1 = (int(currentDc) >> 24) & 0xFF
	dc_b2 = (int(currentDc) >> 16) & 0xFF
	dc_b3 = (int(currentDc) >> 8) & 0xFF
	dc_b4 = int(currentDc) & 0xFF
	
	ep_out.write([chr(0x85), chr(6), chr(dc_b1), chr(dc_b2), chr(dc_b3), chr(dc_b4)])
	ret = ep_in.read(64)
	
	crpm_hb = (int(currentRpm) >> 8) & 0xFF
	crpm_lb = int(currentRpm) & 0xFF
	
	ep_out.write([chr(0x86), chr(4), chr(crpm_hb), chr(crpm_lb)])
	ret = ep_in.read(64)
	time.sleep (0.003)
