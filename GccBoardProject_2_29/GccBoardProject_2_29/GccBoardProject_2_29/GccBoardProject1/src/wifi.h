/*
 * wifi.h
 *
 * Created: 2/8/2024 5:35:01 AM
 *  Author: laptop
 WiFi control pin definitions, WiFi UART parameters and pin definitions, WiFi SPI parameters
 and pin definitions, WiFi function and variable declarations.
 
 */ 


#ifndef WIFI_H_
#define WIFI_H_

#include <asf.h>
#include <string.h>
#include "timer_interface.h"
#include "camera.h"

#define WIFI_USART_ID             ID_USART0
#define WIFI_USART                USART0
#define WIFI_USART_BAUDRATE       115200
#define wifi_usart_handler        USART0_Handler //is this supported to be a different handler?
#define WIFI_USART_IRQn           USART0_IRQn

//need to define the spi here
//wifi_spi_handler
#define wifi_spi_handler		SPI_Handler

#define ALL_INTERRUPT_MASK			0xffffffff

//comm pin
#define WIFI_COMM_PIN				PIO_PB3_IDX
#define WIFI_COMM_PIN_MSK			PIO_PB3//PIO_PB10 
#define WIFI_COMM_PIO				PIOB
#define WIFI_COMM_ID				ID_PIOB
#define WIFI_COMM_ATTR				PIO_IT_RISE_EDGE

//wifi reset pin
#define WIFI_RST_PIN				PIO_PA0_IDX
/*#define WIFI_RST_PIN_MSK			PIO_PA0_IDX
#define WIFI_RST_PIO				PIOA
#define WIFI_RST_ID					ID_PIOA
#define WIFI_RST_ATTR				PIO_IT_RISE_EDGE*/

//WIFI CONTROL PIN DEFINITIONS
//command complete
#define WIFI_COMM_COMPLETE_GPIO  (PIO_PB3_IDX) //(PIO_PB3_IDX)
#define WIFI_COMM_COMPLETE_FLAGS (PIO_PERIPH_B | PIO_DEFAULT) //(PIO_PERIPH_B | PIO_DEFAULT)

//network
#define WIFI_NETWORK_GPIO  (PIO_PB2_IDX) //(PIO_PB2_IDX)
#define WIFI_NETWORK_FLAGS (PIO_PERIPH_B | PIO_DEFAULT)

//clients
#define WIFI_CLIENTS_GPIO  (PIO_PB0_IDX) //(PIO_PB0_IDX)
#define WIFI_CLIENTS_FLAGS (PIO_PERIPH_B | PIO_DEFAULT)

//for wifi provisioning mode
#define WIFI_PROV_PIN				PIO_PA1_IDX
#define WIFI_PROV_PIN_MSK			PIO_PA1
#define WIFI_PROV_PIO				PIOA
#define WIFI_PROV_ID				ID_PIOA
#define WIFI_PROV_ATTR				PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE


#define MAX_INPUT_WIFI				1000
volatile char input_line_wifi[MAX_INPUT_WIFI];
volatile uint32_t received_byte_wifi;
volatile bool new_rx_wifi;
volatile unsigned int input_pos_wifi;
volatile bool wifi_comm_success;

volatile bool wifi_provision_flag;
extern bool processing_done_flag;

 

/** USART0 pin RX */
#define PIN_USART0_RXD    {PIO_PA5A_RXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_RXD_IDX        (PIO_PA5_IDX)
#define PIN_USART0_RXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
/** USART0 pin TX */
#define PIN_USART0_TXD    {PIO_PA6A_TXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_TXD_IDX        (PIO_PA6_IDX)
#define PIN_USART0_TXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
/** USART0 pin CTS */
#define PIN_USART0_CTS    {PIO_PA8A_CTS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_CTS_IDX        (PIO_PA8_IDX)
#define PIN_USART0_CTS_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
/** USART0 pin RTS */
#define PIN_USART0_RTS    {PIO_PA7A_RTS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_RTS_IDX        (PIO_PA7_IDX)
#define PIN_USART0_RTS_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
/** USART0 pin SCK */
#define PIN_USART0_SCK    {PIO_PA2B_SCK0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_USART0_SCK_IDX        (PIO_PA2_IDX)
#define PIN_USART0_SCK_FLAGS      (PIO_PERIPH_B | PIO_DEFAULT)


/** SPI MISO pin definition. */
#define SPI_MISO_GPIO  (PIO_PA12_IDX)
#define SPI_MISO_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)
/** SPI MOSI pin definition. */
#define SPI_MOSI_GPIO  (PIO_PA13_IDX)
#define SPI_MOSI_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)
/** SPI SPCK pin definition. */
#define SPI_SPCK_GPIO  (PIO_PA14_IDX)
#define SPI_SPCK_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)
/** SPI chip select 0 pin definition. (Only one configuration is possible) */
#define SPI_NPCS0_GPIO            (PIO_PA11_IDX)
#define SPI_NPCS0_FLAGS           (PIO_PERIPH_A | PIO_DEFAULT)

/* Chip select. */
#define SPI_CHIP_SEL 0
#define SPI_CHIP_PCS spi_get_pcs(SPI_CHIP_SEL)

/* Clock polarity. */
#define SPI_CLK_POLARITY 0

/* Clock phase. */
#define SPI_CLK_PHASE 0

#define SPI_DLYBS 0x40

#define SPI_DLYBCT 0x10
//WiFi variable declarations

//WiFi function declarations
//Handler for incoming data from the WiFi. Should call process incoming byte wifi when a new byte arrives.
//void wifi_usart_handler(void); DO NOT REDEFINE THIS, NEED TO ALIAS TO THEIRS

//Stores every incoming byte (in byte) from the ESP32 in a buffer.
void process_incoming_byte_wifi(uint8_t in_byte);

//Handler for “command complete” rising-edge interrupt from ESP32. When this is triggered, it is time to process
//the response of the ESP32.
void wifi_command_response_handler(uint32_t ul_id, uint32_t u_mask);

//Processes the response of the ESP32, which should be stored in
//the buffer filled by process incoming byte wifi. This processing should be looking for certain
//responses that the ESP32 should give, such as “SUCCESS” when “test” is sent to it.
void process_data_wifi (void);

//Handler for button to initiate provisioning mode of the ESP32. Should set a flag indicating a request to
//initiate provisioning mode.
void wifi_provision_handler(uint32_t ul_id, uint32_t u_mask);

//Handler for peripheral mode interrupts on SPI bus. When the
//ESP32 SPI controller requests data, this interrupt should send one byte of the image at a time.
//void wifi_spi_handler(void);

//Configuration of USART port used to communicate with the ESP32.
void configure_usart_wifi(void);

//Configuration of “command complete” rising-edge interrupt.
void configure_wifi_comm_pin(void);

//Configuration of button interrupt to initiate provisioning mode.
void configure_wifi_provision_pin(void);

//Configuration of SPI port and interrupts used to send images to the ESP32.
void configure_spi(void);

/*Initialize the SPI port as a peripheral (slave) device. Note:
The embedded industry is trying to phase out the “master/slave” terminology that is widespread.
In the SPI example project, this function is called spi slave initialize, just so you know where to
look. For more details about this topic in general, you can start here and here.*/
void spi_peripheral_initialize(void);


//Set necessary parameters to prepare for SPI transfer. Note:
//Same as above. In the SPI example project, this function corresponds to the spi slave transfer
//function.
void prepare_spi_transfer(void);

//Writes a command (comm) to the ESP32,
//and waits either for an acknowledgment (via the “command complete” pin) or a timeout. The
//timeout can be created by setting the global variable counts to zero, which will automatically
//increment every second, and waiting while counts < cnt.
void write_wifi_command(char* comm, uint8_t cnt);

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
void write_image_to_web(void);



#endif /* WIFI_H_ */