/*
 * camera.c
 *
 * Created: 2/4/2024 3:48:03 PM
 *  Author: nicole
 */ 

#include <asf.h>
#include "camera.h"
#include "ov2640.h"


//from OV7740 example project
/* TWI clock frequency in Hz (400KHz) */
#define TWI_CLK     (400000UL)

static volatile uint32_t g_ul_vsync_flag = false;

/* Pointer to the image data destination buffer */
uint8_t *g_p_uc_cap_dest_buf;
//use preallocated array instead of pointer
/* Rows size of capturing picture */
uint16_t g_us_cap_rows = IMAGE_HEIGHT;
uint16_t g_us_cap_line = (IMAGE_WIDTH * 2);

//size of image
uint32_t image_len;
uint8_t im_buf[buffer_len];

//buffer indices
uint32_t SOI_index;
uint32_t EOI_index;

//watch variables
uint8_t start_byte;
uint8_t end_byte;


// Handler for rising-edge of VSYNC signal. Should set a flag 
//indicating a rising edge of VSYNC
void vsync_handler(uint32_t ul_id, uint32_t ul_mask){
	//from OV7740 example project
	unused(ul_id);
	unused(ul_mask);

	g_ul_vsync_flag = true;
}
//configuration of VSYNC interrupt.
void init_vsync_interrupts(void){
	//from OV7740 example project
	//changed to OV2640
	pio_handler_set(OV2640_VSYNC_PIO, OV2640_VSYNC_ID, OV2640_VSYNC_MASK, OV2640_VSYNC_TYPE, vsync_handler);

	/* Enable PIO controller IRQs */
	NVIC_EnableIRQ((IRQn_Type)OV2640_VSYNC_ID);
}
//Configuration of TWI (two wire interface).
void configure_twi(void){
	gpio_configure_pin(PIN_TWI0_TWCK, PIN_TWI0_TWCK_FLAGS); //ask other ppl
	gpio_configure_pin(PIN_TWI0_TWD, PIN_TWI0_TWD_FLAGS);
	
	twi_options_t opt;

	/* Enable TWI peripheral */
	pmc_enable_periph_clk(ID_BOARD_TWI);

	/* Init TWI peripheral */
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed      = TWI_CLK;
	twi_master_init(BOARD_TWI, &opt);

	/* Configure TWI interrupts */
	NVIC_DisableIRQ(BOARD_TWI_IRQn);
	NVIC_ClearPendingIRQ(BOARD_TWI_IRQn);
	NVIC_SetPriority(BOARD_TWI_IRQn, 0);
	NVIC_EnableIRQ(BOARD_TWI_IRQn);
	
}
//Uses parallel capture and PDC to store image in buffer.
void pio_capture_init(Pio *p_pio, uint32_t ul_id){
	//from
	/* Enable peripheral clock */
	pmc_enable_periph_clk(ul_id);

	/* Disable pio capture */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_PCEN);

	/* Disable rxbuff interrupt */
	p_pio->PIO_PCIDR |= PIO_PCIDR_RXBUFF;

	/* 32bit width*/
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_DSIZE_Msk);
	p_pio->PIO_PCMR |= PIO_PCMR_DSIZE_WORD;

	/* Only HSYNC and VSYNC enabled */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_ALWYS);
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_HALFS);

	/*#if !defined(DEFAULT_MODE_COLORED)
	/* Samples only data with even index */
	//p_pio->PIO_PCMR |= PIO_PCMR_HALFS;
	//p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_FRSTS);
	//#endif
}

static uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t* uc_buf,
uint32_t ul_size) //the second argument should not be a pointer either?
{
	/* Check if the first PDC bank is free */
	if ((p_pio->PIO_RCR == 0) && (p_pio->PIO_RNCR == 0)) {
		p_pio->PIO_RPR = (uint32_t)uc_buf;
		p_pio->PIO_RCR = ul_size;
		p_pio->PIO_PTCR = PIO_PTCR_RXTEN;
		return 1;
		} else if (p_pio->PIO_RNCR == 0) {
		p_pio->PIO_RNPR = (uint32_t)uc_buf;
		p_pio->PIO_RNCR = ul_size;
		return 1;
		} else {
		return 0;
	}
}
// Configuration of camera pins, camera clock (XCLK), and calling the configure twi function.
void init_camera(void){
	
	init_vsync_interrupts();
	delay_ms(100);
	pio_capture_init(OV2640_DATA_BUS_PIO, OV2640_DATA_BUS_ID);
	delay_ms(100);
	//configure camera pins - check these
	gpio_configure_pin(CAMERA_PCLK_GPIO,CAMERA_PCLK_FLAGS); //PCLK
	gpio_configure_pin(CAMERA_D0_GPIO,CAMERA_D0_FLAGS); //DATABUS [7:0]
	gpio_configure_pin(CAMERA_D1_GPIO,CAMERA_D1_FLAGS);
	gpio_configure_pin(CAMERA_D2_GPIO,CAMERA_D2_FLAGS);
	gpio_configure_pin(CAMERA_D3_GPIO,CAMERA_D3_FLAGS);
	gpio_configure_pin(CAMERA_D4_GPIO,CAMERA_D4_FLAGS);
	gpio_configure_pin(CAMERA_D5_GPIO,CAMERA_D5_FLAGS);
	gpio_configure_pin(CAMERA_D6_GPIO,CAMERA_D6_FLAGS);
	gpio_configure_pin(CAMERA_D7_GPIO,CAMERA_D7_FLAGS);
	
	//interrupts - these are actually initialized elsewhere because they are interrupts?
	gpio_configure_pin(CAMERA_VSYNC_GPIO,CAMERA_VSYNC_FLAGS); //VSYNC - vertical
	gpio_configure_pin(CAMERA_HREF_GPIO,CAMERA_HREF_FLAGS); //HREF - horizontal read the datasheet
	
	gpio_configure_pin(CAMERA_SDA_GPIO, CAMERA_SDA_FLAGS);
	gpio_configure_pin(CAMERA_SCL_GPIO, CAMERA_SCL_FLAGS);
	//output
	//gpio_configure_pin(CAMERA_CAM_RST_GPIO,CAMERA_CAM_RST_FLAGS); //CAM_RESET 
	
	//configure camera clock (XCLK and PCLK are not normal GPIOs- timer peripherals)
	gpio_configure_pin(CAMERA_XCLK_GPIO,CAMERA_XCLK_FLAGS);
	/* Init PCK0 to work at 24 Mhz */
	/* 96/4=24 Mhz */
	
	pmc_enable_pllbck(7,0x1,1); //starts the clock
	
	PMC->PMC_PCK[1] = (PMC_PCK_PRES_CLK_4 | PMC_PCK_CSS_PLLB_CLK); //reads pllb as its source clock
	PMC->PMC_SCER = PMC_SCER_PCK1;
	while (!(PMC->PMC_SCSR & PMC_SCSR_PCK1)) {
	}
	
	configure_twi();
}

/*Configuration of OV2640 registers for desired operation. To
properly initialize the camera for JPEG (at 320 × 240 resolution), use the following commands,
instead of the corresponding one in the sample project):
ov_configure(BOARD_TWI, JPEG_INIT);
ov_configure(BOARD_TWI, YUV422);
ov_configure(BOARD_TWI, JPEG);
ov_configure(BOARD_TWI, JPEG_320x240);*/
void configure_camera(void){
	delay_ms(500);
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);
}

uint8_t start_capture(void){ //in debug mode, look at memory and see contents of buffer

	/* Enable vsync interrupt*/
	pio_enable_interrupt(OV2640_VSYNC_PIO, OV2640_VSYNC_MASK);

	/* Capture acquisition will start on rising edge of Vsync signal.
	 * So wait g_vsync_flag = 1 before start process
	 */
	while (!g_ul_vsync_flag) {
	} //will start because flag is true

	/* Disable vsync interrupt*/
	pio_disable_interrupt(OV2640_VSYNC_PIO, OV2640_VSYNC_MASK);

	/* Enable pio capture*/
	pio_capture_enable(OV2640_DATA_BUS_PIO);

	/* Capture data and send it to external SRAM memory thanks to PDC
	 * feature */
	pio_capture_to_buffer(OV2640_DATA_BUS_PIO, im_buf,
			(buffer_len >> 2));

	/* Wait end of capture*/
	while (!((OV2640_DATA_BUS_PIO->PIO_PCISR & PIO_PCIMR_RXBUFF) ==
			PIO_PCIMR_RXBUFF)) {
	}


	/* Disable pio capture*/
	pio_capture_disable(OV2640_DATA_BUS_PIO);

	/* Reset vsync flag*/
	g_ul_vsync_flag = false;
	
	//get an image length
	if (find_image_len()) { //return 1 if get a nonzero image length
		return 1;
	}
	else {
		return 0;
	}
}


/* Pointer to the image data destination buffer */
//uint8_t *g_p_uc_cap_dest_buf;
/* Rows size of capturing picture */
//uint16_t g_us_cap_rows = IMAGE_HEIGHT;
//uint16_t g_us_cap_line = (IMAGE_WIDTH * 2);


//return image length based on JPEG protocol
//just numbers between 0-255
//see lecture 20240201 1:03:00 - 1:08:00
//look for this in the memory map:
//SOI: ff d8 ff e0 (yoya JFIF)
//EOI: ff d9
uint8_t find_image_len(void){
	//flags and indices for SOI and EOI markers
	bool EOI_found = false;
	bool SOI_found = false;	
	
	//search through buffer
	for (uint32_t i=0; i < buffer_len; i++){ 
		if ((im_buf[i] == 0xff) && (im_buf[i+1] == 0xd8)) {
			SOI_found = true;
			SOI_index = i;   
			start_byte =  im_buf[i] ;                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
		}
		if (im_buf[i] == 0xff && (im_buf[i+1] == 0xd9)) { //EOI marker = "ff d9"
			EOI_found = true;
			EOI_index = i+1; //increment to the end of the marker
			end_byte = im_buf[i+1];
		}
		if (SOI_found && EOI_found) {
			break; //need to find both and then break
		}
		else {
			SOI_found = false;
			EOI_found = false;
		}
	}
	//if success (able to find end of image (EOI) and start of image (SOI) markers, with SOI preceeding EOI
	//return 1
	if (SOI_index < EOI_index) {
		image_len = EOI_index +1 - SOI_index;
		return 1;
	}
	else {
		return 0;
	}
}