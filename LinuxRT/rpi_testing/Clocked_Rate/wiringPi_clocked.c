// Run at desired frequency
// Program in C, using WiringPi library


#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

#define POUT 7


int main ()
{
	if (wiringPiSetup () == -1)
		return 1 ;

	pinMode (POUT, OUTPUT) ;         // aka BCM_GPIO pin 17


	int val = 0;
	int period_us = 500; // in us, max is 32767
	int period_ns = 500*1000; // in ns, max is 32767
	int timeTest = 5; // last 5s
	struct timespec sysTime;
	unsigned long long refTime_s, refTime_ns; // max is ...
/*
	while(1){	
		val = 1 - val;
		digitalWrite(POUT, val);	
	}
*/
	


	puts("---> Method 1 : with delays (resolution in us)");

// Method 1 : DELAYs
//	---------------------------------------------------------------
//	---------- Others process step in during execution : ----------
//	--------------- During "usleep", the SCHEDULER pass work ------
//	--------------- on others process of the system !!! -----------
//	-------- Into the "do while" loop, instructions take time -----
//	---------------------------------------------------------------	
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		val = 1 - val; // algo balance
		digitalWrite(POUT, val);
		delayMicroseconds(period_us);
	}
//	---------------------------------------------------------------


// Pause = 5s
puts("pause of 5s...");
usleep(5*1000000);


	puts("---> Method 2 : with timing (resolution=us)");
	
// Method 2 : TIMING
//	---------------------------------------------------------------
//	---------- No others process step in during execution ---------
//	-------- Into the "if" condition, instructions take time ------
//	---------------------------------------------------------------
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		
		if((refTime_ns + period_us*1000) < 1000000000*sysTime.tv_sec + sysTime.tv_nsec) 
		{
			refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
			val = 1 - val; // algo balance
			digitalWrite(POUT, val);
		}
	}
// 	---------------------------------------------------------------


// Pause = 5s
puts("pause of 5s...");
usleep(5*1000000);


	puts("---> Method 2 bis : with timing (resolution=ns)");
	
// Method 2 bis : TIMING
//	---------------------------------------------------------------
//	---------- No others process step in during execution ---------
//	-------- Into the "if" condition, instructions take time ------
//	---------------------------------------------------------------
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		
		if((refTime_ns + period_ns) < 1000000000*sysTime.tv_sec + sysTime.tv_nsec) 
		{
			refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
			val = 1 - val; // algo balance
			digitalWrite(POUT, val);
		}
	}
// 	---------------------------------------------------------------


	return(0);

}
