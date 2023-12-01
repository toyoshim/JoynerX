// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __controller_h__
#define __controller_h__

#include <stdbool.h>
#include <stdint.h>

#include "hid.h"

void controller_reset(void);
void controller_update(const uint8_t hub,
                       const struct hid_info* info,
                       const uint8_t* data,
                       uint16_t size);
void controller_poll(void);
uint8_t controller_head(void);
uint8_t controller_data(uint8_t player, uint8_t index, uint8_t gpout);
uint16_t controller_analog(uint8_t index);

#endif  // __controller_h__