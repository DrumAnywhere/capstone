/* Created By Kelvin Liang and Justin Ferris 
   Drum Anywhere G4
   Date: Feb 22, 2017
   Description: sd.c contains code for the spi interfacing for our project
   				this code handles initilizing the efsl libraries and
   				for communicating between the Altera DE2 and our SD card
*/

#include "DrumAnyWhere.h"

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
		temp = (int) ((fileBuffer[i*4+44] | (fileBuffer[i*4+45] << 8)));
		if(temp > 32768) {
			temp = 65535 - temp;
			temp /= 2;
			temp = 65535 - temp;
			fileBufL[i] = temp;
		} else {
			fileBufL[i] = temp/2;
		}
	}
}
