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


/* Created By Kelvin Liang and Justin Ferris 
   Drum Anywhere G4
   Date: Feb 22, 2017
   Description: audio.c is the integration of our spi interface and our audio codec.
   				code has been written in a way for us to demo our audio output and sd card input
   				4 buttons have been mapped on the Altera DE2, each button corresponds to a drum sound
   				Snare, Bass, HiHat and a Crash are used for this code. For the actual implementation we
   				will have 7 drum sounds and instead of buttons sounds will be played according to drum hits
*/


#include <stdio.h>
#include <system.h>
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


// method declarations
euint8* sdRead(char* fileName, EmbeddedFileSystem *efsl, File *readFile);
void audioInit(alt_up_av_config_dev * audio_config_dev);
void parseWav(euint8* fileBuffer, unsigned long numberSamples, unsigned int* fileBufL);
void sdInit(EmbeddedFileSystem *efsl, File *readFile);
void playSound(unsigned long NumberSamples, unsigned int* wav);
void playSoundAmp(unsigned long NumberSamples, unsigned int* wav, int scale);
void addWav(unsigned int* wav1, unsigned int* wav2, unsigned int* arr);
//static void init_button_pio();

// globals 
alt_up_character_lcd_dev* myLCD;
int button;
alt_up_audio_dev * audio_dev;
//volatile int edge_capture;
//Queue stuff
//OS_EVENT* myQueue;
//#define QSIZE 25
//void* myQueueArray[QSIZE];

// sample sizes for sounds
const unsigned int snareNumberSamples = 7235;
const unsigned int crashNumberSamples = 63342;
const unsigned int hihatNumberSamples = 98218;
const unsigned int hihat2NumberSamples = 6313;
const unsigned int kickNumberSamples = 25448;
const unsigned int tomNumberSamples = 57006;
const unsigned int tom2NumberSamples = 16926;

//static void interrupt_isr_buttonPress(void *context, alt_u32 id) {
//
//	volatile int* edge_capture_ptr = (volatile int*) context;
//	*edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE);
//	alt_up_character_lcd_init(myLCD);
//	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
//	alt_up_character_lcd_string(myLCD, "buttons:");
////	button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
////	OSQPost(myQueue, &button);
//	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE, 0x01);
//}

int main(void) {

//	OSInit();
//	//init Queue
//	myQueue = OSQCreate(myQueueArray, QSIZE);
//	if(myQueue == NULL) {
//		printf("failed to create que");
//	}

	// Create EFSL containers
	EmbeddedFileSystem efsl;
	File readFile;

	// needed objects for software init
	alt_up_av_config_dev * audio_config_dev;
	audio_config_dev = alt_up_av_config_open_dev("/dev/audio_and_video_config_0");
	myLCD = alt_up_character_lcd_open_dev(CHARACTER_LCD_0_NAME);
	alt_up_character_lcd_init(myLCD);

	// Drum Sounds filenames
	char *fileNames[7];
	fileNames[0] = "snare2.wav";
	fileNames[1] = "crash.wav";
	fileNames[2] = "hihat.wav";
	fileNames[3] = "hithat2.wav";
	fileNames[4] = "kick.wav";
	fileNames[5] = "tom.wav";
	fileNames[6] = "tom2.wav";

	// arrays to hold waveforms
	unsigned int snare[snareNumberSamples];
	unsigned int crash[crashNumberSamples];
	unsigned int hihat[hihatNumberSamples];
	unsigned int hihat2[hihat2NumberSamples];
	unsigned int kick[kickNumberSamples];
	unsigned int tom[tomNumberSamples];
	unsigned int tom2[tom2NumberSamples];

	// arrays for reading all data from sd card
	euint8* snareTemp = malloc(29102 * sizeof(eint8));
	euint8* crashTemp = malloc(253412 * sizeof(eint8));
	euint8* hihatTemp = malloc(392916 * sizeof(eint8));
	euint8* hihat2Temp  = malloc(25296 * sizeof(eint8));
	euint8* kickTemp = malloc(104548 * sizeof(eint8));
	euint8* tomTemp = malloc(228186 * sizeof(eint8));
	euint8* tom2Temp = malloc(67748 * sizeof(eint8));

	// init lcd
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Loading Sounds");

	// init spi interface
	sdInit(&efsl, &readFile);

	// read files from sd card
	snareTemp = sdRead(fileNames[0], &efsl, &readFile);
	crashTemp = sdRead(fileNames[1], &efsl, &readFile);
	hihatTemp = sdRead(fileNames[2], &efsl, &readFile);
	hihat2Temp = sdRead(fileNames[3], &efsl, &readFile);
	kickTemp = sdRead(fileNames[4], &efsl, &readFile);
	tomTemp = sdRead(fileNames[5], &efsl, &readFile);
	tom2Temp = sdRead(fileNames[6], &efsl, &readFile);

	// parse the actual data from the .wav files
	parseWav(snareTemp, snareNumberSamples, snare);
	parseWav(crashTemp, crashNumberSamples, crash);
	parseWav(hihatTemp, hihatNumberSamples, hihat);
	parseWav(hihat2Temp, hihat2NumberSamples, hihat2);
	parseWav(kickTemp, kickNumberSamples, kick);
	parseWav(tomTemp, tomNumberSamples, tom);
	parseWav(tom2Temp, tom2NumberSamples, tom2);

	// audio init
	audioInit(audio_config_dev);
	printf("Ready To Play\n");
	// system is now ready to play
//	init_button_pio();
	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, "Ready To Play");
//	INT8U err = 0;

//	printf("irq register: %d\n", IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE));
//	while(1) {
////		int* myButton = OSQPend(myQueue, 0, &err);
////		printf("button: %d\n", &myButton);
//		//printf("hello\n");
////		printf("irq register: %d\n", IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE));
//		alt_up_character_lcd_init(myLCD);
//		alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
//		alt_up_character_lcd_string(myLCD, "Ready To Play");
//	}

	// currently we use this while loop for demonstration purposes
	// for actual implementation we will use an interrupt routine to handle drum soound playing
	while(1) {
		// currently using magic number due to problems with system.h header
		button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);


		switch (button) {

			case 7 :
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "tom/Crash");

				int k = 40000;
				int kcrash = 0;
				unsigned int tempArr[128];
				while(kcrash <= crashNumberSamples - 128) {
					addWav(tom + k, crash + kcrash, tempArr);
					k += 128;
					kcrash += 128;
					if(k >= tomNumberSamples - 128) {
						k = 0;
					}
					playSound(128, tempArr);
				}
				while(button == 7)button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
				break;

			case 11:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "hihat Decay");

				int kk = 0;
				while(kk <= 60000) {
					kk += 128;
					playSound(128, hihat+kk);
				}
				int j;
				for (j = 0; j < 3; j++) {
					kk = 60000;
					while(kk <= 70000) {
						kk += 128;
						playSound(128, hihat+kk);
					}
				}
				while(k <= hihatNumberSamples - 128) {
					k += 128;
					playSound(128, hihat+k);
				}
				while(button == 11)button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
				break;

			case 13:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "HiHat Amp");

				int scale = 3;
				playSoundAmp(hihatNumberSamples, hihat, scale);
				while(button == 13)button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
				break;

			case 14:
				alt_up_character_lcd_init(myLCD);
				alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
				alt_up_character_lcd_string(myLCD, "Hi-Hat");

				playSound(hihatNumberSamples, hihat);
				while(button == 14)button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
				break;
		}
	}
}

void addWav(unsigned int* wav1, unsigned int* wav2, unsigned int* arr) {
	int k;
	for(k = 0; k < 128; k++) {
		arr[k] = wav1[k] + wav2[k];
	}
}


// takes in wav file and number of samples
// plays given sound once
void playSound(unsigned long NumberSamples, unsigned int* wav) {
	int count = 0;
	int bool = 0;
	while(count <= NumberSamples - 128) {
		int fifospace = alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT);
		if(fifospace >= 128) {
				alt_up_audio_write_fifo(audio_dev, wav + count, 128, ALT_UP_AUDIO_RIGHT);
				alt_up_audio_write_fifo(audio_dev, wav + count, 128, ALT_UP_AUDIO_LEFT);
				count += 128;
				bool = 1;
		}
	}
}


void playSoundAmp(unsigned long NumberSamples, unsigned int* wav, int scale) {
	int count = 0;
	int bool = 0;
	unsigned int temp[128];
	while(count <= NumberSamples - 128) {
		int k;
		for(k = 0; k < 128; k++) {
			temp[k] = scale*wav[k+count];
		}
		int fifospace = alt_up_audio_write_fifo_space(audio_dev, ALT_UP_AUDIO_RIGHT);
		if(fifospace >= 128) {
				alt_up_audio_write_fifo(audio_dev, temp, 128, ALT_UP_AUDIO_RIGHT);
				alt_up_audio_write_fifo(audio_dev, temp, 128, ALT_UP_AUDIO_LEFT);
				count += 128;
				bool = 1;
		}
	}
}
// initializes spi interface for sd card reading
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

// sets our codec up, sets the appropriate registers
// set up fro sampling rate of 44100
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

//static void init_button_pio() {
//	void* edge_capture_ptr = (void*) &edge_capture;
//	/* Enable all 4 button interrupts. */
//	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTONS_BASE, 0xf);
//	/* Reset the edge capture register. */
//	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE, 0x0);
//	/* Register the ISR. */
//	alt_irq_register(BUTTONS_IRQ, edge_capture_ptr, interrupt_isr_buttonPress);
//}


// sdRead will read wav file given its filename and the efsl needed variables
// returns the raw .wav data 
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

// takes in raw .wav data, parses needed data to play on codec
void parseWav(euint8* fileBuffer, unsigned long numberSamples, unsigned int *fileBufL) {

	int i;
	unsigned int temp;
	for(i = 0; i < numberSamples; i++) {
		temp = (unsigned int) 2*((fileBuffer[i*4+44] | (fileBuffer[i*4+45] << 8)));
		fileBufL[i] = temp;
	}
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