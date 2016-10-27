/******************************************************************************
 * 
 * Relay card control utility: Generic type definitions
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   There are 3 ways to control the relais:
 *    1. via command line
 *    2. via web interface using a browser
 *    3. via HTTP API using a client application
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   19/08/2015
 *
 * Copyright 2015, Ondrej Wisniewski 
 * 
 * This file is part of crelay.
 * 
 * crelay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with crelay.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *****************************************************************************/ 
 
#ifndef data_types_h
#define data_types_h

/* Config data struct */
typedef struct
{
    /* [HTTP server] */
    const char*  server_iface;
    uint16_t server_port;
    const char* relay1_label;
    const char* relay2_label;
    const char* relay3_label;
    const char* relay4_label;
    const char* relay5_label;
    const char* relay6_label;
    const char* relay7_label;
    const char* relay8_label;
    
    /* [GPIO drv] */
    uint8_t gpio_num_relays;
    uint8_t relay1_gpio_pin;
    uint8_t relay2_gpio_pin;
    uint8_t relay3_gpio_pin;
    uint8_t relay4_gpio_pin;
    uint8_t relay5_gpio_pin;
    uint8_t relay6_gpio_pin;
    uint8_t relay7_gpio_pin;
    uint8_t relay8_gpio_pin;
    
    /* [Sainsmart drv] */
    uint8_t sainsmart_num_relays;
    
} config_t;

#endif
