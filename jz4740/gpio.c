/*
 * gpio.c
 *
 * Init GPIO pins for PMP board.
 *
 * Author: Seeger Chin
 * e-mail: seeger.chin@gmail.com
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <bsp.h>
#include <jz4740.h>
#include <gpio.h>
                            
void gpio_init(void)        
{       
#if USE_UART0             
	__gpio_as_uart0();
#endif                    
#if USE_NAND
	__gpio_as_nand();
#endif

        __gpio_as_i2c();
        __gpio_as_output(POWER);
        __gpio_as_output(LV373_LE);
        __gpio_as_output(LCMPOWER);
        __gpio_as_output(HV7131POWER);
        __gpio_as_output(REDLED);

        /* Enable 74lv373 */
        __gpio_set_pin(LV373_LE);

        /* power on device */
        __gpio_set_pin(POWER);

        /* turn on RED LED */
        __gpio_clear_pin(REDLED);

        /* LCD back light power on */
        __gpio_set_pin(LCMPOWER);

        /* HV7131 back light power on */
        __gpio_set_pin(HV7131POWER);

        /* Disable 74lv373 */
        __gpio_clear_pin(LV373_LE);
}

