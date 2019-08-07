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
	short val = 0;
	// Note : these period are half of the true period !!!
	const int period_us = 500; // in us, max is 32767
	const int period_ns = 500*1000; // in ns, max is 32767
	int timeTest = 5; // last 5s
	struct timespec sysTime;
	unsigned long long refTime_s, refTime_ns; // max is ...



	puts("---> Method 1 : with delays (resolution in us)");

// Method 1 : DELAYs
//	---------------------------------------------------------------
//	---------- Others process step in during execution : ----------
//	--------------- During "usleep", the SCHEDULER pass work ------
//	--------------- on others process of the system !!! -----------
//	-------- Into the "do while" loop, instructions take time -----
//	---------------------------------------------------------------	
	// Enablde GPIOs pins
	if(-1 == GPIOExport(POUT))
		return(1);
	
	// Set GPIOs directions
	if(-1 == GPIODirection(POUT, OUT))
		return(2);
	
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		
		val = 1 - val; // algo balance
		pinWrite(POUT, val);
		usleep(period_us);	
	}

	//Disable GPIO pins
	if(-1 == GPIOUnexport(POUT))
		return(4);
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
	// Enablde GPIOs pins
	if(-1 == GPIOExport(POUT))
		return(1);
	
	// Set GPIOs directions
	if(-1 == GPIODirection(POUT, OUT))
		return(2);
	
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);

		
		if((refTime_ns + period_us*1000) < 1000000000*sysTime.tv_sec + sysTime.tv_nsec) 
		{
			refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
			val = 1 - val; // algo balance
			pinWrite(POUT, val);
		}
	}
	
	//Disable GPIO pins
	if(-1 == GPIOUnexport(POUT))
		return(4);
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
	// Enablde GPIOs pins
	if(-1 == GPIOExport(POUT))
		return(1);
	
	// Set GPIOs directions
	if(-1 == GPIODirection(POUT, OUT))
		return(2);
	
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		
		if((refTime_ns + period_ns) < 1000000000*sysTime.tv_sec + sysTime.tv_nsec) 
		{
			refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
			val = 1 - val; // algo balance
			pinWrite(POUT, val);
		}
	}

	//Disable GPIO pins
	if(-1 == GPIOUnexport(POUT))
		return(4);
// 	---------------------------------------------------------------


// Pause = 5s
puts("pause of 5s...");
usleep(5*1000000);


	puts("---> Method 3 bis : with timing & system call 'open()' (resolution=ns)");

// Method 3 : TIMING & system call 'open()'
//	---------------------------------------------------------------
//	---------- No others process step in during execution ---------
//	-------- Into the "if" condition, instructions take time ------
//	---------------------------------------------------------------
	// Enablde GPIOs pins
	if(-1 == GPIOExport(POUT))
		return(1);
	
	// Set GPIOs directions
	if(-1 == GPIODirection(POUT, OUT))
		return(2);
	
	clock_gettime(CLOCK_REALTIME, &sysTime);
	refTime_s = sysTime.tv_sec;
	refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
	while(sysTime.tv_sec<(timeTest+refTime_s)){
		clock_gettime(CLOCK_REALTIME, &sysTime);
		
		if((refTime_ns + period_ns) < 1000000000*sysTime.tv_sec + sysTime.tv_nsec) 
		{
			refTime_ns = 1000000000*sysTime.tv_sec + sysTime.tv_nsec;
			val = 1 - val; // algo balance
			GPIOWrite(POUT, val);	
		}
	}

	//Disable GPIO pins
	if(-1 == GPIOUnexport(POUT))
		return(4);
// 	---------------------------------------------------------------



	puts("finished !");

	return(0);
}
