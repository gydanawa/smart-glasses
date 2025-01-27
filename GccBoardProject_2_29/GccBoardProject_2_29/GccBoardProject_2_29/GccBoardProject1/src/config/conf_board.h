/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

/* TWI board defines. */ //could also be in camera
#define ID_BOARD_TWI                   ID_TWI0
#define BOARD_TWI                      TWI0
#define BOARD_TWI_IRQn                 TWI0_IRQn

/* SRAM board defines. */
#define SRAM_BASE                      (0x60000000UL) // SRAM address
#define SRAM_CS                        (0UL)
#define CAP_DEST                       SRAM_BASE

/* LCD board defines. */
#define ILI9325_LCD_CS                 (2UL) // Chip select number
#define IMAGE_WIDTH                    (320UL)
#define IMAGE_HEIGHT                   (240UL)

//from OV7740 ex
// Image sensor VSYNC pin.
#define OV2640_VSYNC_PIO	       PIOA
#define OV2640_VSYNC_ID		       ID_PIOA
#define OV2640_VSYNC_MASK          PIO_PA15
#define OV2640_VSYNC_TYPE          PIO_PULLUP

#define OV2640_DATA_BUS_PIO            PIOA
#define OV2640_DATA_BUS_ID             ID_PIOA

#endif // CONF_BOARD_H
