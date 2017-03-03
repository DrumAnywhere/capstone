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

  alt_u8 ReadBuf[6];
  //alt_u8 deviceAddress1 = 0x68;
  alt_u8 deviceAddress2 = 0xD0;
  alt_u8 registerAddress = 0x75;
  alt_u16 buflen = 1;
  float selfTest[6];

    I2C_Start(I2C_SCL_BASE,I2C_SDA_BASE);
    if(!I2C_ReadFromDeviceRegister(I2C_SCL_BASE,I2C_SDA_BASE, deviceAddress2, registerAddress, (alt_u8*)&ReadBuf, buflen, true)){

	  }
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
    printf("WHO AM I: %0x\n", ReadBuf[0]);
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


  	alt_u8 Data[2];
    alt_32 xaxis[3] = {0};
    I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
    // Read the six raw data registers into data array
    I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, 0xD0, ACCEL_ZOUT_H, &Data[0],2, true);
    // Turn the MSB and LSB into a signed 16-bit value
    xaxis[0] = (alt_16)(((alt_16)Data[0] << 8) | Data[1]) ;
  while (1)
  { 

	// Read the six raw data registers into data array
	I2C_ReadMore(I2C_SCL_BASE, I2C_SDA_BASE, &Data[0],2, true);
	// Turn the MSB and LSB into a signed 16-bit value
	xaxis[0] = (alt_16)(((alt_16)Data[0] << 8) | Data[1]) ;
	//printf("Hello from task2\n");
	printf("xaxis = %d\n", xaxis[0]);

	/* printf("Hello from task1\n");
    I2C_Start(I2C_SCL_BASE,I2C_SDA_BASE);
	if(!I2C_ReadFromDeviceRegister(I2C_SCL_BASE,I2C_SDA_BASE, 0xD0, registerAddress, (alt_u8*)&ReadBuf[0], 6, true)){

	  }
	printf("WHO AM I: %0x\n", ReadBuf[0]);
	I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
	*/
    OSTimeDlyHMSM(0, 0, 0, 1);

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
