#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# This file contains information for a specific board - the available pins, and where LEDs,
# Buttons, and other in-built peripherals are. It is used to build documentation as well
# as various source and header files for Espruino.
# ----------------------------------------------------------------------------------------

from ESP8266_BOARD import *;

if 'variables' in info : del info['variables']

info['resizable_jsvars'] = True
info['binary_name'] = 'espruino_%v_esp8266_bigflash'
info['include'] = 'esp8266_bigflash.h'

chip['saved_code'] = {
    'address' : '(uint32_t)_bigflash_free_start',
    'size' : '(_bigflash_free_end-_bigflash_free_start)',
    'page_size' : 4096,
    'flash_available' : 896, # firmware can be up to this size
};

