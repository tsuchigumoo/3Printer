// Run at desired frequency
// Program in C, using only C and Linux' sysfs 


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>



void pinWrite(int value)
{
	char valStr[] = "01";
	

	FILE *fp;
	fp = fopen("/sys/class/gpio/gpio4/value", "w");

	puts("jhiibn");
	usleep(500);	

	if(fp>0)
		fwrite(valStr[value], 1, sizeof(valStr[value]), fp);
	fclose(fp);

	puts("autre ...");
}



int main()
{
	short val = 0;
	valStr
	
	puts("endless...");

	while(1){
		val = 1 - val; // algo balance
		sprintf(valStr, "%d", val);
		pinWrite(val);
	}
// 	---------------------------------------------------------------

	return(0);
}
