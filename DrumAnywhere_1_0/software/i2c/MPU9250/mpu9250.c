#include "mpu9250.h"
#include "includes.h"
#include "system.h"
#include "../i2c/I2C.h"
#include <stdio.h>
#include "altera_avalon_pio_regs.h"
#include <math.h>



// Accelerometer and gyroscope self test; check calibration wrt factory settings
// Should return percent deviation from factory trim values, +/- 14 or less
// deviation is a pass.
void MPU9250SelfTest(float * destination)
{
  alt_u8 deviceAddress = 0xD0;
  alt_u8 rawData[6];
  alt_u8 selfTest[6];
  alt_u32 gAvg[3] = {0}, aAvg[3] = {0}, aSTAvg[3] = {0}, gSTAvg[3] = {0};
  float factoryTrim[6];
  alt_u8 FS = 0;
  alt_u8 ReadBuf[1];
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  // Set gyro sample rate to 1 kHz
  if(!I2C_ReadFromDeviceRegister(I2C_SCL_BASE,I2C_SDA_BASE, deviceAddress, 0x75, (alt_u8*)&ReadBuf, 1, true)){

  }
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  printf("WHO AM I: %0x\n", ReadBuf[0]);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SMPLRT_DIV,(alt_u8*) 0x00, 1);
  // Set gyro sample rate to 1 kHz and DLPF to 92 Hz
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, CONFIG, (alt_u8*)0x02, 1);
  // Set full scale range for the gyro to 250 dps
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, GYRO_CONFIG, (alt_u8*) (1<<FS), 1);
  // Set accelerometer rate to 1 kHz and bandwidth to 92 Hz
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, ACCEL_CONFIG2, (alt_u8*)0x02, 1);
  // Set full scale range for the accelerometer to 2 g
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, ACCEL_CONFIG,(alt_u8*) (1<<FS), 1);
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);


  int ii;
  // Get average current values of gyro and acclerometer
  for ( ii = 0; ii < 200; ii= ii+1) {

  	printf("BHW::ii = %d \n", ii );
  	I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
    // Read the six raw data registers into data array
	I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, 0xD0, ACCEL_XOUT_H, (alt_u8*)&rawData[0],6, true);
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);

	// Turn the MSB and LSB into a signed 16-bit value
    aAvg[0] += (alt_u16)(((alt_u16)rawData[0] << 8) | rawData[1]) ;
    aAvg[1] += (alt_u16)(((alt_u16)rawData[2] << 8) | rawData[3]) ;
    aAvg[2] += (alt_u16)(((alt_u16)rawData[4] << 8) | rawData[5]) ;

  	I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);

    // Read the six raw data registers sequentially into data array
	I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, GYRO_XOUT_H, (alt_u8*)&rawData[0], 6, true);
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);

	// Turn the MSB and LSB into a signed 16-bit value
    gAvg[0] += (alt_u16)(((alt_u16)rawData[0] << 8) | rawData[1]) ;
    gAvg[1] += (alt_u16)(((alt_u16)rawData[2] << 8) | rawData[3]) ;
    gAvg[2] += (alt_u16)(((alt_u16)rawData[4] << 8) | rawData[5]) ;

  }

  // Get average of 200 values and store as average current readings
  for ( ii =0; ii < 3; ii = ii+1)
  {
    aAvg[ii] /= 200;
    gAvg[ii] /= 200;
  }
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);


  // Configure the accelerometer for self-test
  // Enable self test on all three axes and set accelerometer range to +/- 2 g
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, ACCEL_CONFIG, (alt_u8*)0xE0, 1);
  // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, GYRO_CONFIG, (alt_u8*)0xE0, 1);

  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  OSTimeDlyHMSM(0, 0, 0, 25);;  // Delay a while to let the device stabilize


  // Get average self-test values of gyro and acclerometer
  for ( ii =0; ii < 200; ii = ii+1)
  {
	I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
    // Read the six raw data registers into data array
	I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, ACCEL_XOUT_H, &rawData[0], 6, true);
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);

	// Turn the MSB and LSB into a signed 16-bit value
    aSTAvg[0] += (alt_u16)(((alt_u16)rawData[0] << 8) | rawData[1]) ;
    aSTAvg[1] += (alt_u16)(((alt_u16)rawData[2] << 8) | rawData[3]) ;
    aSTAvg[2] += (alt_u16)(((alt_u16)rawData[4] << 8) | rawData[5]) ;

    I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
    // Read the six raw data registers sequentially into data array
	I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, GYRO_XOUT_H, &rawData[0], 6, true);
    I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);

	// Turn the MSB and LSB into a signed 16-bit value
    gSTAvg[0] += (alt_u16)(((alt_u16)rawData[0] << 8) | rawData[1]) ;
    gSTAvg[1] += (alt_u16)(((alt_u16)rawData[2] << 8) | rawData[3]) ;
    gSTAvg[2] += (alt_u16)(((alt_u16)rawData[4] << 8) | rawData[5]) ;
  }

  // Get average of 200 values and store as average self-test readings
  for ( ii =0; ii < 3; ii = ii+1)
  {
    aSTAvg[ii] /= 200;
    gSTAvg[ii] /= 200;
  }

  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);

  // Configure the gyro and accelerometer for normal operation
  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, ACCEL_CONFIG, (alt_u8*)0x00, 1);
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);

  I2C_WriteToDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, GYRO_CONFIG, (alt_u8*)0x00, 1);
  OSTimeDlyHMSM(0, 0, 0, 25);;  // Delay a while to let the device stabilize

  // Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
  // X-axis accel self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_X_ACCEL, &selfTest[0] , 1, true);
  // Y-axis accel self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_Y_ACCEL, &selfTest[1] , 1, true);
  // Z-axis accel self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_Z_ACCEL, &selfTest[2] , 1, true);
  // X-axis gyro self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_X_GYRO, &selfTest[3] , 1, true);
  // Y-axis gyro self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_Y_GYRO, &selfTest[4] , 1, true);
  // Z-axis gyro self-test results
  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_Start(I2C_SCL_BASE, I2C_SDA_BASE);
  I2C_ReadFromDeviceRegister(I2C_SCL_BASE, I2C_SDA_BASE, deviceAddress, SELF_TEST_Z_GYRO, &selfTest[5] , 1, true);

  I2C_Stop(I2C_SCL_BASE, I2C_SDA_BASE);


  // Retrieve factory self-test value from self-test code reads
  // FT[Xa] factory trim calculation
  factoryTrim[0] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[0] - 1.0) ));
  // FT[Ya] factory trim calculation
  factoryTrim[1] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[1] - 1.0) ));
  // FT[Za] factory trim calculation
  factoryTrim[2] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[2] - 1.0) ));
  // FT[Xg] factory trim calculation
  factoryTrim[3] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[3] - 1.0) ));
  // FT[Yg] factory trim calculation
  factoryTrim[4] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[4] - 1.0) ));
  // FT[Zg] factory trim calculation
  factoryTrim[5] = (float)(2620/1<<FS)*(pow(1.01 ,((float)selfTest[5] - 1.0) ));

  // Report results as a ratio of (STR - FT)/FT; the change from Factory Trim
  // of the Self-Test Response
  // To get percent, must multiply by 100
  for ( ii =0; ii < 3; ii = ii+1)
  {
    // Report percent differences
    destination[ii] = 100.0 * ((float)(aSTAvg[ii] - aAvg[ii])) / factoryTrim[ii]
      - 100.;
    // Report percent differences
    destination[ii+3] = 100.0*((float)(gSTAvg[ii] - gAvg[ii]))/factoryTrim[ii+3]
      - 100.;
  }
}
