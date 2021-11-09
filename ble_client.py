from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import struct
import time
import threading

MSG_LOCK = 0x10

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print ("Discovered device", dev.addr)
        elif isNewData:
            print ("Received new data from", dev.addr)
scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n=0
recommended_n = -1
for dev in devices:
    print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
    for (adtype, desc, value) in dev.getScanData():
        print ("  %s = %s" % (desc, value))
        if "ButtonBeauty" in value:
            recommended_n = n
    n += 1
if recommended_n != -1:
    print ("Recommend number {}".format(recommended_n))
number = input('Enter your device number: ')
print('Device', number)
print(devices[number].addr)
print("Connecting...")
dev = Peripheral(devices[number].addr, 'random')
print("Services...")
for svc in dev.services:
    print(str(svc))
    for ch in svc.getCharacteristics():
        print(str(ch))
try:
    buttonService= dev.getServiceByUUID(UUID(0xa000))
    ch_button= dev.getCharacteristics(uuid=UUID(0xa001))[0]
    
    ledService= dev.getServiceByUUID(UUID(0xa002))
    ch_led1= dev.getCharacteristics(uuid=UUID(0xa003))[0]
    ch_led2= dev.getCharacteristics(uuid=UUID(0xa006))[0]
    
    broadService= dev.getServiceByUUID(UUID(0xa004))
    ch_broad= dev.getCharacteristics(uuid=UUID(0xa005))[0]
    
    def send_message(msg):
        ch_broad.write(msg)
        #ch.write(msg1.encode('ascii'))
        #ch.write(msg2.encode('ascii'))
        print("Sent: \"{}\"".format(msg))
        
        while (ch_broad.supportsRead()):
            msgin = ch_broad.read()
            if (msgin != msg):
                #print(type(msgin))
                print("Received: \"{}\"".format(msgin))
                #print([elem.encode("hex") for elem in msgin])
                #print("msgin get")
                break
    
    #msg0 = '1'
    msg1 = "Hello! Who are you?"
    send_message(msg1)
    msg2 = "How to control LEDs?"
    send_message(msg2)
    msg3 = "What else to know?"
    send_message(msg3)
    
    def control_led():
        while 1:
            inp = raw_input()
            if (inp == "LED1 on"):
                ch_led1.write('\x01')
                print("turning on LED1...")
            elif (inp == "LED1 off"):
                ch_led1.write('\x00')
                print("turning off LED1...")
            elif (inp == "LED2 on"):
                ch_led2.write('\x01')
                print("turning on LED2")
            elif (inp == "LED2 off"):
                ch_led2.write('\x00')
                print("turning off LED2")
            elif (inp == "exit"):
                print("Bye~!")
                break
            else:
                print("Wrong Input!")
            
    def monitor_button():
        t = threading.currentThread()
        while (ch_button.supportsRead()):
            if getattr(t, "running", True) == 0:
                break
            msgin = ch_button.read()
            if (msgin != '\0'):
                #print(ord(msgin))
                print("Button pressed.")
            time.sleep(0.5)
                
    
    thread = threading.Thread(target=monitor_button)
    thread.start()
    
    control_led()
    thread.running = False
        
        
finally:
    dev.disconnect()
