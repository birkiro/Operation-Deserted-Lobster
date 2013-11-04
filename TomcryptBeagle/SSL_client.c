/* 
 *	SOLAR-CONTROLLER
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include <complex.h>
#include <stdbool.h>
#include <sys/wait.h>

#define 	SWEEP_DELAY_MS		5

#define 	MAX_GATE_SUPPLY		2.90	//on the DAC output!!!
#define 	MIN_GATE_SUPPLY		2.00	//on the DAC output!!!
#define 	ARRAY_SIZE  (int)((MAX_GATE_SUPPLY - MIN_GATE_SUPPLY) / 0.001)

FILE* file;
int adc0_value, adc1_value, adc2_value, adc4_value, adc6_value;

char logfile[50];
char MPPfile[50];


volatile static int I_V_array[2][ARRAY_SIZE];
int array_index = 0;

float DAC_value = 0.000;

void sleep_ms();

int main(int argc, char ** argv) {
	//init logfile
	fprintf(stdout, "file init\n");
//	system("echo 0 > logfile");

//init DAC
//install DAC module
	system("insmod i2cdev.ko");

	//init ADC
	//install ADC module
	system(
			"insmod /lib/modules/3.2.33-psp26.1/kernel/drivers/input/touchscreen/ti_tscadc.ko");
	//Usage: cat /sys/devices/platform/omap/tsc/ain1

	//init GPIO
	//
	//export pin to userspace gpio1_28 = 32+28=60
	system("echo 60 > /sys/class/gpio/export");
	//Now, there is a /sys/class/gpio/gpio60 directory:
	//set direction of pin
	system("echo out > /sys/class/gpio/gpio60/direction");
	//To set the value of the GPIO output:
	system("echo 0 > /sys/class/gpio/gpio60/value");

	printf("init done. \n");

	// MAIN LOOP
	if (1) {
		/*
		 * Start
		 * Generate webpage and wait for user input
		 * If(test started from web interface)
		 * -Turn on halogen spots
		 * -wait x sec. for lights to stabilize
		 * -read initials
		 * 
		 * -while(test not over)
		 * ---Read ADC (volt & current on panels)
		 * ---Increase DAC
		 *
		 * -Turn lights off
		 * -Calculate MPP
		 * -Generate Test report/graphs
		 * -update webpage
		 * -wait for next test to start
		 */

		printf("Start. \n");
		printf("Generate webpage and wait for user input. \n");
		printf("Turn on halogen spots. \n");
		system("echo 1 > /sys/class/gpio/gpio60/value");

		printf("wait x sec. for lights to stabilize. \n");
		sleep(5);

		printf("Read initial values. \n");
		file = fopen("/sys/devices/platform/omap/tsc/ain1", "r");
		fscanf(file, "%d", &adc0_value);
		fclose(file);
		fprintf(stdout, "Iref adc reading: %d\n", adc0_value);

		file = fopen("/sys/devices/platform/omap/tsc/ain2", "r");
		fscanf(file, "%d", &adc1_value);
		fclose(file);
		fprintf(stdout, "Vref adc reading: %d\n", adc1_value);

		file = fopen("/sys/devices/platform/omap/tsc/ain3", "r");
		fscanf(file, "%d", &adc2_value);
		fclose(file);
		fprintf(stdout, "IOUT offset adc reading: %d\n", adc2_value);

		file = fopen("/sys/devices/platform/omap/tsc/ain5", "r");
		fscanf(file, "%d", &adc4_value);
		fclose(file);
		fprintf(stdout, "VOUT adc reading: %d\n", adc4_value);

		file = fopen("/sys/devices/platform/omap/tsc/ain7", "r");
		fscanf(file, "%d", &adc6_value);
		fclose(file);
		fprintf(stdout, "IOUT adc reading: %d\n", adc6_value);

		printf("Test loop... \n");
		array_index = 0; //reset index
		for (DAC_value = MIN_GATE_SUPPLY; DAC_value < MAX_GATE_SUPPLY;
				DAC_value += 0.001) {
			//write to DAC

			file = fopen("/dev/dac", "w");
			fprintf(file, "%5.3fv", DAC_value);
			fclose(file);
//			printf("DAC value:%5.3fv\n", DAC_value);
			//wait xx ms
			sleep_ms(SWEEP_DELAY_MS);
			//read ADC's
			file = fopen("/sys/devices/platform/omap/tsc/ain5", "r");
			fscanf(file, "%d", &adc4_value);
			fclose(file);
//			fprintf(stdout, "VOUT adc reading: %d\n", adc4_value);
			file = fopen("/sys/devices/platform/omap/tsc/ain7", "r");
			fscanf(file, "%d", &adc6_value);
			fclose(file);
//			fprintf(stdout, "IOUT adc reading: %d\n", adc6_value);
			I_V_array[0][array_index] = adc4_value;
			I_V_array[1][array_index] = adc6_value;
			array_index++;
		}

		//turn off DAC
		DAC_value = 0;
		file = fopen("/dev/dac", "w");
		fprintf(file, "%5.3fv", DAC_value);
		fclose(file);
		printf("DAC value:%5.3fv\n", DAC_value);

		printf("Turn lights off. \n");
		system("echo 0 > /sys/class/gpio/gpio60/value");
		printf("Calculate MPP. \n");
		printf("Generate Test report/graphs. \n");
		printf("update webpage. \n");
		printf("wait for next test to start. \n");
//		sleep(4);

		int MPP_value = 0;
		int MPP_index = 0;
		for (array_index = 0; array_index < ARRAY_SIZE; array_index++) {
			int temp = I_V_array[0][array_index] * I_V_array[1][array_index];
			if (MPP_value < temp) {
				MPP_index = array_index;
				MPP_value = temp;
			}
		}

		system("rm /var/www/MPPfile");
		sprintf(MPPfile, "/var/www/MPPfile", 1);
		file = fopen(MPPfile, "a");
		if (file != NULL) {
			fprintf(file, "V:%d - I:%d\n", I_V_array[0][MPP_index],
					I_V_array[1][MPP_index]);
			fclose(file);
		}

		system("rm /var/www/logfile");
		sprintf(logfile, "/var/www/logfile", 1);
		for (array_index = 0; array_index < ARRAY_SIZE; array_index++) {
//			printf("No:%d - V:%d - I%d \n",array_index, I_V_array[0][array_index],I_V_array[1][array_index]);
			// write to file
			// calculate filename

			file = fopen(logfile, "a");
			if (file != NULL) {
				fprintf(file, "No:%d - V:%d - I:%d\n", array_index,
						I_V_array[0][array_index], I_V_array[1][array_index]);
				fclose(file);
			}
		}
	}
	return 0;
}

void sleep_ms(int ms) {
	usleep(ms * 1000); //convert to microseconds
	return;
}
