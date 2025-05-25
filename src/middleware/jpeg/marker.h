/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_MARKER_H
#define IG_JPEG_MARKER_H


#define JPEG_MARKER_PREFIX (0xFF)

#define JPEG_SOI_MARKER (0xD8) // start of image
#define JPEG_DHT_MARKER (0xC4) // define huffman table
#define JPEG_SOF_MARKER (0xC0) // start of frame
#define JPEG_SOS_MARKER (0xDA) // start of scan
#define JPEG_EOI_MARKER (0xD9) // end of image

#define JPEG_JFIF_APP0_MARKER (0xE0) // application segment 0

#define JPEG_EXIF_APP1_MARKER (0xE1) // application segment 1
#define JPEG_EXIF_APP2_MARKER (0xE2) // application segment 2
#define JPEG_EXIF_DQT_MARKER  (0xDB) // define quantization table
#define JPEG_EXIF_DRI_MARKER  (0xDD) // define restart interoperability


#endif // IG_JPEG_MARKER_H
