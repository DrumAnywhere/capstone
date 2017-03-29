/* Created By Kelvin Liang and Justin Ferris 
   Drum Anywhere G4
   Date: Feb 22, 2017
   Description: buttons.c contains code for
   				button operation on DE2
   				buttons are used as a substitute for 
   				IMU's. We use this to test our synthesizer.
*/

#include "DrumAnyWhere.h"


void init_button_pio() {
	void* edge_capture_ptr = (void*) &edge_capture;
	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTONS_BASE, 0xf);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE, 0x0);
	/* Register the ISR. */
	alt_irq_register(BUTTONS_IRQ, edge_capture_ptr, interrupt_isr_buttonPress);
}

void DE2_conn_init() {
	/* Enable interrupt. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(DE2_POLL_BASE, 0x08);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(DE2_POLL_BASE, 0x08);
	/* Register the ISR. */
	alt_irq_register(DE2_POLL_IRQ, NULL , interrupt_isr_de2Poll);
}

void interrupt_isr_de2Poll (void *context, alt_u32 id) {
	int drum = IORD_ALTERA_AVALON_PIO_DATA(DE2_POLL_BASE);
	drum = drum ^ 0x08;
	setDrum(drum);

	char str[5];
	sprintf(str, "%d", drum);

	alt_up_character_lcd_init(myLCD);
	alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
	alt_up_character_lcd_string(myLCD, str);

	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(DE2_POLL_BASE, 0x08);

}

void interrupt_isr_buttonPress(void *context, alt_u32 id) {

	volatile int* edge_capture_ptr = (volatile int*) context;
	*edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE);

	button = IORD_ALTERA_AVALON_PIO_DATA(BUTTONS_BASE);
	alt_up_audio_enable_write_interrupt(audio_dev);

	if((button & 0x8) == 0) {
		alt_up_character_lcd_init(myLCD);
		alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
		alt_up_character_lcd_string(myLCD, "kick");
		setDrum(kickConst);
	}
	if((button & 0x4) == 0) {
		alt_up_character_lcd_init(myLCD);
		alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
		alt_up_character_lcd_string(myLCD, "tom");
		setDrum(tomConst);
	}
	if((button & 0x2) == 0) {
		alt_up_character_lcd_init(myLCD);
		alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
		alt_up_character_lcd_string(myLCD, "crash");
		setDrum(crashConst);
	}
	if((button & 0x1) == 0) {
		alt_up_character_lcd_init(myLCD);
		alt_up_character_lcd_set_cursor_pos(myLCD, 0, 1);
		alt_up_character_lcd_string(myLCD, "snare");
		setDrum(snareConst);
	}

	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE, 0x01);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTONS_BASE);
}

void setDrum(int drum) {
	if(isPlaying[drum] == 0) {
		isPlaying[drum] = 1;
	} else {
		drums[drum]->index = 0;
	}
}