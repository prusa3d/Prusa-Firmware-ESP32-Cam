/**
 * exif.c - Functions to generate Exif metadata
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
 */

#include <time.h>
#include <sys/time.h>

#include "exif.h"
#include "exif_defines.h"

// TIFF header byte order
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define TIFF_BYTE_ORDER 0x4949
#else // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define TIFF_BYTE_ORDER 0x4d4d
#endif // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// Reimplementation of htons() that can be used to initialize a struct member.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define htons_macro(x) \
  (((x) & 0x00ff) << 8 | \
   ((x) & 0xff00) >> 8)
#else // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define htons_macro(x) (x)
#endif // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

/**
 * Type for storing Tiff Rational typed data
 */
#pragma pack(1)
typedef struct {
  uint32_t num;
  uint32_t denom;
} TiffRational;
#pragma pack()

/**
 * Type used for IFD entries
 *
 * This type is used to store a value within the Tiff IFD.
 */
#pragma pack(1)
typedef struct {
  uint16_t tag; // Data tag
  uint16_t type; // Data type
  uint32_t length; // length of data
  // Offset of data from start of TIFF data, or data it self if length <= 4.
  // Always use the IFD_SET_*() macros to modify the value.
  uint32_t value;
} IfdEntry;
#pragma pack()

// Some helper macros to initialize the IFD value's. The types shorter then
// 4-bytes need to be aligned such that they are directly after the previous
// field. These macros take care of this.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define IFD_SET_BYTE(VAL)   (VAL)
# define IFD_SET_SHORT(VAL)  (VAL)
# define IFD_SET_LONG(VAL)   (VAL)
# define IFD_SET_OFFSET(REF, VAR) offsetof(REF, VAR)
# define IFD_SET_UNDEF(V0, V1, V2, V3) \
                             (((V3) & 0xff) << 24 | \
                              ((V2) & 0xff) << 16 | \
                              ((V1) & 0xff) << 8 | \
                              ((V0) & 0xff))
#else // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define IFD_SET_BYTE(VAL)   (((VAL) & 0xff) << 24)
# define IFD_SET_SHORT(VAL)  (((VAL) & 0xffff) << 16)
# define IFD_SET_LONG(VAL)   (VAL)
# define IFD_SET_OFFSET(REF, VAR) offsetof(REF, VAR)
# define IFD_SET_UNDEF(V0, V1, V2, V3) \
                             (((V0) & 0xff) << 24 | \
                              ((V1) & 0xff) << 16 | \
                              ((V2) & 0xff) << 8 | \
                              ((V3) & 0xff))
#endif // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// Amount of entries in the 0th IFD
#ifdef WITH_GNSS
# define IFD0_ENTRY_CNT 11
#else
# define IFD0_ENTRY_CNT 12
#endif

// Amount of entries in the Exif private IFD
#define IFD_EXIF_ENTRY_CNT 6

// Amount of entries in the GPS private IFD
#define IFD_GPS_ENTRY_CNT 12

// GPS map datum, probably always 'WGS-84'
#define GPS_MAP_DATUM "WGS-84"

/**
 * New Jpeg/Exif header
 *
 * This defines the new JPEG/Exif header that is added to the images. To keep
 * it simple, the structure of the header is completely static.
 */
#pragma pack(1)
struct JpegExifHdr {
  uint16_t jpeg_soi; // htons(0xffd8)
  uint16_t marker; // htons(0xffe1)
  uint16_t len;    // htons(length)
  uint8_t exif_identifier[6]; // 'Exif\0\0'

  struct TiffData {
    struct {
      uint16_t byte_order; // 'II' || 'MM'
      uint16_t signature; // 0x002A
      uint32_t ifd_offset; // 0x8
    } tiff_hdr;
    struct {
      uint16_t cnt; // amount of entries
      IfdEntry entries[IFD0_ENTRY_CNT];
      uint32_t next_ifd; // Offset of next IFD, or 0x0 if last IFD
    } ifd0;
    struct {
      TiffRational XYResolution;
      char make[sizeof(CAMERA_MAKE)];
      char model[sizeof(CAMERA_MODEL)];
      char software[sizeof(CAMERA_SOFTWARE)];
      char datetime[20];
    } ifd0_data;
    struct {
      uint16_t cnt; // amount of entries
      IfdEntry entries[IFD_EXIF_ENTRY_CNT];
      uint32_t next_ifd; // Offset of next IFD, or 0x0 if last IFD
    } ifd_exif;
#ifdef WITH_GNSS
    struct {
      uint16_t cnt; // amount of entries
      IfdEntry entries[IFD_GPS_ENTRY_CNT];
      uint32_t next_ifd; // Offset of next IFD, or 0x0 if last IFD
    } ifd_gps;
    struct {
      TiffRational latitude[3];
      TiffRational longitude[3];
      TiffRational altitude;
      TiffRational time_stamp[3];
      char map_datum[sizeof(GPS_MAP_DATUM)];
      uint8_t processing_method[8 + 3];
      char date_stamp[11];
    } ifd_gps_data;
#endif //WITH_GNSS
  } tiff_data;
} exif_hdr = {
  htons_macro(0xffd8),
  htons_macro(0xffe1),
  htons_macro(sizeof(JpegExifHdr) - offsetof(JpegExifHdr, len)),
  { 'E', 'x', 'i', 'f', 0, 0 },
  {
    .tiff_hdr = { TIFF_BYTE_ORDER, 0x002A, 0x8 },
    .ifd0 = {
      .cnt = IFD0_ENTRY_CNT,
      .entries = {
        { TagTiffMake,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd0_data.make),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data.make) },
        { TagTiffModel,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd0_data.model),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data.model) },
#define TAG_IFD0_ORIENTATION_IDX 2
        { TagTiffOrientation,
          TiffTypeShort, 1,
          IFD_SET_SHORT(1) },
        { TagTiffXResolution,
          TiffTypeRational, 1,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data) },
        { TagTiffYResolution,
          TiffTypeRational, 1,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data) },
        { TagTiffResolutionUnit,
          TiffTypeShort, 1,
          IFD_SET_SHORT(0x0002) },
        { TagTiffSoftware,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd0_data.software),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data.software) },
        { TagTiffDateTime,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd0_data.datetime),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd0_data.datetime) },
        { TagTiffYCbCrPositioning,
          TiffTypeShort, 1,
          IFD_SET_SHORT(0x0001) },
        { TagTiffExifIFD,
          TiffTypeLong, 1,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_exif) },
#ifdef WITH_GNSS
        { TagTiffGPSIFD,
          TiffTypeLong, 1,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps) },
#endif // WITH_GNSS
      },
      .next_ifd = 0
    },
    .ifd0_data = {
      { 72, 1 },
      CAMERA_MAKE,
      CAMERA_MODEL,
      CAMERA_SOFTWARE,
      "    :  :     :  :  ",
    },
    .ifd_exif = {
      .cnt = IFD_EXIF_ENTRY_CNT,
      .entries = {
        { TagExifVersion,
          TiffTypeUndef, 4,
          IFD_SET_UNDEF(0x30, 0x32, 0x33, 0x30) },
        { TagExifComponentsConfiguration,
          TiffTypeUndef, 4,
          IFD_SET_UNDEF(0x01, 0x02, 0x03, 0x00) },
#define TAG_EXIF_SUBSEC_TIME_IDX 2
        { TagExifSubSecTime,
          TiffTypeAscii, 4,
          IFD_SET_UNDEF(0x20, 0x20, 0x20, 0x00) },
        { TagExifColorSpace,
          TiffTypeShort, 1,
          IFD_SET_SHORT(1) },
#define TAG_EXIF_PIXEL_X_DIMENSION_IDX 4
        { TagExifPixelXDimension,
          TiffTypeShort, 1,
          IFD_SET_SHORT(1600) },
#define TAG_EXIF_PIXEL_Y_DIMENSION_IDX (TAG_EXIF_PIXEL_X_DIMENSION_IDX + 1)
        { TagExifPixelYDimension,
          TiffTypeShort, 1,
          IFD_SET_SHORT(1200) },
      },
      .next_ifd = 0
    },
#ifdef WITH_GNSS
    .ifd_gps = {
      .cnt = IFD_GPS_ENTRY_CNT,
      .entries = {
        { TagGPSVersionID,
          TiffTypeByte, 4,
          IFD_SET_UNDEF(2, 3, 0, 0) },
#define TAG_GPS_LATITUDE_REF_IDX 1
        { TagGPSLatitudeRef,
          TiffTypeAscii, 2,
          IFD_SET_UNDEF(0x00, 0x00, 0x00, 0x00) },
        { TagGPSLatitude,
          TiffTypeRational, 3,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.latitude) },
#define TAG_GPS_LONGITUDE_REF_IDX 3
        { TagGPSLongitudeRef,
          TiffTypeAscii, 2,
          IFD_SET_UNDEF(0x00, 0x00, 0x00, 0x00) },
        { TagGPSLongitude,
          TiffTypeRational, 3,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.longitude) },
#define TAG_GPS_ALTITUDE_REF_IDX 5
        { TagGPSAltitudeRef,
          TiffTypeByte, 1,
          IFD_SET_UNDEF(0x00, 0x00, 0x00, 0x00) },
        { TagGPSAltitude,
          TiffTypeRational, 1,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.altitude) },
        { TagGPSTimeStamp,
          TiffTypeRational, 3,
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.time_stamp) },
#define TAG_GPS_STATUS_IDX 8
        { TagGPSStatus,
          TiffTypeAscii, 2,
          IFD_SET_UNDEF('V', 0x00, 0x00, 0x00) },
/*TODO: currently not available from MicroNMEA
        { TagGPSMeasureMode,
          TiffTypeAscii, 2,
          IFD_SET_UNDEF('2', 0x00, 0x00, 0x00) },*/
        { TagGPSMapDatum,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd_gps_data.map_datum),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.map_datum) },
        { TagGPSProcessingMethod,
          TiffTypeUndef,
          sizeof(exif_hdr.tiff_data.ifd_gps_data.processing_method),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.processing_method) },
        { TagGPSDateStamp,
          TiffTypeAscii,
          sizeof(exif_hdr.tiff_data.ifd_gps_data.date_stamp),
          IFD_SET_OFFSET(JpegExifHdr::TiffData, ifd_gps_data.date_stamp) },
      },
      .next_ifd = 0
    },
    .ifd_gps_data = {
      { { 0, 1000*1000 }, { 0, 1 }, { 0, 1 } },
      { { 0, 1000*1000 }, { 0, 1 }, { 0, 1 } },
      { 0, 1000 },
      { { 0, 1 }, { 0, 1 }, { 0, 1 } },
      GPS_MAP_DATUM,
      { 0x41, 0x53, 0x43, 0x49, 0x49, 0x00, 0x00, 0x00, 'G', 'P', 'S' },
      "    :  :  ",
    }
#endif // WITH_GNSS
  }
};
#pragma pack()

bool update_exif_from_cfg(const uint8_t c)
{
  exif_hdr.tiff_data.ifd0.entries[TAG_IFD0_ORIENTATION_IDX].value = IFD_SET_SHORT(c);

  return true;
}

#ifdef WITH_GNSS
void update_exif_gps(const MicroNMEA& nmea)
{
  // Latitude
  long lat = nmea.getLatitude();
  if (lat < 0) {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_LATITUDE_REF_IDX].value = IFD_SET_BYTE('S');
    lat *= -1;
  } else {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_LATITUDE_REF_IDX].value = IFD_SET_BYTE('N');
  }
  exif_hdr.tiff_data.ifd_gps_data.latitude[0].num = lat;

  // Longitude
  long lon = nmea.getLongitude();
  if (lon < 0) {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_LONGITUDE_REF_IDX].value = IFD_SET_BYTE('W');
    lon *= -1;
  } else {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_LONGITUDE_REF_IDX].value = IFD_SET_BYTE('E');
  }
  exif_hdr.tiff_data.ifd_gps_data.longitude[0].num = lon;

  // Altitude
  long alt;
  if (nmea.getAltitude(alt)) {
    if (alt < 0) {
      exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_ALTITUDE_REF_IDX].value = IFD_SET_BYTE(1);
      alt *= -1;
    } else {
      exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_ALTITUDE_REF_IDX].value = IFD_SET_BYTE(0);
    }
    exif_hdr.tiff_data.ifd_gps_data.altitude.num = alt;
  } else {
    exif_hdr.tiff_data.ifd_gps_data.altitude.num = 0;
  }

  // time stamp
  exif_hdr.tiff_data.ifd_gps_data.time_stamp[0].num = nmea.getHour();
  exif_hdr.tiff_data.ifd_gps_data.time_stamp[1].num = nmea.getMinute();
  exif_hdr.tiff_data.ifd_gps_data.time_stamp[2].num = nmea.getSecond();

  // GPS Status
  if (nmea.isValid()) {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_STATUS_IDX].value = IFD_SET_BYTE('A');
  } else {
    exif_hdr.tiff_data.ifd_gps.entries[TAG_GPS_STATUS_IDX].value = IFD_SET_BYTE('V');
  }

  // date stamp
  snprintf(exif_hdr.tiff_data.ifd_gps_data.date_stamp,
      sizeof(exif_hdr.tiff_data.ifd_gps_data.date_stamp),
      "%04u:%02u:%02u",
      nmea.getYear(),
      nmea.getMonth(),
      nmea.getDay());
}
#endif // WITH_GNSS

const uint8_t *get_exif_header(camera_fb_t *fb, const uint8_t **exif_buf, size_t *exif_len)
{
  // TODO: pass config to function and use that to set some of the image
  // taking conditions. Or do this only once, with a update config
  // function????

  // Get current time
  struct timeval now_tv;
  if (gettimeofday(&now_tv, NULL) != 0) {
    now_tv.tv_sec = time(NULL);
    now_tv.tv_usec = 0;
  }

  // Set date time
  struct tm timeinfo;
  localtime_r(&now_tv.tv_sec, &timeinfo);
  strftime(exif_hdr.tiff_data.ifd0_data.datetime,
      sizeof(exif_hdr.tiff_data.ifd0_data.datetime),
      "%Y:%m:%d %H:%M:%S", &timeinfo);

  // Set sub-seconds time
  snprintf( (char *) &(exif_hdr.tiff_data.ifd_exif.entries[TAG_EXIF_SUBSEC_TIME_IDX].value), 9, "%03ld", now_tv.tv_usec/1000);

  // Update image dimensions
  exif_hdr.tiff_data.ifd_exif.entries[TAG_EXIF_PIXEL_X_DIMENSION_IDX].value = IFD_SET_SHORT(fb->width);
  exif_hdr.tiff_data.ifd_exif.entries[TAG_EXIF_PIXEL_Y_DIMENSION_IDX].value = IFD_SET_SHORT(fb->height);

  *exif_len = sizeof(exif_hdr);

  if (exif_buf != NULL) {
    *exif_buf = (uint8_t *) &exif_hdr;
  }

  return (uint8_t *) &exif_hdr;
}

size_t get_jpeg_data_offset(camera_fb_t *fb)
{
  if (fb->len < 6) {
    return 0;
  }

  // Check JPEG SOI
  if (fb->buf[0] != 0xff || fb->buf[1] != 0xd8) {
    return 0;
  }
  size_t data_offset = 2; // Offset to first JPEG segment after header

  // Check if JFIF header
  if (fb->buf[2] == 0xff && fb->buf[3] == 0xe0) {
    uint16_t jfif_len = fb->buf[4] << 8 | fb->buf[5];

    data_offset += 2 + jfif_len;
  }

  if (data_offset >= fb->len) {
    return 0;
  }

  return data_offset;
}
