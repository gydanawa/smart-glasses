/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "wifi.h"
#include "camera.h"
#include "timer_interface.h"
#include "ov2640.h"

bool test_pass = false;

int main (void)
{
	//system clock and board initializations
	board_init();
	sysclk_init();
	wdt_disable(WDT);
	
	//configure and start timer (look at "timer_interface" functions)
	configure_tc(); //this calls tc_start(TC0, 0); to start the timer
	
	//configure wifi usart and spi, as well as command complete and provision pins interrupts
	configure_usart_wifi();
	configure_spi();
	spi_peripheral_initialize();
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();
	wifi_provision_flag = false;

	usart_write_line(WIFI_USART, "set spi_baud 100000\r\n"); //change to write_wifi_command?
	//delay_ms(1000);
	
	//configure indicators and "command complete", "network" and "clients" gpios through UART interface of ESP32
	//a) configure the top LED as the wlan output //IO25
	write_wifi_command("set wlan_gpio 25\r\n", 3);
	//b) configure the middle LED as the websockets output //IO26
	write_wifi_command("set websocket_gpio 26\r\n", 3);
	//c) configure the bottom LED as the AP output //IO27
	write_wifi_command("set ap_gpio 27\r\n", 3);
	//configure the command complete, network, and clients gpios (control signals?)
	write_wifi_command("set comm_gpio 21\r\n", 3);
	write_wifi_command("set net_gpio 22\r\n", 3); 
	write_wifi_command("set clients_gpio 23\r\n", 3);

	ioport_set_pin_dir(WIFI_RST_PIN, IOPORT_DIR_OUTPUT);
	
	delay_ms(500);
	
	//initialize and configure the camera
	init_camera(); //clk configured here
	configure_camera();
	//delay_ms(100);
	
	//start_capture();
	
	//reset the wifi and wait for it to connect to a network
	//reset the wifi module
		//pull pin low, wait 100ms, then set high
	ioport_set_pin_level(WIFI_RST_PIN, IOPORT_PIN_LEVEL_LOW);
	delay_ms(200);
	ioport_set_pin_level(WIFI_RST_PIN, IOPORT_PIN_LEVEL_HIGH);
	
	
	//wait for wifi to connect to a network
	while (ioport_get_pin_level(WIFI_NETWORK_GPIO) == false) {
		//listen for provision pin
		if (wifi_provision_flag) {
			//put wifi chip into provision mode
			write_wifi_command("provision\r\n", 3);
			//clear the flag
			wifi_provision_flag = false;
		}
	}
	
	
	//send "test" to the WiFi module and wait for response of "SUCCESS"
	delay_ms(5000);
	write_wifi_command("test\r\n", 10);
	delay_ms(1000);
	
	while (!processing_done_flag) {
		//if you do not receive a response, wait 10 seconds, reset the Wifi module, and try again
		delay_ms(10000); //wait 10s
		ioport_set_pin_level(WIFI_RST_PIN, IOPORT_PIN_LEVEL_LOW);
		delay_ms(100);
		ioport_set_pin_level(WIFI_RST_PIN, IOPORT_PIN_LEVEL_HIGH);
		delay_ms(2000);
		write_wifi_command("test\r\n", 10);
		delay_ms(100);
	}
	
	//main loop begins
	while (1) {
		//check for provision request and act accordingly
		if (wifi_provision_flag) {
			write_wifi_command("provision\r\n", 3);
			wifi_provision_flag = false;
		}
		//if network is available and clients are connected, take picture
		if (ioport_get_pin_level(WIFI_NETWORK_GPIO) && ioport_get_pin_level(WIFI_CLIENTS_GPIO)) {//format string first, then use it && (ioport_get_pin_level(WIFI_CLIENTS_GPIO)) {
				//take a picture and check if it was successful
				if (start_capture()) { //start_capture returns 1 for success

\					write_image_to_web();
				}
		}			
	}
}
