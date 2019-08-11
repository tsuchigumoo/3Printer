// Run at desired frequency
// Program in C, using only C and Linux' sysfs 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define PIN 24 /// unused here
#define POUT 4


static int GPIOExport(int pin) // Activate 'pin'
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written; // unsigned integer with a size of the maximum permitted by the OS
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY); // adresse qui pointe sur les "File Descriptor" (the "file descriptor" point to the opened "file description" just created whom point to the "file". Thus, it provide others I/O functions a ways to refer to that file.
	
	if(-1 == fd){// if there is a problem at the opening...
			fprintf(stderr, "Failed to open export for writing!\n");
			return(-1);

	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin); // write the number of variable "pin" in a buffer (specified size 3) || well, quite similar to a cast
	write(fd, buffer, bytes_written); // then write the string in the file
	close(fd);
	return(0);
}


static int GPIOUnexport(int pin) // Deactivate 'pin'

{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY); 
	
	if(-1 == fd){
			fprintf(stderr, "Failed to open unexport for writing!\n");
			return(-1);

	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}


static int GPIODirection(int pin, int dir) // Set direction 'dir' to 'pin'
{
	static const char s_directions_str[] = "in\0out";

#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if(-1 == fd){
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if(-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)){
		fprintf(stderr, "Failed to set direction!\n");
	}

	close(fd);
	return(0);
}


static int GPIORead(int pin) // Write state HIGH or LOW to 'pin'
{
#define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if(-1 == fd){
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}
	
	if(-1 == read(fd, value_str, 3)){
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);
	return(atoi(value_str));
}


static int GPIOWrite(int pin, int value) // Write state HIGH or LOW to 'pin'
{
	static const char s_values_str[] = "01";
	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if(-1 == fd){
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
	
	if(1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)){
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}


static int pinWrite(int numPin, int value)
{
	static const char s_values_str[] = "01";
	char path[VALUE_MAX];
	sprintf(path, "/sys/class/gpio/gpio%d/value", numPin);

	FILE *fp;
	fp = fopen(path, "w");
	if(fp>0)
		fwrite(&s_values_str[value], 1, sizeof(s_values_str[value]), fp);
	fclose(fp);

	return(0);
}



int main()
{
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
		double res[3][20]; // array of results depending the method


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
				// Enablde GPIOs pins
				if(-1 == GPIOExport(POUT))
					return(1);
				
				// Set GPIOs directions
				if(-1 == GPIODirection(POUT, OUT))
					return(2);
				
				clock_gettime(CLOCK_REALTIME, &sysTime);
				startTime_s = sysTime.tv_sec;
				while(sysTime.tv_sec < (startTime_s+timeTest)){
					clock_gettime(CLOCK_REALTIME, &sysTime);
					
					val = 1 - val; // algo balance
					pinWrite(POUT, val);
					//usleep(period_us);
					nanosleep(&sleepTime, &remTime);
					count++;
				}

				//Disable GPIO pins
				if(-1 == GPIOUnexport(POUT))
					return(4);
				
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
				// Enablde GPIOs pins
				if(-1 == GPIOExport(POUT))
					return(1);
				
				// Set GPIOs directions
				if(-1 == GPIODirection(POUT, OUT))
					return(2);
				
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
						pinWrite(POUT, val);
						count++;
					}
				}
				
				//Disable GPIO pins
				if(-1 == GPIOUnexport(POUT))
					return(4);
				
				//Save result
				res[1][i] = (double)count/5/2;
// 	---------------------------------------------------------------


				// Pause
				printf("pause of %d second...\n", pause/1000000);
				usleep(pause);


				printf("---> Method 3 bis : with timing & system call 'open()' (resolution=ns)\n");

// Method 3 : TIMING & system call 'open()'
//	---------------------------------------------------------------
//	---------- No others process step in during execution ---------
//	-------- Into the "if" condition, instructions take time ------
//	- Here we use open() which is a system call secific to the OS -
//	---------------------------------------------------------------
				count=0;
				// Enablde GPIOs pins
				if(-1 == GPIOExport(POUT))
					return(1);
				
				// Set GPIOs directions
				if(-1 == GPIODirection(POUT, OUT))
					return(2);
				
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
						GPIOWrite(POUT, val);
						count++;
					}
				}
				
				//Disable GPIO pins
				if(-1 == GPIOUnexport(POUT))
					return(4);
				
				//Save result
				res[2][i] = (double)count/5/2;
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
		printf(" \t\t\t Method 1 \t\t\t Method 2 \t\t\t Method 3\n");

		for(int i=0; i<20; i++)
		{
			for(int j=0; j<3; j++)
			{
				printf(" \t\t\t %.3f", res[j][i]);
			}
			printf("\n\n");
		}
// 	---------------------------------------------------------------


		printf("finished !\n");

		return(0);

}
