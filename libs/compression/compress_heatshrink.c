/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *  Wrapper for LZ4 encode/decode
 * ----------------------------------------------------------------------------
 */

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "compress_heatshrink.h"
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

#define BUFFERSIZE 128

/** gets data from array, writes to callback */
/** when we finish reading the array, the 'chunk' callback function could point to another chunk of dataLen size to continue encoding */
void heatshrink_encode(unsigned char *data, size_t dataLen, void (*callback)(unsigned char ch, uint32_t *cbdata), uint32_t *cbdata, unsigned char *(*chunk)(unsigned char *data, size_t dataLen, size_t chunks)) {
  heatshrink_encoder hse;
  uint8_t outBuf[BUFFERSIZE];
  heatshrink_encoder_reset(&hse);

  size_t i;
  size_t count = 0;
  size_t sunk = 0;
  size_t polled = 0;
  size_t chunks = 1;
  while (sunk < dataLen) {
    bool ok = heatshrink_encoder_sink(&hse, &data[sunk], dataLen - sunk, &count) >= 0;
    assert(ok);
    sunk += count;
    if (sunk == dataLen) {
      if(!chunk || !(data = chunk(data, dataLen, chunks++))) {
        heatshrink_encoder_finish(&hse);
      }
      else {
        sunk = 0;
      }
    }

    HSE_poll_res pres;
    do {
      pres = heatshrink_encoder_poll(&hse, outBuf, sizeof(outBuf), &count);
      assert(pres >= 0);
      for (i=0;i<count;i++)
        callback(outBuf[i], cbdata);
      polled += count;
    } while (pres == HSER_POLL_MORE);
    assert(pres == HSER_POLL_EMPTY);
    if (sunk == dataLen) {
      heatshrink_encoder_finish(&hse);
    }
  }
}

/** gets data from callback, writes it into array */
/** when destination is full, the 'chunk' callback function may point a new destiantion chunk of dataLen size */
void heatshrink_decode(int (*callback)(uint32_t *cbdata), uint32_t *cbdata, unsigned char *data, size_t dataLen, unsigned char *(*chunk)(unsigned char *data, size_t dataLen, size_t chunks)) {
  heatshrink_decoder hsd;
  uint8_t inBuf[BUFFERSIZE];
  uint8_t outBuf[BUFFERSIZE];
  heatshrink_decoder_reset(&hsd);

  size_t count = 0;
  size_t polled = 0;
  int lastByte = 0;
  size_t inBufCount = 0;
  size_t chunks = 1;
  while (lastByte >= 0 || inBufCount>0) {
    // Read data from flash
    while (inBufCount<BUFFERSIZE && lastByte>=0) {
      lastByte = callback(cbdata);
      if (lastByte >= 0)
        inBuf[inBufCount++] = (uint8_t)lastByte;
    }
    // decode
    bool ok = heatshrink_decoder_sink(&hsd, inBuf, inBufCount, &count) >= 0;
    // if not all the data was read, shift what's left to the start of our buffer
    if (count < inBufCount) {
      size_t i;
      for (i=count;i<inBufCount;i++)
        inBuf[i-count] = inBuf[i];
    }
    inBufCount -= count;
    assert(ok);
    if (lastByte < 0) {
      heatshrink_decoder_finish(&hsd);
    }

    HSE_poll_res pres;
    do {
      size_t available = dataLen - polled ;
      if(available) {
        pres = heatshrink_decoder_poll(&hsd, &data[polled], available, &count);
        assert(pres >= 0);
	polled += count;
      }
      else {
	// This is needed because heatshrink_decoder_poll will always return HSER_POLL_MORE instead of HSER_POLL_EMPTY if buffer size is 0
	// Should fix heatshrink_decoder_poll instead :)
        pres = heatshrink_decoder_poll(&hsd, outBuf, dataLen > BUFFERSIZE ? BUFFERSIZE : dataLen, &count);
        assert(pres >= 0);
        if(count) {
	  assert(chunk);
	  data = chunk(data, dataLen, chunks++);
	  assert(data);
          for(polled = 0 ; polled < count ; polled++) {
            data[polled] = outBuf[polled];
          }
        }
      }
    } while (pres == HSER_POLL_MORE);
    assert(pres == HSER_POLL_EMPTY);
    if (lastByte < 0) {
      heatshrink_decoder_finish(&hsd);
    }
  }
}
