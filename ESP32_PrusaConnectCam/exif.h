/**
 * exif.h - Functions to generate Exif metadata
 *
 * Copyright (c) 2019, David Imhoff <dimhoff.devel@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Project URL: https://github.com/dimhoff/ESP32-CAM_Interval
 * 
 */
#ifndef __EXIF_H__
#define __EXIF_H__

#include <stdint.h>
#include <stddef.h>
#include "esp_camera.h"
#ifdef WITH_GNSS
# include <MicroNMEA.h>
#endif

#include "mcu_cfg.h"

/**
 * Update the configurable EXIF header fields
 *
 * Update all field in the EXIF header that depend on the configuration. Needs
 * to be called every time the configuration is changed.
 *
 * @returns	True on success, else false
 */
bool update_exif_from_cfg(const uint8_t);

#ifdef WITH_GNSS
/**
 * Update GPS data in EXIF header
 *
 * Should be called for every call to get_exif_header() if GNSS support is enabled.
 */
void update_exif_gps(const MicroNMEA& nmea);
#endif // WITH_GNSS

/**
 * Get Exif header
 *
 * Get a buffer with a new Exif header that can be prepended to a JPEG image.
 * The JPEG does need to be stripped from its original header first, see
 * get_jpeg_data_offset().
 *
 * The returned pointer point so a static buffer, and should not be altered.
 * This function is not reentrant safe.
 *
 * @param fb		Frame buffer of captured image. Encoding is expected to
 *                      be JPEG
 * @param exif_buf	If not NULL, used to return pointer to Exif buffer in
 * @param exif_buf	Used to return the size of the Exif buffer
 *
 * @returns		Pointer to Exif buffer, or NULL on error
 */
const uint8_t *get_exif_header(camera_fb_t *fb, const uint8_t **exif_buf, size_t *exif_len);

/**
 * Get offset of first none header byte in buffer
 *
 * Get the offset of the first none JPEG header byte in capture buffer. This
 * can be used to strip the JPEG SOI and JFIF headers from an image.
 *
 * @returns	offset of first non header byte, or 0 on error
 */
size_t get_jpeg_data_offset(camera_fb_t *fb);

#endif // __EXIF_H__
