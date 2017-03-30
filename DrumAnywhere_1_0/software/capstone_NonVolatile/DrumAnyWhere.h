#ifndef DrumAnyWhere_H_
#define DrumAnyWHere_H_

#include <stdio.h>
#include <system.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

// sd
euint8* sdRead(char* fileName, EmbeddedFileSystem *efsl, File *readFile);
void parseWav(euint8* fileBuffer, unsigned long numberSamples, unsigned int* fileBufL);
void sdInit(EmbeddedFileSystem *efsl, File *readFile);

// audio 
void audioInit(alt_up_av_config_dev * audio_config_dev);
void audio_isr(void* context, alt_u32 id);
void synthesize();

// buttons
void init_button_pio();
void DE2_conn_init();
void interrupt_isr_buttonPress(void *context, alt_u32 id);
void interrupt_isr_de2Poll (void *context, alt_u32 id);
void setDrum(int drum);

// IMU
void pollIMU(void* pdata);

typedef struct {
	unsigned int* waveform;
	int index;
	int scale;
	int numberOfSamples;
}Drum;

#define numDrums 7
Drum* drums[numDrums];

#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK1_PRIORITY      2
#define TASK2_PRIORITY      1


// globals 
alt_up_character_lcd_dev* myLCD;
int button;
alt_up_audio_dev * audio_dev;

#define SAMPLE_SIZE 16

// sample sizes for sounds
#define snareNumberSamples 7235
#define crashNumberSamples 63342
#define hihatNumberSamples 98218
#define hihat2NumberSamples 6313
#define kickNumberSamples 5000 // check this value, I reduced it because the original number is wrong
#define tomNumberSamples 57006
#define tom2NumberSamples 37500 // check this value, I reduced it because the original number is wrong

#define snareSize 29102
#define crashSize 253412
#define hihatSize 392916
#define hihat2Size 25296
#define kickSize 89054
#define tomSize 228186
#define tom2Size 180580

#define snareConst 0
#define crashConst 5
#define hihatConst 3
#define hihat2Const 4
#define kickConst 6
#define tomConst 1
#define tom2Const 2

#define DE2_mask 0x08


// arrays to hold waveforms
unsigned int snare[snareNumberSamples];
unsigned int crash[crashNumberSamples];
unsigned int hihat[hihatNumberSamples];
unsigned int hihat2[hihat2NumberSamples];
unsigned int kick[kickNumberSamples];
unsigned int tom[tomNumberSamples];
unsigned int tom2[tom2NumberSamples];

int isPlaying[numDrums];
unsigned int nextToPlay[SAMPLE_SIZE];

int sem;
OS_EVENT* semaphore;
volatile int edge_capture;

#endif
