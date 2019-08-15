//  Run at desired frequencies
//  Program in C, using WiringPi library from Gordon Henderson (now deprecated)

//  Based on blink.c from Gordon Henderson
//      see here : http://wiringpi.com/examples/blink/
//  I use WiringPi pin numbering scheme. 
//  Also here, we investigate the capability to reach some desired frequencies with Delays or Timings based algorithms


#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

#define PIN 24 /// unused here
#define POUT 7


int main ()
{
		if (wiringPiSetup () == -1) // init WP
			return 1 ;

		pinMode(POUT, OUTPUT); // aka BCM_GPIO pin 17	


		// INITIALISATIONS
		short val = 0; // value of pin to set 
		// Note : these periods are half of the true period !!!
		long long period_us = 500000; // in us, period to set
		long long period_ns = 500000*1000; // in ns, period to set
		long long startTime_s; // Used to compare timings for the duration of the test
		int timeTest = 5; // last 5s
		unsigned int pause = 1*1000000; // last 1s 
		struct timespec sysTime; // used to get TimeOfDay exact at the nanosecond for the duration of periods
		struct timespec sleepTime, remTime; // used to set nanosleep() function ===> http://man7.org/linux/man-pages/man2/nanosleep.2.html , Here we can get the "error" through remTime ?
			sleepTime.tv_sec = 0; // WARNING : here we dont plan to used a period over the second, so only care about 'tv_nsec'.    If you want a period above second, use 'usleep(period_us)'.
			sleepTime.tv_nsec = (long)period_ns; 
		long long refTime_s, refTime_ns; // Used to compare timings for the duration of periods
		long long list_periods_ns[] = {500000000, 250000000, 100000000, 50000000, 25000000, 10000000, 5000000, 2500000, 1000000, 500000, 250000, 100000, 50000, 25000, 10000, 5000, 2500, 1000, 500, 250}; // all these periods to bench
		long long count; // used to count the real number of periods achieved during the test
		double res[2][20]; // array of results depending the method


		printf("===> Starting...\n\n"); 

		for(int i = 0; i<20; i++)
		{
				period_ns = list_periods_ns[i]; // every loop a new period is set for benchmarking the method
				sleepTime.tv_nsec = (long)period_ns; 
				if(i<18)
					period_us = list_periods_ns[i]/1000; // 500ns and 250ns can't be achieved with microsecond resolution (only Method 1 can (if you want) use microsecond)


				printf("---> Method 1 : with delays (resolution in ns)\n");

// Method 1 : DELAYs
//	---------------------------------------------------------------
//	---------- Others process step in during execution : ----------
//	--------------- During "usleep", the SCHEDULER pass work ------
//	--------------- on others process of the system !!! -----------
//	-------- Into the "do while" loop, instructions take time -----
//	--- Here we use fwrite, a buffered function of stdio (in C) ---
//	---------------------------------------------------------------	
				count=0;
				clock_gettime(CLOCK_REALTIME, &sysTime);
				startTime_s = sysTime.tv_sec;
				while(sysTime.tv_sec < (startTime_s+timeTest)){
					clock_gettime(CLOCK_REALTIME, &sysTime);
					
					val = 1 - val; // algo balance
					digitalWrite(POUT, val);
					//usleep(period_us);
					nanosleep(&sleepTime, &remTime);
					count++;
				}

				//Save result
				res[0][i] = (double)(count)/5/2;
//	---------------------------------------------------------------


				// Pause
				printf("pause of %d second...\n", pause/1000000);
				usleep(pause);


				printf("---> Method 2 : with timing (resolution=ns)\n");
					
// Method 2 : TIMING
//	---------------------------------------------------------------
//	---------- No others process step in during execution ---------
//	-------- Into the "if" condition, instructions take time ------
//	--- Here we use fwrite, a buffered function of stdio (in C) ---
//	---------------------------------------------------------------
				count=0;
				clock_gettime(CLOCK_REALTIME, &sysTime);
				refTime_ns = sysTime.tv_nsec;
				startTime_s = sysTime.tv_sec;
				refTime_s = startTime_s;
				while(sysTime.tv_sec<(startTime_s+timeTest)){
					clock_gettime(CLOCK_REALTIME, &sysTime);
					
					if(((sysTime.tv_sec == refTime_s) && (sysTime.tv_nsec >= (refTime_ns + period_ns))) || (sysTime.tv_sec > refTime_s)) 
					{
						refTime_ns = sysTime.tv_nsec;
						refTime_s = sysTime.tv_sec;
						val = 1 - val; // algo balance
						digitalWrite(POUT, val);
						count++;
					}
				}
				
				//Save result
				res[1][i] = (double)count/5/2;
// 	---------------------------------------------------------------


				// Pause
				printf("pause of %d second...\n", pause/1000000);
				usleep(pause);

		}


// 	---------------------------------------------------------------
//	--------- This part display the results : ---------------------
//	---------     -> row = frequency adjusted ---------------------
//	---------     -> column = Method used -------------------------
// 	---------------------------------------------------------------
		printf("------- BENCHMARK's RESULTS -------\n");
		printf(" \t\t\t Method 1 \t\t\t Method 2\n");

		for(int i=0; i<20; i++)
		{
			for(int j=0; j<2; j++)
			{
				printf(" \t\t\t %.3f", res[j][i]);
			}
			printf("\n\n");
		}
// 	---------------------------------------------------------------


		printf("finished !\n");

		return(0);

}
