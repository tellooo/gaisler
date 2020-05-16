#ifndef GR716_PIN_H
#define GR716_PIN_H

#define IO_MODE_GPIO      0x0 
#define IO_MODE_APBUART   0x1 
#define IO_MODE_MEM       0x2 
#define IO_MODE_PW        0x3 
#define IO_MODE_1553      0x4  
#define IO_MODE_CAN       0x5  
#define IO_MODE_I2C       0x6 
#define IO_MODE_SPI       0x7  
#define IO_MODE_ADC       0x8  
#define IO_MODE_DAC       0x8  
#define IO_MODE_ADCDAC    0x9  
#define IO_MODE_PWM       0xA  
#define IO_MODE_SPW       0xB  
#define IO_MODE_SPI4S     0xB  
#define IO_MODE_AHBUART   0xC 
#define IO_MODE_TDP       0xD 
#define IO_MODE_MAX       0xD 

/*
 * configure one IO switch matrix entry
 *
 * This function updates one field in SYS.CFG.GPx to configure the
 * specified pin with the functionality requested by mode.
 *
 * Parameters pin and mode are range checked before registers are written.
 *
 * pin:           GPIO pin number (0..63)
 * mode:          Any of IO_MODE_* (0..0xe)
 * return:        0 on success, else non-zero.
 */
int gr716_set_pinfunc(
        unsigned int pin,
        unsigned int mode
);

#define LVDS_MODE_SPW       0x0
#define LVDS_MODE_SPI4S     0x1
#define LVDS_MODE_SPIM      0x2
#define LVDS_MODE_SPIS      0x3
#define LVDS_DISABLE        0x8

/*
 * configure LVDS functionality
 *
 * This function updates one field in SYS.CFG.LVDS to configure the
 * on-chip LVDS transceivers with the functionality requested by mode.
 *
 * mode:          Any of LVDS_MODE_* (0..3 or 8)
 * return:        0 on success, else non-zero.
 */
int gr716_set_lvdsfunc(
        unsigned int mode
);

#endif

