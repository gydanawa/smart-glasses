/*
 * wifi.c
 *
 * Created: 2/8/2024 5:34:38 AM
 *  Author: laptop

 */ 

#include <asf.h>
#include "wifi.h"

// WiFi variable initializations.
volatile uint32_t transfer_index = 0;
volatile uint32_t transfer_len = 0;

//flag to indicate status of processing
bool processing_done_flag = false;


//WiFi function declarations
//Handler for incoming data from the WiFi. Should call process incoming byte wifi when a new byte arrives.
void wifi_usart_handler(void) {
	uint32_t ul_status;
	
	/* Read USART Status. */
	ul_status = usart_get_status(WIFI_USART);

	/* Check if receive buffer is full */
	if (ul_status & US_CSR_RXBUFF) {
		usart_read(WIFI_USART, &received_byte_wifi);
		new_rx_wifi = true;
		process_incoming_byte_wifi((uint8_t)received_byte_wifi);
	}
}

//Handler for “command complete” rising-edge interrupt from ESP32. When this is triggered, it is time to process
//the response of the ESP32.
void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask) {
	unused(ul_id);
	unused(ul_mask);
	
	wifi_comm_success = true;
	process_data_wifi();
	//throw out entire buffer and restart input position 
	for (int jj=0; jj<MAX_INPUT_WIFI; jj++) {
		input_line_wifi[jj] = 0;
	}
	input_pos_wifi = 0;
}

//Stores every incoming byte (in byte) from the ESP32 in a buffer.
void process_incoming_byte_wifi(uint8_t in_byte) {
	input_line_wifi[input_pos_wifi++] = in_byte;
}

//Processes the response of the ESP32, which should be stored in
//the buffer filled by process incoming byte wifi. This processing should be looking for certain
//responses that the ESP32 should give, such as “SUCCESS” when “test” is sent to it.
void process_data_wifi (void) {
	if (strstr(input_line_wifi, "SUCCESS")) { //check if uart connection is available
		processing_done_flag = true; //this flag is used in main()
	}
}

//Handler for button to initiate provisioning mode of the ESP32. Should set a flag indicating a request to 
//initiate provisioning mode.
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask) {
	unused(ul_id);
	unused(ul_mask);
	
	wifi_provision_flag = true; //flag to initiate provisioning
}

//Handler for peripheral mode interrupts on SPI bus. When the
//ESP32 SPI controller requests data, this interrupt should send one byte of the image at a time.
//ILYA NOTES:
//1. need to set length of transfer to length of image
//2. set transfer index to appropriate location of image buffer where image starts
//3. when receive clks from wifi chip, transfer 1 byte at a time from buffer to wifi chip
void wifi_spi_handler(void) {
	static uint16_t data;
	uint8_t uc_pcs;

	if (spi_read_status(SPI) & SPI_SR_RDRF) {
		spi_read(SPI, &data, &uc_pcs);

		if (transfer_len--) { //but this is image length
			//uint8_t my_byte = im_buf[transfer_index];
			spi_write(SPI, im_buf[transfer_index++], 0, 0);  //need to draw values from image buffer //automatically increments the transfer_index
			
			//gets called each clock cycle
			//prep transfer length same length as image
			//start transferring at the first index
		}
	}
}

//Configuration of USART port used to communicate with the ESP32.
void configure_usart_wifi(void) {
	gpio_configure_pin(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
	gpio_configure_pin(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);
	
	
	const sam_usart_opt_t usart_console_settings = {
		WIFI_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		/* This field is only used in IrDA mode. */
		0
	};
	
	/* Enable the peripheral clock in the PMC. */
	sysclk_enable_peripheral_clock(WIFI_USART_ID);

	/* Configure USART in serial mode. */
	usart_init_rs232(WIFI_USART, &usart_console_settings, sysclk_get_peripheral_hz());

	/* Disable all the interrupts. */
	usart_disable_interrupt(WIFI_USART, ALL_INTERRUPT_MASK);

	/* Enable the receiver and transmitter. */
	usart_enable_tx(WIFI_USART);
	usart_enable_rx(WIFI_USART);

	/* Configure and enable interrupt of USART. */
	NVIC_EnableIRQ(WIFI_USART_IRQn);
	
	usart_enable_interrupt(WIFI_USART, US_IER_RXRDY); 
	//every time receive byte, flip bit in register, 
	//so we know there's something to do w/ new data, interrupt gets triggered by this
}

//Configuration of “command complete” rising-edge interrupt.
void configure_wifi_comm_pin(void) {
	/* Configure PIO clock. */
	pmc_enable_periph_clk(WIFI_COMM_ID);

	/* Initialize PIO interrupt handler, see PIO definition in conf_board.h -- look for rising edge
	**/
	pio_handler_set(WIFI_COMM_PIO, WIFI_COMM_ID, WIFI_COMM_PIN_MSK,
			WIFI_COMM_ATTR, wifi_command_response_handler);

	/* Enable PIO controller IRQs. */
	NVIC_EnableIRQ((IRQn_Type)WIFI_COMM_ID);

	/* Enable PIO interrupt lines. */
	pio_enable_interrupt(WIFI_COMM_PIO, WIFI_COMM_PIN_MSK);
}

//Configuration of button interrupt to initiate provisioning mode.
void configure_wifi_provision_pin(void) {
	/* Configure PIO clock. */
	pmc_enable_periph_clk(WIFI_PROV_ID);

	/* Initialize PIO interrupt handler, see PIO definition in conf_board.h -- look for rising edge
	**/
	pio_handler_set(WIFI_PROV_PIO, WIFI_PROV_ID, WIFI_PROV_PIN_MSK,
			WIFI_PROV_ATTR, wifi_provision_handler);

	/* Enable PIO controller IRQs. */
	NVIC_EnableIRQ((IRQn_Type)WIFI_PROV_ID);

	/* Enable PIO interrupt lines. */
	pio_enable_interrupt(WIFI_PROV_PIO, WIFI_PROV_PIN_MSK);
}

//Configuration of SPI port and interrupts used to send images to the ESP32.
void configure_spi(void) {
	gpio_configure_pin(SPI_MISO_GPIO, SPI_MISO_FLAGS);
	gpio_configure_pin(SPI_MOSI_GPIO, SPI_MOSI_FLAGS);
	gpio_configure_pin(SPI_SPCK_GPIO, SPI_SPCK_FLAGS);
	gpio_configure_pin(SPI_NPCS0_GPIO, SPI_NPCS0_FLAGS);
	//gpio_configure_pin(WIFI_COMM_COMPLETE_GPIO, CAMERA_SDA_FLAGS);
	
	/* Configure SPI interrupts for slave only. */
	NVIC_DisableIRQ(SPI_IRQn);
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_SetPriority(SPI_IRQn, 0);
	NVIC_EnableIRQ(SPI_IRQn);
}

/*Initialize the SPI port as a peripheral (slave) device. Note:
The embedded industry is trying to phase out the “master/slave” terminology that is widespread.
In the SPI example project, this function is called spi slave initialize, just so you know where to
look. For more details about this topic in general, you can start here and here.*/
void spi_peripheral_initialize(void) {
	spi_enable_clock(SPI);
	spi_disable(SPI);
	spi_reset(SPI);
	spi_set_slave_mode(SPI);
	spi_disable_mode_fault_detect(SPI);
	spi_set_peripheral_chip_select_value(SPI, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI, SPI_CHIP_SEL, SPI_CSR_BITS_8_BIT);
	spi_enable_interrupt(SPI, SPI_IER_RDRF);
	spi_enable(SPI);
}


//Set necessary parameters to prepare for SPI transfer. Note:
//Same as above. In the SPI example project, this function corresponds to the spi slave transfer
//function.
void prepare_spi_transfer(void) {
	transfer_len = image_len; //from find image length
	transfer_index = SOI_index; //initialize to image offset
}

//Writes a command (comm) to the ESP32,
//and waits either for an acknowledgment (via the “command complete” pin) or a timeout. The
//timeout can be created by setting the global variable counts to zero, which will automatically
//increment every second, and waiting while counts < cnt.
void write_wifi_command(char* comm, uint8_t cnt) {
	//write command to ESP32
	usart_write_line(WIFI_USART, comm);
	
	//timeout: reset counts to zero
	counts = 0;
	
	//wifi_comm_success = false;
	while (counts < cnt) {
		//wait for acknowledgment
		if (wifi_comm_success) { //if receive response faster, return immediately
			wifi_comm_success = false;
			return; 
		}
	}
}

/*Writes an image from the SAM4S8B to the ESP32. If the
length of the image is zero (i.e. the image is not valid), return. Otherwise, follow this protocol
(illustrated in Appendix C):
1. Configure the SPI interface to be ready for a transfer by setting its parameters appropriately.
2. Issue the command “image transfer xxxx”, where xxxx is replaced by the length of the
image you want to transfer.
3. The ESP32 will then set the “command complete” pin low and begin transferring the image
over SPI.
4. After the image is done sending, the ESP32 will set the “command complete” pin high. The
MCU should sense this and then move on */
void write_image_to_web(void) {
	//check if image length is nonzero
	if (image_len) {
		//1. configure SPI interface to be ready by setting parameters
		prepare_spi_transfer();
		//2. issue command "image transfer" //image length is a global (property of camera)
		//format string first, then use it
		char comm_str[80];
		sprintf(comm_str, "image_transfer %d\r\n", image_len);
		
		write_wifi_command(comm_str, 3); //length of image we want to transfer?
	}
}

