/*
   pulse.c

   gcc -o pulse pulse.c -lpigpio -lrt -lpthread

   sudo ./pulse
*/

#include <stdio.h>
#include <pigpio.h>

#define POUT 4



int main(int argc, char *argv[])
{
   double start;
   puts("init...");

   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }

	puts("seting up...");
   /* Set GPIO modes */
   gpioSetMode(POUT, PI_OUTPUT);
   
puts("writing...");
      gpioWrite(POUT, 1);

      time_sleep(10.5);



   start = time_time();
/*
   while ((time_time() - start) < 5.0)
   {
      gpioWrite(POUT, 1);

      time_sleep(0.5);

      gpioWrite(POUT, 0); 

      time_sleep(0.5);

   }
*/
   puts("terminating...");
   /* Stop DMA, release resources */
   gpioTerminate();

   return 0;
}
