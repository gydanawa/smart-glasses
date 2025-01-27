/*
 * camera.h
 *
 * Created: 2/4/2024 3:48:45 PM
 *  Author: nicol
 */ 


#ifndef CAMERA_H_
#define CAMERA_H_
#include <asf.h>
#include <string.h>
//form ilya example
#define CAMERA_TWI					TWI0
#define CAMERA_TWI_ID				ID_TWI0
#define CAMERA_TWI_BAUDRATE			115200
#define CAMERA_TWI_HANDLER			TWI0_Handler
#define CAMERA_TWI_TRQn				TWI0_IRQn
#define CAMERA_TWI_CHAR_LENGTH		US_MR_CHRL_8_BIT
#define CAMERA_TWI_PARITY			US_MR_PAR_NO
#define CAMERA_TWI_STOP_BITS		US_MR_NBSTOP_1_BIT
#define CAMERA_TWI_MODE				US_MR_CHMODE_NORMAL

//define TWI pins
#define PIN_TWI0_TWCK (PIO_PA4A_TWCK0)
#define PIN_TWI0_TWCK_FLAGS (PIO_PERIPH_A | PIO_PULLUP)
#define PIN_TWI0_TWD (PIO_PA3A_TWD0)
#define PIN_TWI0_TWD_FLAGS (PIO_PERIPH_A | PIO_PULLUP)

/** TWI0 Data pins definition */
#define TWI0_DATA_GPIO                 PIO_PA3_IDX
#define TWI0_DATA_FLAGS                (PIO_PERIPH_A | PIO_DEFAULT)
#define TWI0_DATA_MASK                 PIO_PA3
#define TWI0_DATA_PIO                  PIOA
#define TWI0_DATA_ID                   ID_PIOA
#define TWI0_DATA_TYPE                 PIO_PERIPH_A
#define TWI0_DATA_ATTR                 PIO_DEFAULT

/** TWI0 clock pins definition */
#define TWI0_CLK_GPIO                  PIO_PA4_IDX
#define TWI0_CLK_FLAGS                 (PIO_PERIPH_A | PIO_DEFAULT)
#define TWI0_CLK_MASK                  PIO_PA4
#define TWI0_CLK_PIO                   PIOA
#define TWI0_CLK_ID                    ID_PIOA
#define TWI0_CLK_TYPE                  PIO_PERIPH_A
#define TWI0_CLK_ATTR                  PIO_DEFAULT


#define CAMERA_SDA_GPIO (PIO_PA3_IDX)
#define CAMERA_SDA_FLAGS (PIO_PERIPH_A | PIO_PULLUP)

#define CAMERA_SCL_GPIO (PIO_PA4_IDX)
#define CAMERA_SCL_FLAGS (PIO_PERIPH_A | PIO_PULLUP)

#define CAMERA_XCLK_GPIO (PIO_PA17_IDX)
#define CAMERA_XCLK_FLAGS (PIO_PERIPH_B | PIO_DEFAULT)

#define CAMERA_PCLK_GPIO (PIO_PA23_IDX)
#define CAMERA_PCLK_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D0_GPIO (PIO_PA24_IDX)
#define CAMERA_D0_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D1_GPIO (PIO_PA25_IDX)
#define CAMERA_D1_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D2_GPIO (PIO_PA26_IDX)
#define CAMERA_D2_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D3_GPIO (PIO_PA27_IDX)
#define CAMERA_D3_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D4_GPIO (PIO_PA28_IDX)
#define CAMERA_D4_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D5_GPIO (PIO_PA29_IDX)
#define CAMERA_D5_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D6_GPIO (PIO_PA30_IDX)
#define CAMERA_D6_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_D7_GPIO (PIO_PA31_IDX)
#define CAMERA_D7_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

#define CAMERA_CAM_RST_GPIO (PIO_PA20_IDX)
#define CAMERA_CAM_RST_FLAGS (PIO_PERIPH_A | PIO_DEFAULT)

//interrupts

#define CAMERA_VSYNC_GPIO (PIO_PA15_IDX)
#define CAMERA_VSYNC_FLAGS (PIO_PULLUP | PIO_IT_RISE_EDGE)
#define OV2640_VSYNC_MASK          PIO_PA15
#define OV2640_VSYNC_PIO	       PIOA
#define OV2640_VSYNC_ID		       ID_PIOA
#define OV2640_VSYNC_TYPE          PIO_PULLUP
#define OV2640_VSYNC_ATTR		   PIO_IT_RISE_EDGE

#define CAMERA_HREF_GPIO (PIO_PA16_IDX)
#define CAMERA_HREF_FLAGS (PIO_PULLUP | PIO_IT_RISE_EDGE)
#define OV2640_HREF_PIN_MSK				PIO_PA16
#define OV2640_HREF_PIO					PIOA
#define OV2640_HREF_ID					ID_PIOA 
#define OV2640_HREF_TYPE				PIO_PULLUP
#define OV2640_HREF_ATTR				PIO_IT_RISE_EDGE

#define OV2640_DATA_BUS_PIO            PIOA
#define OV2640_DATA_BUS_ID             ID_PIOA




void vsync_handler(uint32_t ul_id, uint32_t ul_mask);
void init_vsync_interrupts(void);
void configure_twi(void);
void pio_capture_init(Pio *p_pio, uint32_t ul_id);
static uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t *uc_buf,uint32_t ul_size);
void init_camera(void);
void configure_camera(void);
uint8_t start_capture(void);
uint8_t find_image_len(void);

//global variable for image length, to be used in wifi.c
extern uint32_t image_len;

#define buffer_len 100000
extern uint8_t im_buf[buffer_len];

extern uint32_t SOI_index;
extern uint32_t EOI_index;

#endif /* CAMERA_H_ */