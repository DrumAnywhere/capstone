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


#include <stdio.h>
#include "includes.h"
#include "i2c/I2C.h"
#include "MPU9250/mpu9250.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2



/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata){

	//Temporary buffer to read into
	alt_u8 ReadBuf[6];
	//Device address
	alt_u8 deviceAddress = 0xD0;
	//WHO AM I Register
	alt_u8 whoamireg = 0x75;
	alt_u16 buflen = 1;
	// Self tests, biases and resolution for DPS conversions
	float selfTest[6];
	float gyroBias[3]  = {0, 0, 0},	accelBias[3] = {0, 0, 0};
	float aRes, gRes;


    // WHO AM I testing
  	I2C_Start(I2C_SCL_BASE,I2C_SDA_BASE);
    if(!I2C_ReadFromDeviceRegister(I2C_SCL_BASE,I2C_SDA_BASE, deviceAddress, whoamireg, (alt_u8*)&ReadBuf, buflen, true)){

	  }
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
    printf("WHO AM I: %0x\n", ReadBuf[0]);

    printf("Starting Self Tests\n");
    //Self Testing
    MPU9250SelfTest(selfTest);
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

  	/*//Calibrate MPU9250 and load Bias into registers
  	calibrateMPU9250(gyroBias, accelBias);


  	printf("MPU9250 Bias Values\n");
  	printf("Ax: %i \n", 1000*accelBias[0]);
  	printf("Ay: %i \n", 1000*accelBias[1]);
  	printf("Az: %i \n", 1000*accelBias[2]);
  	printf("Gx: %f \n", gyroBias[0]);
  	printf("Gy: %f \n", gyroBias[1]);
  	printf("Gz: %f \n", gyroBias[2]);
	*/

  	// Initialize device for active mode read of acclerometer, gyroscope, and temperature
  	initMPU9250();
  	printf("MPU9250 initialized for active data mode....\n");

  	// Get sensor resolutions
  	getAres(&aRes);
  	getGres(&gRes);
  	printf("ares %f\n", aRes);
  	printf("gres %f\n", gRes);


  while (1)
  {
	// Variables to hold latest sensor data values
	float ax, ay, az, gx, gy, gz;
	// Stores the 16-bit signed accelerometer and gyroscope sensor output
	alt_16 accelCount[3];
	alt_16 gyroCount[3];

    printf("Reading Values\n");
	readAccelData(accelCount); // Read the x/y/z accelerometer values
	// Calculating the acceleration values into actual g's
	// Depends on scale being set
	ax = (float)accelCount[0]*aRes;
	ay = (float)accelCount[1]*aRes;
	az = (float)accelCount[2]*aRes;


	readGyroData(gyroCount); // Read the x/y/z gyroscope values
	// Calculating the gyro values into actual degrees per second
	// Depends on scale being set
	gx = (float)gyroCount[0]*gRes;
	gy = (float)gyroCount[1]*gRes;
	gz = (float)gyroCount[2]*gRes;

  	printf("Ax: %f, ",1000*ax);
  	printf("Ay: %f, ", 1000*ay);
  	printf("Az: %f ",1000*az);
  	printf("mg \n");
  	printf("Gx: %f, ",gx);
  	printf("Gy: %f, ", gy);
  	printf("Gz: %f ", gz);
  	printf("deg/sec \n");
	OSTimeDlyHMSM(0, 0, 1, 0);


  }
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata)
{


  while (1)
  { 

    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}
/* The main function creates two task and starts multi-tasking */
int main(void)
{
  
  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSStart();
  return 0;
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