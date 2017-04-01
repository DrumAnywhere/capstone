/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/

/*************************************************************************

Drum Anywhere - G4
Created By - Shivansh Singla and Jake Davidson
Date Created - 20th Feb, 2016
Description - This program uses the MPU9250 library along with the I2C
communication library to establish a connection to the MPUs and start 
reading raw data from the registers anfter proper self tests, initialization 
and calibration. Once the raw data is read, it is converted to meaningful 
acceleration and gyration values using appropriate resolution functions.


**************************************************************************/

#include <stdio.h>
#include "includes.h"
#include "i2c/I2C.h"
#include "MPU9250/mpu9250.h"
#include "altera_avalon_pio_regs.h"
#include <time.h>
#include "altera_up_avalon_character_lcd.h"

void init_pedal_pio();
void interrupt_isr_pedalPress(void *context, alt_u32 id);

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1

// globals
alt_up_character_lcd_dev* myLCD;
int kick_drum;

/* This task initializes the IMUs using I2C communication and reads data from the registers */
void task1(void* pdata){

	// init lcd
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Self Testing");

	//Temporary buffer to read into
	alt_u8 ReadBuf[6];
	//Device address
	alt_u8 deviceAddress = 0xD0;
	//WHO AM I Register
	alt_u8 whoamireg = 0x75;
	alt_u16 buflen = 1;
	// Self tests, biases and resolution for DPS conversions
	float selfTest[6];
	float aRes, gRes;

    // WHO AM I testing
  	I2C_Start(I2C_SCL_BASE,I2C_SDA_BASE);
    if(!I2C_ReadFromDeviceRegister(I2C_SCL_BASE,I2C_SDA_BASE, deviceAddress, whoamireg, (alt_u8*)&ReadBuf, buflen, true)){

	  }
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
    printf("DrumStick #1 - WHO AM I: %0x\n", ReadBuf[0]);

    printf("Starting Self Tests - DrumStick #1\n");

    //Self Testing
    MPU9250SelfTest(selfTest, I2C_SCL_BASE,I2C_SDA_BASE);
    printf("x-axis self test: acceleration trim within : ");
   	printf("%.1f", selfTest[0]); printf("%% of factory value\n");
  	printf("y-axis self test: acceleration trim within : ");
  	printf("%.1f", selfTest[1]); printf("%% of factory value\n");
  	printf("z-axis self test: acceleration trim within : ");
  	printf("%.1f", selfTest[2]); printf("%% of factory value\n");
  	printf("x-axis self test: gyration trim within : ");
  	printf("%.1f", selfTest[3]); printf("%% of factory value\n");
  	printf("y-axis self test: gyration trim within : ");
  	printf("%.1f", selfTest[4]); printf("%% of factory value\n");
  	printf("z-axis self test: gyration trim within : ");
  	printf("%.1f", selfTest[5]); printf("%% of factory value\n");


  	printf("DrumStick #1 initialized for active data mode....\n");

  	// Get sensor resolutions
  	getAres(&aRes);
  	getGres(&gRes);

	int hit_flag_1 = 0;
	int gz_hit_flag_1 = 0;

	int drum1_index = 1;
	int drum1_gy_count_up = 0;
	int drum1_gy_count_down = 0;

	// WHO AM I testing
	I2C_Start(I2C_SCL_2_BASE, I2C_SDA_2_BASE);
	if(!I2C_ReadFromDeviceRegister(I2C_SCL_2_BASE, I2C_SDA_2_BASE, deviceAddress, whoamireg, (alt_u8*)&ReadBuf, buflen, true)){

	}
	I2C_Stop(I2C_SCL_2_BASE, I2C_SDA_2_BASE);
	printf("DrumStick #2 - WHO AM I: %0x\n", ReadBuf[0]);

	printf("Starting Self Tests - DrumStick #2\n");

	//Self Testing
	MPU9250SelfTest(selfTest, I2C_SCL_2_BASE, I2C_SDA_2_BASE);
	printf("x-axis self test: acceleration trim within : ");
	printf("%.1f", selfTest[0]); printf("%% of factory value\n");
	printf("y-axis self test: acceleration trim within : ");
	printf("%.1f", selfTest[1]); printf("%% of factory value\n");
	printf("z-axis self test: acceleration trim within : ");
	printf("%.1f", selfTest[2]); printf("%% of factory value\n");
	printf("x-axis self test: gyration trim within : ");
	printf("%.1f", selfTest[3]); printf("%% of factory value\n");
	printf("y-axis self test: gyration trim within : ");
	printf("%.1f", selfTest[4]); printf("%% of factory value\n");
	printf("z-axis self test: gyration trim within : ");
	printf("%.1f", selfTest[5]); printf("%% of factory value\n");


	printf("DrumStick #2 initialized for active data mode....\n");
	int hit_flag_2 = 0;
	int gz_hit_flag_2 = 0;

	int drum2_index = 1;
	int drum2_gy_count_up = 0;
	int drum2_gy_count_down = 0;

	// init lcd
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Done Testing");

	kick_drum = 0;
	float d1_az_old = 0;
	float d2_az_old = 0;
	// Variables to hold latest sensor data values
	float ax, ay, az, gx, gy, gz;
	int d1_intensity, d2_intensity;
	// Stores the 16-bit signed accelerometer and gyroscope sensor output
	alt_16 accelCount[3];
	alt_16 gyroCount[3];

  while (1) {

	if(kick_drum == 1){
		IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, 6|0x08|16);
		OSTimeDlyHMSM(0, 0, 0, 1);
		IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, INTERRUPT_RESET);
		kick_drum = 0;
	}
	//printf("pedal values %d\n",IORD_ALTERA_AVALON_PIO_DATA(FOOTPEDAL_BASE));

	//********************* DRUMSTICK #1 *********************************************
	// Read the x/y/z accelerometer values
	readAccelData(accelCount, I2C_SCL_BASE,I2C_SDA_BASE);
	 // Read the x/y/z gyroscope values
	readGyroData(gyroCount, I2C_SCL_BASE,I2C_SDA_BASE);

	// Calculating the acceleration values into actual g's
	// Depends on scale being set
	ax = (float)accelCount[0]*aRes;
	ay = (float)accelCount[1]*aRes;
	az = (float)accelCount[2]*aRes;

	// Calculating the gyro values into actual degrees per second
	// Depends on scale being set
	gx = (float)gyroCount[0]*gRes;
	gy = (float)gyroCount[1]*gRes;
	gz = (float)gyroCount[2]*gRes;

	//printf("gx =  %f, gy = %f, gz = %f \n", gx, gy, gz);
	//printf("%f\n", gy);
	/*char str1[7];
	sprintf(str1, "%f", az*NEWTON_SCALE);
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, str1);
*/

	// Horizontal Tracking
	if((gz) < RIGHT_THRESHOLD){
		if(!gz_hit_flag_1 && drum1_index != tom2Const && drum1_index != hihat2Const){
			drum1_index += 1;
			gz_hit_flag_1 = 1;
		}
	}
	else if (gz < RIGHT_THRESHOLD_RESET){
		gz_hit_flag_1 = 0;
	}

	if((gz) > LEFT_THRESHOLD){
		if(!gz_hit_flag_1 && drum1_index != snareConst && drum1_index != crashConst){
			drum1_index -= 1;
			gz_hit_flag_1 = 1;
		}
	}
	else if (gz > LEFT_THRESHOLD_RESET){
		gz_hit_flag_1 = 0;
	}

	// Vertical shift tracking
	if(gy < UP_THRESHOLD){
		drum1_gy_count_up++;
	}
	else{
		drum1_gy_count_up = 0;
	}

	if(gy > DOWN_THRESHOLD){
		drum1_gy_count_down++;
	}
	else{
		drum1_gy_count_down = 0;
	}

	if(drum1_gy_count_up > SHIFT_UP_COUNT && drum1_index < crashConst ){
		drum1_index += SHIFT_ROW;
	}
	if(drum1_gy_count_down > SHIFT_DOWN_COUNT && drum1_index > tom2Const){
		drum1_index -= SHIFT_ROW;
	}

	// Hits
	if((((az)*NEWTON_SCALE) < HIT_THRESHOLD)){
		if(!hit_flag_1 && d1_az_old < az){

			//printf("Hit One: Position %d\n", drum1_index);
			if(d1_az_old*NEWTON_SCALE > SOFT_HIT ){
				d1_intensity = 16;
			}
			if(d1_az_old*NEWTON_SCALE < SOFT_HIT && d1_az_old*NEWTON_SCALE > MEDIUM_HIT ){
				d1_intensity = 32;
			}
			if(d1_az_old*NEWTON_SCALE < MEDIUM_HIT){
				d1_intensity = 48;
			}
			/*char str1[5];
			sprintf(str1, "%d", d1_intensity);

			alt_up_character_lcd_init(myLCD);
			alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
			alt_up_character_lcd_string(myLCD, str1);
*/
			int index_or_intensity = drum1_index|d1_intensity;
			IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, index_or_intensity|INTERRUPT_MASK);
			OSTimeDlyHMSM(0, 0, 0, 1);
			IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, INTERRUPT_RESET);
			hit_flag_1 = 1;
		}

	}
	else if ((((az)*NEWTON_SCALE) > NO_HIT_THRESHOLD)){
		hit_flag_1 = 0;
	}
	d1_az_old = az;

	// ************************ DRUMSTICK #2 **********************************
	// Read the x/y/z accelerometer values
	readAccelData(accelCount, I2C_SCL_2_BASE, I2C_SDA_2_BASE);
	 // Read the x/y/z gyroscope values
	readGyroData(gyroCount, I2C_SCL_2_BASE, I2C_SDA_2_BASE);

	// Calculating the acceleration values into actual g's
	// Depends on scale being set
	ax = (float)accelCount[0]*aRes;
	ay = (float)accelCount[1]*aRes;
	az = (float)accelCount[2]*aRes;

	// Calculating the gyro values into actual degrees per second
	// Depends on scale being set
	gx = (float)gyroCount[0]*gRes;
	gy = (float)gyroCount[1]*gRes;
	gz = (float)gyroCount[2]*gRes;




	// Horizontal Tracking
	if((gz) < RIGHT_THRESHOLD){
		if(!gz_hit_flag_2 && drum2_index != tom2Const && drum2_index != hihat2Const){
			drum2_index += 1;
			gz_hit_flag_2 = 1;
		}
	}
	else if (gz < RIGHT_THRESHOLD_RESET){
		gz_hit_flag_2 = 0;
	}

	if((gz) > LEFT_THRESHOLD){
		if(!gz_hit_flag_2 && drum2_index != snareConst && drum2_index != crashConst){
			drum2_index -= 1;
			gz_hit_flag_2 = 1;
		}
	}
	else if (gz > LEFT_THRESHOLD_RESET){
		gz_hit_flag_2 = 0;
	}

	// Vertical shift tracking
	if(gy < UP_THRESHOLD){
		drum2_gy_count_up++;
	}
	else{
		drum2_gy_count_up = 0;
	}

	if(gy > DOWN_THRESHOLD){
		drum2_gy_count_down++;
	}
	else{
		drum2_gy_count_down = 0;
	}

	if(drum2_gy_count_up > SHIFT_UP_COUNT && drum2_index < crashConst){
		drum2_index += SHIFT_ROW;
	}
	if(drum2_gy_count_down > SHIFT_DOWN_COUNT && drum2_index > tom2Const){
		drum2_index -= SHIFT_ROW;
	}


	// Hits
	if((((az)*NEWTON_SCALE) < HIT_THRESHOLD)){
		if(!hit_flag_2 && ((d2_az_old*NEWTON_SCALE) < az*(NEWTON_SCALE))){

			char str1[10];
			sprintf(str1, "%f", az*NEWTON_SCALE);
			alt_up_character_lcd_init(myLCD);
			alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
			alt_up_character_lcd_string(myLCD, str1);


			//printf("Hit One: Position %d\n", drum1_index);
			if(d2_az_old*NEWTON_SCALE > SOFT_HIT ){
				d2_intensity = 16;
			}
			if(d2_az_old*NEWTON_SCALE < SOFT_HIT && d2_az_old*NEWTON_SCALE > MEDIUM_HIT ){
				d2_intensity = 32;
			}
			if(d2_az_old*NEWTON_SCALE < MEDIUM_HIT){
				d2_intensity = 48;
			}

			/*char str1[5];
			sprintf(str1, "%d", d2_intensity);

			alt_up_character_lcd_init(myLCD);
			alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
			alt_up_character_lcd_string(myLCD, str1);
*/
			int index_or_intensity = drum2_index|d2_intensity;
			IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, index_or_intensity|INTERRUPT_MASK);
			OSTimeDlyHMSM(0, 0, 0, 1);
			IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, INTERRUPT_RESET);
			hit_flag_2 = 1;
		}
	}
	else if ((((az)*NEWTON_SCALE) > NO_HIT_THRESHOLD)){
		hit_flag_2 = 0;
	}
	d2_az_old = az;

	OSTimeDlyHMSM(0, 0, 0, 1);
  }
}

/* The main function creates two task and starts multi-tasking */
int main(void)
{
  myLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_0_NAME);
  alt_up_character_lcd_init(myLCD);
  init_pedal_pio();

  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
  OSStart();
  return 0;
}


void init_pedal_pio() {
	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(FOOTPEDAL_BASE, 0x01);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(FOOTPEDAL_BASE, 0x00);
	/* Register the ISR. */
	alt_irq_register(FOOTPEDAL_IRQ, NULL,  interrupt_isr_pedalPress);
}


void interrupt_isr_pedalPress(void *context, alt_u32 id) {

	/*IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, 6|0x08);
	int i;
	for (i = 0; i < 50; i++) {
		IORD_ALTERA_AVALON_PIO_EDGE_CAP(FOOTPEDAL_BASE);
	}
	IOWR_ALTERA_AVALON_PIO_DATA(DRUM_OUT_BASE, 0x00);*/
	kick_drum = 1;
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(FOOTPEDAL_BASE, 0x01);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(FOOTPEDAL_BASE);
}
/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
