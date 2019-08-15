#!/usr/bin/python3
#-*- coding: utf-8 -*-


#   Run at desired frequencies
#   Program in Python, using unofficial port of Gordon's WiringPi library
#       see here : https://github.com/WiringPi/WiringPi-Python
#   I use WiringPi pin numbering scheme. 
#   Also here, we investigate the capability to reach some desired frequencies with Delays or Timings based algorithms






import numpy
import wiringpi
import time

pin = 7
val = 1
pause_s = 1 # Pause of 1s between tests
test_Time = 5.0 # test will lasts 5s
period_s = 0.0005 # Actually half of the true period
period_ns = 500*1000 # half of the true period
list_periods = [0.5, 0.25, 0.1, 0.05, 0.025, 0.01, 0.005, 0.0025, 0.001, 0.0005, 0.00025, 0.0001, 0.00005, 0.000025, 0.00001, 0.000005, 0.0000025, 0.000001, 0.0000005, 0.00000025]
start_Time_s = ref_Time_s = 0.0 # in second
ref_Time_ns = 0 # in nanosecond
count = 0 # count / 5 = Hz
res = numpy.zeros((20,2), dtype='f')


# Initialisation de wiringPi... 
wiringpi.wiringPiSetup()

# Activate the pin
wiringpi.pinMode(pin, 1)



print('==========____START____==========')


# Auto loop on diff period...
for i in range(20):
    period_s =list_periods[i]
    period_ns = list_periods[i]*1000000000


    print('Method 1 : Delays... use double in seconds')
    # ----------------------------------------
    # First method : Delays
    # ----------------------------------------
    count = 0
    start_Time_s = time.perf_counter()
    while time.perf_counter() < (start_Time_s+test_Time):
        val = 1 - val # algo balance
        wiringpi.digitalWrite(pin,val)
        count = count + 1
        time.sleep(period_s)
    print('Method 1 = {var1:.6f} Hz / {var2:.0f} Hz. ==> {var3:.6f} %'.format(var1= count/5/2, var2 = 1/2/period_s, var3 = count/5/2*2*period_s*100))
    res [i][0] = count/5/2
    # ----------------------------------------



    # Pause = 1s
    print('Pause of 1s')
    time.sleep(pause_s)


    print('Method 2 : Timings... use double in seconds')
    # ----------------------------------------:
    # Second method : Timings (s)
    # ----------------------------------------
    count = 0
    start_Time_s = ref_Time_s = time.perf_counter()
    while time.perf_counter() < (start_Time_s+test_Time):
        if  (ref_Time_s+period_s) < time.perf_counter():
            ref_Time_s = time.perf_counter()
            val = 1 - val # algo balance
            wiringpi.digitalWrite(pin,val)
            count = count + 1
    print('Method 2 = {var1:.6f} Hz / {var2:.0f} Hz. ==> {var3:.6f} %'.format(var1= count/5/2, var2 = 1/2/period_s, var3 = count/5/2*2*period_s*100))
    res [i][1] = count/5/2
    # ----------------------------------------


    # Pause = 1s
    print('Pause of 1s')
    time.sleep(pause_s)

# ----------------------------------------
# --------- PRINTING OF RESULTS ----------
# ----------------------------------------
print('\n\t------- Benchmark\'s results -------\n')
print('\t\t\t Method 1\t\t\t | Method 2\n\n')

for i in range(20):
    print('\t\t\t {var1:.3f} Hz\t\t\t | {var2:.3f} Hz\n\n'.format(var1 = res[i][0], var2 = res[i][1]))
# ----------------------------------------


print('==========____FINISHED____==========')


