/**
 * exif_defines.h - Defines for constants from Exif v2.3 standard
 *
 * Written 2019, David Imhoff <dimhoff.devel@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __EXIF_DEFINES_H__
#define __EXIF_DEFINES_H__

// TIFF Rev. 6.0 Attribute Information Used in Exif
enum {
	// A. Tags relating to image data structure
	TagTiffImageWidth          = 0x100, // Image width
	TagTiffImageLength         = 0x101, // Image height
	TagTiffBitsPerSample       = 0x102, // Number of bits per component
	TagTiffCompression         = 0x103, // Compression scheme
	TagTiffPhotometricInterpretation = 0x106, // Pixel composition
	TagTiffOrientation         = 0x112, // Orientation of image
	TagTiffSamplesPerPixel     = 0x115, // Number of components
	TagTiffPlanarConfiguration = 0x11C, // Image data arrangement
	TagTiffYCbCrSubSampling    = 0x212, // Subsampling ratio of Y to C
	TagTiffYCbCrPositioning    = 0x213, // Y and C positioning
	TagTiffXResolution         = 0x11A, // Image resolution in width direction
	TagTiffYResolution         = 0x11B, // Image resolution in height direction
	TagTiffResolutionUnit      = 0x128, // Unit of X and Y resolution

	// B. Tags relating to recording offset
	TagTiffStripOffsets                = 0x111, // Image data location
	TagTiffRowsPerStrip                = 0x116, // Number of rows per strip
	TagTiffStripByteCounts             = 0x117, // Bytes per compressed strip
	TagTiffJPEGInterchangeFormat       = 0x201, // Offset to JPEG SOI
	TagTiffJPEGInterchangeFormatLength = 0x202, // Bytes of JPEG data

	// C. Tags relating to image data characteristics
	TagTiffTransferFunction      = 0x12D, // Transfer function
	TagTiffWhitePoint            = 0x13E, // White point chromaticity
	TagTiffPrimaryChromaticities = 0x13F, // Chromaticities of primaries
	TagTiffYCbCrCoefficients     = 0x211, // Color space transformation matrix coefficients
	TagTiffReferenceBlackWhite   = 0x214, // Pair of black and white reference values

	// D. Other tags
	TagTiffDateTime         = 0x132, // File change date and time
	TagTiffImageDescription = 0x10E, // Image title
	TagTiffMake             = 0x10F, // Image input equipment manufacturer
	TagTiffModel            = 0x110, // Image input equipment model
	TagTiffSoftware         = 0x131, // Software used
	TagTiffArtist           = 0x13B, // Person who created the image
	TagTiffCopyright        = 0x8298, // Copyright holder

	// Exif private IFD's
	TagTiffExifIFD             = 0x8769, // Exif IFD pointer
	TagTiffGPSIFD              = 0x8825, // Exif IFD pointer
	TagTiffInteroperabilityIFD = 0xA005, // Exif IFD pointer
};

// Exif IFD Attribute Information
enum {
	// Tags Relating to Version
	TagExifVersion         = 0x9000, // Exif version
	TagExifFlashpixVersion = 0xA000, // Supported Flashpix version

	// Tag Relating to Image Data Characteristics
	TagExifColorSpace = 0xA001, // Color space information
	TagExifGamma      = 0xA500, // Gamma

	// Tags Relating to Image Configuration
	TagExifComponentsConfiguration = 0x9101, // Meaning of each component
	TagExifCompressedBitsPerPixel  = 0x9102, // Image compression mode
	TagExifPixelXDimension         = 0xA002, // Valid image width
	TagExifPixelYDimension         = 0xA003, // Valid image height

	// Tags Relating to User Information
	TagExifMakerNote   = 0x927C, // Manufacturer notes
	TagExifUserComment = 0x9286, // User comments

	// Tag Relating to Related File Information
	TagExifRelatedSoundFile = 0xA004, // Related audio file

	// Tags Relating to Date and Time
	TagExifDateTimeOriginal    = 0x9003, // Date and time of original data generation
	TagExifDateTimeDigitized   = 0x9004, // Date and time of digital data generation
	TagExifSubSecTime          = 0x9290, // DateTime subseconds
	TagExifSubSecTimeOriginal  = 0x9291, // DateTimeOriginal subseconds
	TagExifSubSecTimeDigitized = 0x9292, // DateTimeDigitized subseconds

	// Tags Relating to Picture-Taking Conditions
	TagExifExposureTime              = 0x829A, // Exposure time
	TagExifFNumber                   = 0x829D, // F number
	TagExifExposureProgram           = 0x8822, // Exposure program
	TagExifSpectralSensitivity       = 0x8824, // Spectral sensitivity
	TagExifPhotographicSensitivity   = 0x8827, // Photographic Sensitivity
	TagExifOECF                      = 0x8828, // Optoelectric conversion factor
	TagExifSensitivityType           = 0x8830, // Sensitivity Type
	TagExifStandardOutputSensitivity = 0x8831, // Standard Output Sensitivity
	TagExifRecommendedExposureIndex  = 0x8832, // Recommended ExposureIndex
	TagExifISOSpeed                  = 0x8833, // ISO Speed
	TagExifISOSpeedLatitudeyyy       = 0x8834, // ISO Speed Latitude yyy
	TagExifISOSpeedLatitudezzz       = 0x8835, // ISO Speed Latitude zzz
	TagExifShutterSpeedValue         = 0x9201, // Shutter speed
	TagExifApertureValue             = 0x9202, // Aperture
	TagExifBrightnessValue           = 0x9203, // Brightness
	TagExifExposureBiasValue         = 0x9204, // Exposure bias
	TagExifMaxApertureValue          = 0x9205, // Maximum lens aperture
	TagExifSubjectDistance           = 0x9206, // Subject distance
	TagExifMeteringMode              = 0x9207, // Metering mode
	TagExifLightSource               = 0x9208, // Light source
	TagExifFlash                     = 0x9209, // Flash
	TagExifFocalLength               = 0x920A, // Lens focal length
	TagExifSubjectArea               = 0x9214, // Subject area
	TagExifFlashEnergy               = 0xA20B, // Flash energy
	TagExifSpatialFrequencyResponse  = 0xA20C, // Spatial frequency response
	TagExifFocalPlaneXResolution     = 0xA20E, // Focal plane X resolution
	TagExifFocalPlaneYResolution     = 0xA20F, // Focal plane Y resolution
	TagExifFocalPlaneResolutionUnit  = 0xA210, // Focal plane resolution unit
	TagExifSubjectLocation           = 0xA214, // Subject location
	TagExifExposureIndex             = 0xA215, // Exposure index
	TagExifSensingMethod             = 0xA217, // Sensing method
	TagExifFileSource                = 0xA300, // File source
	TagExifSceneType                 = 0xA301, // Scene type
	TagExifCFAPattern                = 0xA302, // CFA pattern
	TagExifCustomRendered            = 0xA401, // Custom image processing
	TagExifExposureMode              = 0xA402, // Exposure mode
	TagExifWhiteBalance              = 0xA403, // White balance
	TagExifDigitalZoomRatio          = 0xA404, // Digital zoom ratio
	TagExifFocalLengthIn35mmFilm     = 0xA405, // Focal length in 35 mm film
	TagExifSceneCaptureType          = 0xA406, // Scene capture type
	TagExifGainControl               = 0xA407, // Gain control
	TagExifContrast                  = 0xA408, // Contrast
	TagExifSaturation                = 0xA409, // Saturation
	TagExifSharpness                 = 0xA40A, // Sharpness
	TagExifDeviceSettingDescription  = 0xA40B, // Device settings description
	TagExifSubjectDistanceRange      = 0xA40C, // Subject distance range

	// Other Tags
	TagExifImageUniqueID     = 0xA420, // Unique image ID
	TagExifCameraOwnerName   = 0xA430, // Camera Owner Name
	TagExifBodySerialNumber  = 0xA431, // Body Serial Number
	TagExifLensSpecification = 0xA432, // Lens Specification
	TagExifLensMake          = 0xA433, // Lens Make
	TagExifLensModel         = 0xA434, // Lens Model
	TagExifLensSerialNumber  = 0xA435, // Lens Serial Number
};

// GPS Attribute Information
enum {
	TagGPSVersionID         = 0x00, // GPS tag version
	TagGPSLatitudeRef       = 0x01, // North or South Latitude
	TagGPSLatitude          = 0x02, // Latitude
	TagGPSLongitudeRef      = 0x03, // East or West Longitude
	TagGPSLongitude         = 0x04, // Longitude
	TagGPSAltitudeRef       = 0x05, // Altitude reference
	TagGPSAltitude          = 0x06, // Altitude
	TagGPSTimeStamp         = 0x07, // GPS time (atomic clock)
	TagGPSSatellites        = 0x08, // GPS satellites used for measurement
	TagGPSStatus            = 0x09, // GPS receiver status
	TagGPSMeasureMode       = 0x0A, // GPS measurement mode
	TagGPSDOP               = 0x0B, // Measurement precision
	TagGPSSpeedRef          = 0x0C, // Speed unit
	TagGPSSpeed             = 0x0D, // Speed of GPS receiver
	TagGPSTrackRef          = 0x0E, // Reference for direction of movement
	TagGPSTrack             = 0x0F, // Direction of movement
	TagGPSImgDirectionRef   = 0x10, // Reference for direction of image
	TagGPSImgDirection      = 0x11, // Direction of image
	TagGPSMapDatum          = 0x12, // Geodetic survey data used
	TagGPSDestLatitudeRef   = 0x13, // Reference for latitude of destination
	TagGPSDestLatitude      = 0x14, // Latitude of destination
	TagGPSDestLongitudeRef  = 0x15, // Reference for longitude of destination
	TagGPSDestLongitude     = 0x16, // Longitude of destination
	TagGPSDestBearingRef    = 0x17, // Reference for bearing of destination
	TagGPSDestBearing       = 0x18, // Bearing of destination
	TagGPSDestDistanceRef   = 0x19, // Reference for distance to destination
	TagGPSDestDistance      = 0x1A, // Distance to destination
	TagGPSProcessingMethod  = 0x1B, // Name of GPS processing method
	TagGPSAreaInformation   = 0x1C, // Name of GPS area
	TagGPSDateStamp         = 0x1D, // GPS date
	TagGPSDifferential      = 0x1E, // GPS differential correction
	TagGPSHPositioningError = 0x1F, // Horizontal positioning error
};

// TIFF type fields values
enum {
	TiffTypeByte      = 1,
	TiffTypeAscii     = 2,
	TiffTypeShort     = 3,
	TiffTypeLong      = 4,
	TiffTypeRational  = 5,
	TiffTypeUndef     = 7,
	TiffTypeSLong     = 9,
	TiffTypeSRational = 10,
};

#endif // __EXIF_DEFINES_H__
