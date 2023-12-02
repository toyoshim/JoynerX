// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "gpio.h"
#include "io.h"
#include "timer3.h"

#include "controller.h"

static __code uint8_t* flash = (__code uint8_t*)0xf000;

static struct settings settings;
static uint8_t current_setting;  // valid range is [0...5]
static uint16_t poll_msec = 0;
static uint8_t led_current_mode = 0;

static void apply(void) {
  uint16_t offset = 10;
  for (uint8_t id = 0; id < current_setting; ++id) {
    offset += 169;
  }
  const uint8_t core0 = flash[offset++];
  const uint8_t core1 = flash[offset++];
  const uint8_t core2 = flash[offset++];
  settings.id = core0 >> 5;
  if (settings.id > 3) {
    settings.id = 0;
  }
  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 6; ++i) {
      const uint8_t data = flash[offset++];
      const uint8_t type = data >> 4;
      settings.analog_index[p][i] = (type == 2) ? (data & 7) : 0xff;
      settings.analog_polarity[p][i] = data & 8;
    }
  }

  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 16; ++i) {
      for (uint8_t j = 0; j < 4; ++j) {
        settings.digital_map[p][i].data[j] = flash[offset++];
      }
    }
  }

  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 6; ++i) {
      const uint8_t data = flash[offset++];
      settings.rapid_fire[p][i * 2 + 0] = (data >> 4) & 7;
      settings.rapid_fire[p][i * 2 + 1] = data & 7;
    }
  }
  settings.sequence[0].pattern = 0xff;
  settings.sequence[0].bit = 1;
  settings.sequence[0].mask = 0xff;
  settings.sequence[0].invert = false;
  settings.sequence[0].on = true;
  for (uint8_t i = 1; i < 8; ++i) {
    settings.sequence[i].pattern = flash[offset++];
    settings.sequence[i].bit = 1;
    settings.sequence[i].mask = (2 << (flash[offset] & 7)) - 1;
    settings.sequence[i].invert = flash[offset++] & 0x80;
    settings.sequence[i].on = true;
  }

  controller_reset();
}

void settings_init(void) {
  led_init(1, 5, LOW);

  if (flash[0] != 'I' || flash[1] != 'O' || flash[2] != 'N' ||
      flash[3] != 'C' || flash[4] != 1) {
    led_mode(L_BLINK_THREE_TIMES);
    for (;;) {
      led_poll();
    }
  }
  current_setting = 0;
  apply();

  settings_led_mode(L_ON);
}

void settings_poll(void) {
  led_poll();
}

void settings_select(uint8_t id) {
  current_setting = id;
  apply();
}

struct settings* settings_get(void) {
  return &settings;
}

void settings_led_mode(uint8_t mode) {
  led_current_mode = mode;
  led_mode(led_current_mode);
}

void settings_rapid_sync(void) {
  for (uint8_t i = 1; i < 8; ++i) {
    settings.sequence[i].bit <<= 1;
    if (0 == (settings.sequence[i].bit & settings.sequence[i].mask)) {
      settings.sequence[i].bit = 1;
    }
    settings.sequence[i].on =
        settings.sequence[i].pattern & settings.sequence[i].bit;
  }
}