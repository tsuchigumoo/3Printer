#!/usr/bin/python3
#-*- coding: utf-8 -*-

import wiringpi
import time


pin = 7
val = 1
test_Time = 5.0 # test will lasts 5s
period_s = 0.0005 # Actually half of the true period
period_ns = 500*1000 # half of the true period
list_periods = [0.5, 0.25, 0.1, 0.05, 0.025, 0.01, 0.005, 0.0025, 0.001, 0.0005, 0.00025, 0.0001, 0.00005, 0.000025, 0.00001, 0.000005, 0.0000025, 0.000001, 0.0000005, 0.00000025]
start_Time_s = ref_Time_s = 0.0 # in second
ref_Time_ns = 0 # in nanosecond
count = 0 # count / 5 = Hz

# Initialisation de wiringPi... 
wiringpi.wiringPiSetup()

# Activate the pin
wiringpi.pinMode(pin, 1)

print('==========____START____==========')

# Auto loop on diff period...
for i in list_periods:
    period_s = i
    period_ns = i*1000000000


#    print('Method 1 : Delays...')
    # ----------------------------------------
    # First method : Delays
    # ----------------------------------------
    count = 0
    start_Time_s = time.perf_counter()
#    print(start_Time_s, ' - ', time.perf_counter()) # much more slower than seconds... not true time. Try => time.perfcounter ???
    while time.perf_counter() < (start_Time_s+test_Time):
        val = 1 - val # algo balance
        wiringpi.digitalWrite(pin,val)
        count = count + 1
        time.sleep(period_s)
    print('Method 1 = {var1:.6f} Hz / {var2:.0f} Hz. ==> {var3:.6f} %'.format(var1= count/5/2, var2 = 1/2/period_s, var3 = count/5/2*2*period_s*100))
    # ----------------------------------------



    # Pause = 1s
#    print('Pause of 1s')
    time.sleep(1);


#    print('Method 2 : Timings... use double var for seconds')
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
    # ----------------------------------------


    # Pause = 1s
#    print('Pause of 1s')
    time.sleep(1);


#    print('Method 2 bis : Timings... use double var for nanoseconds')
    # ----------------------------------------:
    # Second method : Timings (ns)
    # ----------------------------------------
    count = 0
    start_Time_s = time.perf_counter()
    ref_Time_ns = time.perf_counter_ns()
    while time.perf_counter() < (start_Time_s+test_Time):
        if  (ref_Time_ns+period_ns) < time.perf_counter_ns():
            ref_Time_ns = time.perf_counter_ns()
            val = 1 - val # algo balance
            wiringpi.digitalWrite(pin,val)
            count = count + 1
    print('Method 2 bis = {var1:.6f} Hz / {var2:.0f} Hz. ==> {var3:.6f} %'.format(var1= count/5/2, var2 = 1/2/period_ns*1000000000, var3 = count/5/2*2*period_ns/1000000000*100))
    # ----------------------------------------


    # Pause = 1s
#    print('Pause of 1s')
    time.sleep(1);



print('==========____FINISHED____==========')



