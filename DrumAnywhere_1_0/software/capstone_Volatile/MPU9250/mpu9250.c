#include "mpu9250.h"
#include "includes.h"
#include "system.h"
#include "../i2c/I2C.h"
#include <stdio.h>
#include "altera_avalon_pio_regs.h"
#include <math.h>



/*************************************************************************

Drum Anywhere - G4
Created By - Shivansh Singla and Jake Davidson
Date Created - 1st March, 2016
Description - This is the library used to communicate with MPU9250 using 
I2C protocol library. This is used for reading raw data from the registers,
performing self tests, initialization and calibration. Once the raw data 
is read, it is converted to meaningful acceleration and gyration values 
using appropriate resolution functions.

**************************************************************************/
/*
 * Sets the resolution of the accelerometer
 */

void getAres(float* aRes){
	// Possible accelerometer scales (and their register bit settings) are:
	// 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
	*aRes =  (float)2.0 / (float)32768.0;
}

/*
 * Sets the resolution of the gyroscope
 */
void getGres(float* gRes){
	// Possible gyro scales (and their register bit settings) are:
	// 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS (11).
	*gRes =  (float)1000.0 / (float)32768.0;
}

/*
 * Read the accelerometer data*/
void readAccelData(alt_16 * destination, alt_u32 scl_base, alt_u32 sda_base)
{
  alt_u8 deviceAddress = 0xD0;
  alt_u8 rawData[6];  // x/y/z accel register data stored here
  // Read the six raw data registers into data array
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_XOUT_H, &rawData[0], 6, true);
  I2C_Stop(scl_base, sda_base);

  // Turn the MSB and LSB into a signed 16-bit value
  destination[0] = ((alt_16)rawData[0] << 8) | rawData[1] ;
  destination[1] = ((alt_16)rawData[2] << 8) | rawData[3] ;
  destination[2] = ((alt_16)rawData[4] << 8) | rawData[5] ;
  //printf("destination[0] %d\n", destination[0]);
  //printf("destination[1] %d\n", destination[1]);
  //printf("destination[2] %d\n", destination[2]);
}

/* Read the gyroscope data*/
void readGyroData(alt_16 * destination, alt_u32 scl_base, alt_u32 sda_base)
{
  alt_u8 deviceAddress = 0xD0;
  alt_u8 rawData[6];  // x/y/z gyro register data stored here
  // Read the six raw data registers sequentially into data array
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_XOUT_H, &rawData[0], 6, true);
  I2C_Stop(scl_base, sda_base);

  // Turn the MSB and LSB into a signed 16-bit value
  destination[0] = ((alt_16)rawData[0] << 8) | rawData[1] ;
  destination[1] = ((alt_16)rawData[2] << 8) | rawData[3] ;
  destination[2] = ((alt_16)rawData[4] << 8) | rawData[5] ;
}

/*
 * Initialize the IMU to Active/Read mode
 * */
void initMPU9250(alt_u32 scl_base, alt_u32 sda_base){
  alt_u8 deviceAddress = 0xD0;
  alt_u8 c;
  // wake up device
  // Clear sleep mode bit (6), enable all sensors
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_1, 0x00, 1);

  OSTimeDlyHMSM(0, 0, 0, 100); ; // Wait for all registers to reset

  // Get stable time source
  // Auto select clock source to be PLL gyroscope reference if ready else
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_1, 0x01, 1);

  OSTimeDlyHMSM(0, 0, 0, 200);

  // Configure Gyro and Thermometer
  // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz,
  // respectively;
  // minimum delay time for this setting is 5.9 ms, which means sensor fusion
  // update rates cannot be higher than 1 / 0.0059 = 170 Hz
  // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
  // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!),
  // 8 kHz, or 1 kHz
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, CONFIG, 0x03, 1);

  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  // Use a 200 Hz rate; a rate consistent with the filter update rate
  // determined inset in CONFIG above.
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, SMPLRT_DIV, 0x04, 1);

  // Set gyroscope full scale range
  // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are
  // left-shifted into positions 4:3
  I2C_Stop(scl_base, sda_base);
  // get current GYRO_CONFIG register value
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG, c, 1, true);
  I2C_Stop(scl_base, sda_base);
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x02; // Clear Fchoice bits [1:0]
  c = c & ~0x18; // Clear AFS bits [4:3]
  c = c | GFS_250DPS   << 3; // Set full scale range for the gyro
  // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of
  // GYRO_CONFIG
  // c =| 0x00;
  // Write new GYRO_CONFIG value to register
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG, c, 1);
  I2C_Stop(scl_base, sda_base);

  // Set accelerometer full-scale range configuration
  // Get current ACCEL_CONFIG register value
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG, c, 1, true);
  I2C_Stop(scl_base, sda_base);
  // c = c & ~0xE0; // Clear self-test bits [7:5]
  c = c & ~0x18;  // Clear AFS bits [4:3]
  c = c | AFS_2G << 3; // Set full scale range for the accelerometer
  // Write new ACCEL_CONFIG register value
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG, c, 1);
  I2C_Stop(scl_base, sda_base);

  // Set accelerometer sample rate configuration
  // It is possible to get a 4 kHz sample rate from the accelerometer by
  // choosing 1 for accel_fchoice_b bit [3]; in this case the bandwidth is
  // 1.13 kHz
  // Get current ACCEL_CONFIG2 register value
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG2, c, 1, true);
  I2C_Stop(scl_base, sda_base);
  c = c & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
  c = c | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz

  // Write new ACCEL_CONFIG2 register value
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG2, c, 1);


  // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
  // but all these rates are further reduced by a factor of 5 to 200 Hz because
  // of the SMPLRT_DIV setting

  // Configure Interrupts and Bypass Enable
  // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH
  // until interrupt cleared, clear on read of INT_STATUS, and enable
  // I2C_BYPASS_EN so additional chips can join the I2C bus and all can be
  // controlled by the Arduino as master.
  //I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, INT_PIN_CFG, 0x22, 1);

  // Enable data ready (bit 0) interrupt
  //I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, INT_ENABLE, 0x01, 1);

  I2C_Stop(scl_base, sda_base);
  OSTimeDlyHMSM(0, 0, 0, 100);
}


// Function which accumulates gyro and accelerometer data after device
// initialization. It calculates the average of the at-rest readings and then
// loads the resulting offsets into accelerometer and gyro bias registers.
void calibrateMPU9250(float * gyroBias, float * accelBias, alt_u32 scl_base, alt_u32 sda_base) {

  alt_u8 deviceAddress = 0xD0;
  alt_u8 data[12]; // data array to hold accelerometer and gyro x, y, z, data
  alt_u16 ii, packet_count, fifo_count;
  alt_32 gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

  // reset device
  // Write a one to bit 7 reset bit; toggle reset device
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_1, READ_FLAG, 1);
  OSTimeDlyHMSM(0, 0, 0, 100);
  // get stable time source; Auto select clock source to be PLL gyroscope
  // reference if ready else use the internal oscillator, bits 2:0 = 001
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_1, 0x01, 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_2, 0x00, 1);

  OSTimeDlyHMSM(0, 0, 0, 200);

  // Configure device for bias calculation
  // Disable all interrupts
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, INT_ENABLE, 0x00, 1);
  // Disable FIFO
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, FIFO_EN, 0x00, 1);
  // Turn on internal clock source
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, PWR_MGMT_1, 0x00, 1);
  // Disable I2C master
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, I2C_MST_CTRL, 0x00, 1);
  // Disable FIFO and I2C master modes
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  USER_CTRL, 0x00, 1);
  // Reset FIFO and DMP
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  USER_CTRL, 0x0C, 1);
  OSTimeDlyHMSM(0, 0, 0, 15);

  // Configure MPU6050 gyro and accelerometer for bias calculation
  // Set low-pass filter to 188 Hz
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, CONFIG, 0x01, 1);
  // Set sample rate to 1 kHz
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  SMPLRT_DIV, 0x00, 1);
  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG, 0x00, 1);
  // Set accelerometer full-scale to 2 g, maximum sensitivity
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  ACCEL_CONFIG, 0x00, 1);

  alt_u16  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
  alt_u16  accelsensitivity = 16384; // = 16384 LSB/g

  // Configure FIFO to capture accelerometer and gyro data for bias calculation
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  USER_CTRL, 0x40, 1);  // Enable FIFO
  // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in
  // MPU-9150)
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  FIFO_EN, 0x78, 1);

  OSTimeDlyHMSM(0, 0, 0, 40);  // accumulate 40 samples in 40 milliseconds = 480 bytes

  // At end of sample accumulation, turn off FIFO sensor read
  // Disable gyro and accelerometer sensors for FIFO
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  FIFO_EN, 0x00, 1);


  I2C_Stop(scl_base, sda_base);

  I2C_Start(scl_base, sda_base);
  // Read FIFO sample count
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, FIFO_COUNTH, &data[0], 2, true);
  I2C_Stop(scl_base, sda_base);

  fifo_count = ((alt_u16)data[0] << 8) | data[1];
  // How many sets of full gyro and accelerometer data for averaging
  packet_count = fifo_count/12;

  for (ii = 0; ii < packet_count; ii++)
  {
    alt_16 accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    // Read data for averaging
    I2C_Start(scl_base, sda_base);
	// Read FIFO sample count
	I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, FIFO_R_W, &data[0], 12, true);
	I2C_Stop(scl_base, sda_base);
    // Form signed 16-bit integer for each sample in FIFO
    accel_temp[0] = (alt_16) (((alt_16)data[0] << 8) | data[1]  );
    accel_temp[1] = (alt_16) (((alt_16)data[2] << 8) | data[3]  );
    accel_temp[2] = (alt_16) (((alt_16)data[4] << 8) | data[5]  );
    gyro_temp[0]  = (alt_16) (((alt_16)data[6] << 8) | data[7]  );
    gyro_temp[1]  = (alt_16) (((alt_16)data[8] << 8) | data[9]  );
    gyro_temp[2]  = (alt_16) (((alt_16)data[10] << 8) | data[11]);

    // Sum individual signed 16-bit biases to get accumulated signed 32-bit
    // biases.
    accel_bias[0] += (alt_32) accel_temp[0];
    accel_bias[1] += (alt_32) accel_temp[1];
    accel_bias[2] += (alt_32) accel_temp[2];
    gyro_bias[0]  += (alt_32) gyro_temp[0];
    gyro_bias[1]  += (alt_32) gyro_temp[1];
    gyro_bias[2]  += (alt_32) gyro_temp[2];
  }
  // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
  accel_bias[0] /= (alt_32) packet_count;
  accel_bias[1] /= (alt_32) packet_count;
  accel_bias[2] /= (alt_32) packet_count;
  gyro_bias[0]  /= (alt_32) packet_count;
  gyro_bias[1]  /= (alt_32) packet_count;
  gyro_bias[2]  /= (alt_32) packet_count;

  // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
  if (accel_bias[2] > 0L)
  {
    accel_bias[2] -= (alt_32) accelsensitivity;
  }
  else
  {
    accel_bias[2] += (alt_32) accelsensitivity;
  }

  // Construct the gyro biases for push to the hardware gyro bias registers,
  // which are reset to zero upon device startup.
  // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input
  // format.
  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF;
  // Biases are additive, so change sign on calculated average gyro biases
  data[1] = (-gyro_bias[0]/4)       & 0xFF;
  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
  data[3] = (-gyro_bias[1]/4)       & 0xFF;
  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
  data[5] = (-gyro_bias[2]/4)       & 0xFF;

  // Push gyro biases to hardware registers

  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  XG_OFFSET_H, data[0], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  XG_OFFSET_L, data[1], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, YG_OFFSET_H, data[2], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, YG_OFFSET_L, data[3], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ZG_OFFSET_H, data[4], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ZG_OFFSET_L, data[5], 1);
  I2C_Stop(scl_base, sda_base);


  // Output scaled gyro biases for display in the main program
  gyroBias[0] = (float) gyro_bias[0]/(float) gyrosensitivity;
  gyroBias[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
  gyroBias[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

  // Construct the accelerometer biases for push to the hardware accelerometer
  // bias registers. These registers contain factory trim values which must be
  // added to the calculated accelerometer biases; on boot up these registers
  // will hold non-zero values. In addition, bit 0 of the lower byte must be
  // preserved since it is used for temperature compensation calculations.
  // Accelerometer bias registers expect bias input as 2048 LSB per g, so that
  // the accelerometer biases calculated above must be divided by 8.

  // A place to hold the factory accelerometer trim biases
  alt_32 accel_bias_reg[3] = {0, 0, 0};
  // Read factory accelerometer trim values
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, XA_OFFSET_H, &data[0], 2, true);
  I2C_Stop(scl_base, sda_base);
  accel_bias_reg[0] = (alt_32) (((alt_16)data[0] << 8) | data[1]);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, YA_OFFSET_H, &data[0], 2, true);
  I2C_Stop(scl_base, sda_base);
  accel_bias_reg[1] = (alt_32) (((alt_16)data[0] << 8) | data[1]);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, ZA_OFFSET_H, &data[0], 2, true);
  I2C_Stop(scl_base, sda_base);
  accel_bias_reg[2] = (alt_32) (((alt_16)data[0] << 8) | data[1]);

  // Define mask for temperature compensation bit 0 of lower byte of
  // accelerometer bias registers
  alt_u32 mask = 1uL;
  // Define array to hold mask bit for each accelerometer bias axis
  alt_u8 mask_bit[3] = {0, 0, 0};

  for (ii = 0; ii < 3; ii++)
  {
    // If temperature compensation bit is set, record that fact in mask_bit
    if ((accel_bias_reg[ii] & mask))
    {
      mask_bit[ii] = 0x01;
    }
  }

  // Construct total accelerometer bias, including calculated average
  // accelerometer bias from above
  // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g
  // (16 g full scale)
  accel_bias_reg[0] -= (accel_bias[0]/8);
  accel_bias_reg[1] -= (accel_bias[1]/8);
  accel_bias_reg[2] -= (accel_bias[2]/8);

  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
  data[1] = (accel_bias_reg[0])      & 0xFF;
  // preserve temperature compensation bit when writing back to accelerometer
  // bias registers
  data[1] = data[1] | mask_bit[0];
  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
  data[3] = (accel_bias_reg[1])      & 0xFF;
  // Preserve temperature compensation bit when writing back to accelerometer
  // bias registers
  data[3] = data[3] | mask_bit[1];
  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
  data[5] = (accel_bias_reg[2])      & 0xFF;
  // Preserve temperature compensation bit when writing back to accelerometer
  // bias registers
  data[5] = data[5] | mask_bit[2];

  // Apparently this is not working for the acceleration biases in the MPU-9250
  // Are we handling the temperature correction bit properly?
  // Push accelerometer biases to hardware registers
  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  XA_OFFSET_H, data[0], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress,  XA_OFFSET_L, data[1], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, YA_OFFSET_H, data[2], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, YA_OFFSET_L, data[3], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ZA_OFFSET_H, data[4], 1);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ZA_OFFSET_L, data[5], 1);
  I2C_Stop(scl_base, sda_base);


  // Output scaled accelerometer biases for display in the main program
  accelBias[0] = (float)accel_bias[0]/(float)accelsensitivity;
  accelBias[1] = (float)accel_bias[1]/(float)accelsensitivity;
  accelBias[2] = (float)accel_bias[2]/(float)accelsensitivity;
}


// Accelerometer and gyroscope self test; check calibration wrt factory settings
// Should return percent deviation from factory trim values, +/- 14 or less
// deviation is a pass.
void MPU9250SelfTest(float * destination, alt_u32 scl_base, alt_u32 sda_base)
{
  alt_u8 deviceAddress = 0xD0;
  alt_u8 rawData[6];
  alt_u8 selfTest[6];
  alt_32 gAvg[3] = {0}, aAvg[3] = {0}, aSTAvg[3] = {0}, gSTAvg[3] = {0};
  float factoryTrim[6];
  alt_u8 FS = 0;
  alt_u8 ReadBuf[1];

  I2C_Start(scl_base, sda_base);
  // Set gyro sample rate to 1 kHz
  if(!I2C_ReadFromDeviceRegister(scl_base,sda_base, deviceAddress, 0x75, (alt_u8*)&ReadBuf, 1, true)){

  }
  I2C_Stop(scl_base, sda_base);
  printf("WHO AM I: %0x\n", ReadBuf[0]);


  I2C_Start(scl_base, sda_base);
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, SMPLRT_DIV, 0x00, 1);
  // Set gyro sample rate to 1 kHz and DLPF to 92 Hz
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, CONFIG, 0x02, 1);
  // Set full scale range for the gyro to 250 dps
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG,  0x10, 1);
  // Set accelerometer rate to 1 kHz and bandwidth to 92 Hz
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG2, 0x02, 1);
  // Set full scale range for the accelerometer to 2 g
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG, 1<<FS, 1);
  I2C_Stop(scl_base, sda_base);


  int ii;
  // Get average current values of gyro and acclerometer
  for ( ii = 0; ii < 100; ii= ii+1) {

  	//printf("BHW::ii = %d \n", ii );
  	I2C_Start(scl_base, sda_base);
    // Read the six raw data registers into data array
	I2C_ReadFromDeviceRegister(scl_base, sda_base, 0xD0, ACCEL_XOUT_H, &rawData[0],6, true);
    I2C_Stop(scl_base, sda_base);

	// Turn the MSB and LSB into a signed 16-bit value
    aAvg[0] += (alt_16)(((alt_16)rawData[0] << 8) | rawData[1]) ;
    aAvg[1] += (alt_16)(((alt_16)rawData[2] << 8) | rawData[3]) ;
    aAvg[2] += (alt_16)(((alt_16)rawData[4] << 8) | rawData[5]) ;

  	I2C_Start(scl_base, sda_base);

    // Read the six raw data registers sequentially into data array
	I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_XOUT_H, &rawData[0], 6, true);
    I2C_Stop(scl_base, sda_base);

	// Turn the MSB and LSB into a signed 16-bit value
    gAvg[0] += (alt_16)(((alt_16)rawData[0] << 8) | rawData[1]) ;
    gAvg[1] += (alt_16)(((alt_16)rawData[2] << 8) | rawData[3]) ;
    gAvg[2] += (alt_16)(((alt_16)rawData[4] << 8) | rawData[5]) ;

  }

  // Get average of 200 values and store as average current readings
  for ( ii =0; ii < 3; ii = ii+1)
  {
    aAvg[ii] /= 100;
    gAvg[ii] /= 100;
  }
  I2C_Start(scl_base, sda_base);


  // Configure the accelerometer for self-test
  // Enable self test on all three axes and set accelerometer range to +/- 2 g
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG, 0xE0, 1);
  // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG, 0xF0, 1);

  I2C_Stop(scl_base, sda_base);
  OSTimeDlyHMSM(0, 0, 0, 25);  // Delay a while to let the device stabilize


  // Get average self-test values of gyro and acclerometer
  for ( ii =0; ii < 100; ii = ii+1)
  {
	I2C_Start(scl_base, sda_base);
    // Read the six raw data registers into data array
	I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_XOUT_H, &rawData[0], 6, true);
    I2C_Stop(scl_base, sda_base);

	// Turn the MSB and LSB into a signed 16-bit value
    aSTAvg[0] += (alt_16)(((alt_16)rawData[0] << 8) | rawData[1]) ;
    aSTAvg[1] += (alt_16)(((alt_16)rawData[2] << 8) | rawData[3]) ;
    aSTAvg[2] += (alt_16)(((alt_16)rawData[4] << 8) | rawData[5]) ;

    I2C_Start(scl_base, sda_base);
    // Read the six raw data registers sequentially into data array
	I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_XOUT_H, &rawData[0], 6, true);
    I2C_Stop(scl_base, sda_base);

	// Turn the MSB and LSB into a signed 16-bit value
    gSTAvg[0] += (alt_16)(((alt_16)rawData[0] << 8) | rawData[1]) ;
    gSTAvg[1] += (alt_16)(((alt_16)rawData[2] << 8) | rawData[3]) ;
    gSTAvg[2] += (alt_16)(((alt_16)rawData[4] << 8) | rawData[5]) ;
  }

  // Get average of 200 values and store as average self-test readings
  for ( ii =0; ii < 3; ii = ii+1)
  {
    aSTAvg[ii] /= 100;
    gSTAvg[ii] /= 100;
  }

  I2C_Start(scl_base, sda_base);

  // Configure the gyro and accelerometer for normal operation
  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, ACCEL_CONFIG, 0x00, 1);
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);

  I2C_WriteToDeviceRegister(scl_base, sda_base, deviceAddress, GYRO_CONFIG, 0x10, 1);
  OSTimeDlyHMSM(0, 0, 0, 25);;  // Delay a while to let the device stabilize

  // Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
  // X-axis accel self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_X_ACCEL, &selfTest[0] , 1, true);
  // Y-axis accel self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_Y_ACCEL, &selfTest[1] , 1, true);
  // Z-axis accel self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_Z_ACCEL, &selfTest[2] , 1, true);
  // X-axis gyro self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_X_GYRO, &selfTest[3] , 1, true);
  // Y-axis gyro self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_Y_GYRO, &selfTest[4] , 1, true);
  // Z-axis gyro self-test results
  I2C_Stop(scl_base, sda_base);
  I2C_Start(scl_base, sda_base);
  I2C_ReadFromDeviceRegister(scl_base, sda_base, deviceAddress, SELF_TEST_Z_GYRO, &selfTest[5] , 1, true);

  I2C_Stop(scl_base, sda_base);


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
	//printf("aSTAvg = %f\n",(float)aSTAvg[ii]);
	//printf("aAvg = %f\n",(float)aAvg[ii]);
	//printf("factoryTrim = %f\n",(float)factoryTrim[ii]);
    // Report percent differences
    destination[ii] = 100.0 * (((float)(aSTAvg[ii] - aAvg[ii])) / factoryTrim[ii]);
    // Report percent differences
    destination[ii+3] = 100.0*(((float)(gSTAvg[ii] - gAvg[ii]))/factoryTrim[ii+3]);
    //printf("destination[%d] = %f\n",ii, destination[ii]);
    // printf("destination[%d] = %f\n",ii+3, destination[ii+3]);
    //printf("test1 difference: %f\n",aSTAvg[ii] - aAvg[ii]);
    //printf("test q: %f\n",416 / 6479.571777);

  }
}
