import time
from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD
from datetime import datetime
from influxdb import InfluxDBClient
import struct #delete if not using Struct


# Configure InfluxDB connection variables
host = 'localhost' # My Ubuntu NUC
port = 8086 # default port
user = 'admin' # the user/password created for the pi, with write access
password = '<password>' #your password - DON'T FORGET TO UPDATE!
dbname = 'overwatch' # the database we created earlier
interval = 60 # Sample period in seconds

dataPayload = 0
payload = 0

unitTrigger = 1 #this is the trigger that looks for various unit numbers.  Extend off this for additional thermometers.  Also see line 85.


# Create the InfluxDB client object
client = InfluxDBClient(host, port, user, password, dbname)

BOARD.setup()

class LoRaRcvCont(LoRa):
    def __init__(self, verbose=False):
        super(LoRaRcvCont, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)

    def start(self):
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        while True:
            sleep(.5)
            rssi_value = self.get_rssi_value()
            status = self.get_modem_status()
            sys.stdout.flush()
            

    def on_rx_done(self):
        # current date and time
        now = datetime.now()
        format = "%a %b %d %H:%M:%S %Y"    

        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)
        dateTimeObj = datetime.now()
        shortTime = now.strftime(format)

        dataPayload = bytes(payload).decode("utf-8", errors='ignore').strip().strip('\x00')

        data = dataPayload.split("|") #splitting the payload into a list of strings

        unitID = data[0]
        celcius = int(data[1])
        fahrenheit = 9.0/5.0 * celcius + 32 

        print("Unit ID: ",unitID)  
        print("Body Temperature: ",celcius,"C")  
        print("Body Temperature: ",fahrenheit,"F")
        print('\n')

        if header == unitTrigger:
                print ("Unit Number:" unitID "detected")
                overwatch = [
                {
                  "measurement": "sensor_data",
                      "tags": {
                          "Unit ID": unitID,
                      },
                      "time": time.ctime(),
                      "fields": {
                          "temperature_c": celcius,
                          "temperature_f": fahrenheit,
                      }
                  }
                ]
        else :
                print ("Not Triggered") # extend off this for other thermometer units with other IDs (set on the Thermometer side code)

        print('\n')  

        self.set_mode(MODE.SLEEP)
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT) 


        if client.write_points(influx_data) == True:
            print("Data written to DB successfully")
            print('\n') 
        else:  # write failed.
            print("DB write failed.")
            print('\n') 


lora = LoRaRcvCont(verbose=False)
lora.set_mode(MODE.STDBY)


#  BASIC LoRA SETTINGS. 

# Defaults:
# f = 434.0MHz
# Bw = 125 kHz
# Cr = 4/5
# Sf = 128chips/symbol
# CRC on 13 dBm
# lora.set_pa_config(pa_select=1)

lora.set_pa_config(pa_select=1)



# ADVANCED LoRA SETTINGS.  Can use to change frequency, output power, etc.

# # f = 915 MHz
# # Bw = 125 kHz
# # Cr = 4/8
# # Sf = 4096chips/symbol
# # CRC on. 13 dBm
# lora.set_freq(915.0)  
# lora.set_pa_config(pa_select=1, max_power=21, output_power=15)
# lora.set_bw(BW.BW125)
# lora.set_coding_rate(CODING_RATE.CR4_8)
# lora.set_spreading_factor(12)
# lora.set_rx_crc(True)
# lora.set_low_data_rate_optim(True)




try:
    lora.start()
except KeyboardInterrupt:
    sys.stdout.flush()
    print("")
    sys.stderr.write("KeyboardInterrupt\n")
finally:
    sys.stdout.flush()
    print("")
    lora.set_mode(MODE.SLEEP)
    BOARD.teardown()
