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
#include "system.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "includes.h"
#include "altera_up_avalon_audio.h"
#include "altera_up_avalon_audio_and_video_config.h"
#include "altera_avalon_pio_regs.h"
#include "altera_up_avalon_character_lcd.h"
#include <efs.h>
#include <ls.h>
#include <errno.h>
#include "altera_avalon_pio_regs.h"

euint8* sdRead(char* fileName, EmbeddedFileSystem *efsl, File *readFile);
void audioInit(alt_up_av_config_dev * audio_config_dev);
void parseWav(euint8* fileBuffer, unsigned long numberSamples, unsigned int* fileBufL);
void sdInit(EmbeddedFileSystem *efsl, File *readFile);
void playSound(unsigned long NumberSamples, unsigned int* wav);

#define PI 3.14159265
#define     BUFFER_SIZE    128
alt_up_character_lcd_dev* myLCD;
int button;
alt_up_audio_dev * audio_dev;

int main(void) {

	// Create EFSL containers
	EmbeddedFileSystem efsl;
	File readFile;

	alt_up_av_config_dev * audio_config_dev;
	audio_config_dev = alt_up_av_config_open_dev("/dev/audio_and_video_config_0");
	myLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_0_NAME);
	alt_up_character_lcd_init(myLCD);

	char *fileNames[7];
	fileNames[0] = "snare2.wav";
	fileNames[1] = "crash.wav";
	fileNames[2] = "hihat.wav";
	fileNames[3] = "hithat2.wav";
	fileNames[4] = "kick.wav";
	fileNames[5] = "tom.wav";
	fileNames[6] = "tom2.wav";

	unsigned long snareNumberSamples = 7235;
	unsigned long crashNumberSamples = 63342;
	unsigned long hihatNumberSamples = 98218;
	unsigned long hihat2NumberSamples = 6313;
	unsigned long kickNumberSamples = 25448;
	unsigned long tomNumberSamples = 34977;
	unsigned long tom2NumberSamples = 16926;

	unsigned int snare[snareNumberSamples];
	unsigned int crash[crashNumberSamples];
	unsigned int hihat[hihatNumberSamples];
	unsigned int hihat2[hihat2NumberSamples];
	unsigned int kick[kickNumberSamples];
	unsigned int tom[tomNumberSamples];
	unsigned int tom2[tom2NumberSamples];

	euint8* snareTemp = malloc(29102 * sizeof(eint8));
	euint8* crashTemp = malloc(253412 * sizeof(eint8));
	euint8* hihatTemp = malloc(392916 * sizeof(eint8));
	euint8* hihat2Temp  = malloc(25296 * sizeof(eint8));
	euint8* kickTemp = malloc(104548 * sizeof(eint8));
	euint8* tomTemp = malloc(139952 * sizeof(eint8));
	euint8* tom2Temp = malloc(67748 * sizeof(eint8));

	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Loading Sounds");

	sdInit(&efsl, &readFile);
	snareTemp = sdRead(fileNames[0], &efsl, &readFile);
	crashTemp = sdRead(fileNames[1], &efsl, &readFile);
	hihatTemp = sdRead(fileNames[2], &efsl, &readFile);
	hihat2Temp = sdRead(fileNames[3], &efsl, &readFile);
	kickTemp = sdRead(fileNames[4], &efsl, &readFile);
	tomTemp = sdRead(fileNames[5], &efsl, &readFile);
	tom2Temp = sdRead(fileNames[6], &efsl, &readFile);


	parseWav(snareTemp, snareNumberSamples, snare);
	parseWav(crashTemp, crashNumberSamples, crash);
	parseWav(hihatTemp, hihatNumberSamples, hihat);
	parseWav(hihat2Temp, hihat2NumberSamples, hihat2);
	parseWav(kickTemp, kickNumberSamples, kick);
	parseWav(tomTemp, tomNumberSamples, tom);
	parseWav(tom2Temp, tom2NumberSamples, tom2);

	audioInit(audio_config_dev);
	printf("Ready To Play");
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Ready To Play");

	while(1) {

		button = IORD_ALTERA_AVALON_PIO_DATA(0x1109060);


		switch (button) {

			case 7 :
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "Bass");
				playSound(tom2NumberSamples, tom2);
				while(button == 7)button = IORD_ALTERA_AVALON_PIO_DATA(0x1109060);
				break;
			case 11:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "Snare");
				playSound(snareNumberSamples, snare);
				while(button == 11)button = IORD_ALTERA_AVALON_PIO_DATA(0x1109060);
				break;
			case 13:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "Crash");
				playSound(crashNumberSamples, crash);
				while(button == 13)button = IORD_ALTERA_AVALON_PIO_DATA(0x1109060);
				break;
			case 14:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "Hi-Hat");
				playSound(hihatNumberSamples, hihat);
				while(button == 14)button = IORD_ALTERA_AVALON_PIO_DATA(0x1109060);
				break;

		}
	}
}

void playSound(unsigned long NumberSamples, unsigned int* wav) {
	int count = 0;
	while(count <= NumberSamples - 128) {
		int fifospace = alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT);
		if(fifospace >= 128) {
				alt_up_audio_write_fifo(audio_dev, wav + count, 128, ALT_UP_AUDIO_RIGHT);
				alt_up_audio_write_fifo(audio_dev, wav + count, 128, ALT_UP_AUDIO_LEFT);
				count += 128;
		}
	}
}

void sdInit(EmbeddedFileSystem *efsl, File *readFile) {

	// Initialises the filesystem on the SD card, if the filesystem does not
	// init properly then it displays an error message.
	printf("Attempting to init filesystem");
	int ret = efs_init(efsl, SD_CARD_SPI_MASTER_NAME);

	// Initialize efsl
	if(ret != 0)
	{
	printf("...could not initialize filesystem.\n");
	}
	else
	printf("...success!\n");
}


void audioInit(alt_up_av_config_dev * audio_config_dev) {

	if ( audio_config_dev == NULL)
		printf("Error: could not open audio config device \n");
	else
		printf("Opened audio config device \n");

	audio_dev = alt_up_audio_open_dev ("/dev/audio_0");
	if ( audio_dev == NULL)
		printf("Error: could not open audio device \n");
	else
		printf("Opened audio device \n");

	alt_up_av_config_reset(audio_config_dev);
	alt_up_audio_reset_audio_core(audio_dev);

	/* Write to configuration registers in the audio codec; see datasheet for what these values mean */
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x0, 0x17);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x1, 0x17);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x2, 0x68);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x3, 0x68);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x4, 0x15);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x5, 0x06);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x6, 0x00);
	alt_up_av_config_write_audio_cfg_register(audio_config_dev, 0x8, 0x22);
}


// sdRead will read wav file given its filename and the efsl needed variables
euint8* sdRead(char* fileName, EmbeddedFileSystem *efsl, File *readFile) {

	// read all files from sdCard

	// You could do some scanning of the file system here using the UNIX-like
	// API functions such as "ls_openDir(...)" and "ls_getNext(...). Reference
	// the included PDF for the documentation to do such a thing. This example
	// simply shows reading a file with a known filename.

	// Open the test file
	printf("\nAttempting to open file: \"%s\"\n", fileName);

	int ret1 = file_fopen(readFile, &efsl->myFs, fileName, 'r');
	printf("ret1: %d\n", ret1);
	if (ret1 != 0)
	{
		printf("Error:\tCould not open file\n");
	}
	else
	{
		printf("Reading file...\n");
	}

	euint8* fileBuffer = malloc(readFile->FileSize * sizeof(eint8));
	if (!(fileBuffer)) {
		perror("malloc failed!\n");
	}

	// Read all the file's contents into the buffer. See the file_fread(...) function
	// for the ability to read chunks of the file at a time, which is desirable for
	// larger files.
	unsigned int bytesRead = file_read(readFile, readFile->FileSize, fileBuffer);
	printf("%u bytes read from the file\n", bytesRead);

	// Close the file
	if (file_fclose(readFile) != 0) {
		printf("Error:\tCould not close file properly\n");
	}

	// Unmount the file system
	fs_umount(&efsl->myFs);
	return fileBuffer;
}


void parseWav(euint8* fileBuffer, unsigned long numberSamples, unsigned int *fileBufL) {

//	int channels = fileBuffer[22] | fileBuffer[23] << 8;
//	int bits_per_sample = fileBuffer[34] | fileBuffer[35] << 8;
//	unsigned int data_size = fileBuffer[40] | (fileBuffer[41] << 8) | (fileBuffer[42] << 16) | (fileBuffer[43] << 24);
//	unsigned int samples = (8*data_size)/(channels*bits_per_sample);
	//memcpy(sdCard->numberSamples, samples, sizeof(unsigned int));

	int i;
	unsigned int temp;
	for(i = 0; i < numberSamples; i++) {
		temp = (unsigned int) (fileBuffer[i*4+44] | (fileBuffer[i*4+45] << 8));
		fileBufL[i] = temp;
	}
}
		//Header Info
//	memcpy(&sdCard->wavHeader.riff, sdCard->fileBuffer, 4*sizeof(char));
//
//	int temp = sdCard->fileBuffer[4] | (sdCard->fileBuffer[5] << 8) | (sdCard->fileBuffer[6] << 16) | (sdCard->fileBuffer[7] << 24);

//	memcpy(&sdCard->wavHeader.overall_size, &temp, sizeof(unsigned int));
//	memcpy(&sdCard->wavHeader.wave, sdCard->fileBuffer+8, sizeof(char)*4);
//	memcpy(&sdCard->wavHeader.fmt_chunk_marker, sdCard->fileBuffer+12, sizeof(char)*4);
//	temp = sdCard->fileBuffer[16] | (sdCard->fileBuffer[17] << 8) | (sdCard->fileBuffer[18] << 16) | (sdCard->fileBuffer[19] << 24);
//	memcpy(&sdCard->wavHeader.length_of_fmt, &temp, sizeof(unsigned int));
//	temp = sdCard->fileBuffer[20] | sdCard->fileBuffer[21];
//	memcpy(&sdCard->wavHeader.format_type, &temp, sizeof(unsigned int));
//	unsigned int temp = sdCard->fileBuffer[22] | sdCard->fileBuffer[23] << 8;
//	memcpy(&sdCard->channels, &temp, sizeof(unsigned int));
//	temp = sdCard->fileBuffer[24] | (sdCard->fileBuffer[25] << 8) | (sdCard->fileBuffer[26] << 16) | (sdCard->fileBuffer[27] << 24);
//	memcpy(&sdCard->wavHeader.sample_rate, &temp, sizeof(unsigned int));
//	temp = sdCard->fileBuffer[28] | (sdCard->fileBuffer[29] << 8) | (sdCard->fileBuffer[30] << 16) | (sdCard->fileBuffer[31] << 24);
//	memcpy(&sdCard->wavHeader.byterate, &temp, sizeof(unsigned int));
//	temp = sdCard->fileBuffer[32] | sdCard->fileBuffer[33] << 8;
//	memcpy(&sdCard->wavHeader.block_align, &temp, sizeof(unsigned int));
//	temp = sdCard->fileBuffer[34] | sdCard->fileBuffer[35] << 8;
//	memcpy(&sdCard->bits_per_sample, &temp, sizeof(unsigned int));
//	memcpy(&sdCard->wavHeader.data_chunk_header, sdCard->fileBuffer+36, sizeof(char)*4);
//	temp = sdCard->fileBuffer[40] | (sdCard->fileBuffer[41] << 8) | (sdCard->fileBuffer[42] << 16) | (sdCard->fileBuffer[43] << 24);
//	memcpy(&sdCard->data_size, &temp, sizeof(unsigned int));
//	sdCard->numberSamples = (8*sdCard->data_size)/(sdCard->channels*sdCard->bits_per_sample);

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
