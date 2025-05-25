/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>

#include "middleware/jpeg/marker.h"
#include "parser.h"

#include <omw/encoding.h>



#define JPEG_EXIF_TAG_VERSION          (0x9000)
#define JPEG_EXIF_TAG_MAKE             (0x010F)
#define JPEG_EXIF_TAG_MODEL            (0x0110)
#define JPEG_EXIF_TAG_ORIENTATION      (0x0112)
#define JPEG_EXIF_TAG_DATETIMEORIG     (0x9003)
#define JPEG_EXIF_TAG_FNUMBER          (0x829D)
#define JPEG_EXIF_TAG_EXPOSURETIME     (0x829A)
#define JPEG_EXIF_TAG_ISOSPEED         (0x8833)
#define JPEG_EXIF_TAG_METERINGMODE     (0x9207)
#define JPEG_EXIF_TAG_ARTIST           (0x013B)
#define JPEG_EXIF_TAG_COPYRIGHT        (0x8298)
#define JPEG_EXIF_TAG_USERCOMMENT      (0x9286)
#define JPEG_EXIF_TAG_IMAGEDESCRIPTION (0x010E)



jpeg::SegmentInfo::SegmentInfo(const uint8_t* data, size_t count)
    : m_type(SegmentType::none), m_size(0)
{
    if ((count >= 2) && (data[0] == JPEG_MARKER_PREFIX))
    {
        const uint8_t segmentMarker = data[1];

        if (segmentMarker == JPEG_SOI_MARKER) { m_type = SegmentType::SOI; }
        else if (segmentMarker == JPEG_SOS_MARKER) { m_type = SegmentType::SOS; }
        else if (segmentMarker == JPEG_EOI_MARKER) { m_type = SegmentType::EOI; }

        else if (segmentMarker == JPEG_JFIF_APP0_MARKER) { m_type = SegmentType::APP0; }

        else if (segmentMarker == JPEG_EXIF_APP1_MARKER) { m_type = SegmentType::APP1; }
        else if (segmentMarker == JPEG_EXIF_APP2_MARKER) { m_type = SegmentType::APP2; }

        else { m_type = SegmentType::unknown; }

        m_setSize(data, count);
    }
}

void jpeg::SegmentInfo::m_setSize(const uint8_t* data, size_t count)
{
    if (m_type == SegmentType::none) { m_size = 0; }
    else if (m_type == SegmentType::SOI) { m_size = 0; }
    else if (m_type == SegmentType::EOI) { m_size = 0; }
    else
    {
        if (count >= 4) { m_size = omw::bigEndian::decode_ui16(data + 2); }
    }
}



std::string jpeg::toString(const SegmentType& segmentType)
{
    std::string str;

    switch (segmentType)
    {
    case SegmentType::none:
        str = "none";
        break;

    case SegmentType::unknown:
        str = "unknown";
        break;

    case SegmentType::SOI:
        str = "SOI";
        break;

    case SegmentType::DHT:
        str = "DHT";
        break;

    case SegmentType::SOF:
        str = "SOF";
        break;

    case SegmentType::SOS:
        str = "SOS";
        break;

    case SegmentType::EOI:
        str = "EOI";
        break;



        // JFIF

    case SegmentType::APP0:
        str = "APP0";
        break;



        // EXIF

    case SegmentType::APP1:
        str = "APP1";
        break;

    case SegmentType::APP2:
        str = "APP2";
        break;

    case SegmentType::DQT:
        str = "DQT";
        break;

    case SegmentType::DRI:
        str = "DRI";
        break;
    }

    return str;
}
