/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <string>

#include "middleware/jpeg/marker.h"
#include "middleware/jpeg/tiff-parser.h"
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



std::string jpeg::Version::toString() const
{
#warning "is this JFIF version parsing ok?"

    std::string str = std::to_string(m_major) + '.';

    if (m_minor < 10) { str += '0'; }
    str += std::to_string(m_minor);

    return str;
}



#if 0
jpeg::exif::Version::Version(int32_t major, int32_t minor_revision)
    : m_semver(-1, -1, -1)
{
    if ((major >= 0) && (minor_revision >= 0))
    {
#warning "is this Exif version parsing ok?"
#if 1
        int32_t minor = minor_revision;
        int32_t revision = 0;

        if (minor_revision > 9)
        {
            minor = minor_revision / 10;
            revision = minor_revision - 10 * minor;
        }
#else
        const int32_t minor = ((minor_revision & 0xF0) >> 4);
        const int32_t revision = (minor_revision & 0x0F);
#endif

        m_semver.set(major, minor, revision);
    }
}

std::string jpeg::exif::Version::toString() const
{
    std::string str = std::to_string(major());

    str += '.';

    str += std::to_string(minor());
    if (revision() > 0) { str += std::to_string(revision()); }

    return str;
}
#endif



jpeg::SegmentInfo::SegmentInfo(const uint8_t* data, size_t count)
    : m_type(SegmentType::none), m_size(0), m_app0Identifier()
{
    if ((count >= 2) && (data[0] == JPEG_MARKER_PREFIX))
    {
        const uint8_t segmentMarker = data[1];

        if (segmentMarker == JPEG_SOI_MARKER) { m_type = SegmentType::SOI; }
        else if (segmentMarker == JPEG_SOS_MARKER) { m_type = SegmentType::SOS; }
        else if (segmentMarker == JPEG_EOI_MARKER) { m_type = SegmentType::EOI; }

        else if (segmentMarker == JPEG_JFIF_APP0_MARKER)
        {
            m_type = SegmentType::APP0;
            if ((count >= 9) && (data[8] == 0)) { m_app0Identifier = std::string((const char*)(data + 4)); }
        }

        else if (segmentMarker == JPEG_EXIF_APP1_MARKER) { m_type = SegmentType::APP1; }
        else if (segmentMarker == JPEG_EXIF_APP2_MARKER) { m_type = SegmentType::APP2; }

        else { m_type = SegmentType::unknown; }

        m_setSize(data, count);
    }
}

bool jpeg::SegmentInfo::isApp0Segment() const { return ((m_type == SegmentType::APP0) && (m_app0Identifier == "JFIF")); }
bool jpeg::SegmentInfo::isApp0ExtensionSegment() const { return ((m_type == SegmentType::APP0) && (m_app0Identifier == "JFXX")); }

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



void jpeg::App0Segment::m_parse(const uint8_t* data, size_t count)
{
    auto specSize = [](size_t nThumbPixels) { return (size_t)((size_t)18 + (size_t)3 * nThumbPixels); };



    // clamp byte count to segment size
    if (m_info.segmentSize() < count) { count = m_info.segmentSize(); }

    size_t n = 0;

    if (count >= specSize(0))
    {
        m_jfifVersion = Version(data[9], data[10]);

        m_thumbnailWidth = data[16];
        m_thumbnailHeight = data[17];

        n = (size_t)m_thumbnailWidth * (size_t)m_thumbnailHeight;
    }

    if ((count >= specSize(n)) && (count == m_info.segmentSize()))
    {
        // uncompressed 24bit RGB thumbnail image data
        // data[18] is R0
        // data[19] is G0
        // data[20] is B0
        // ...
        // data[18 + 3*(n-1)] is Rn-1
        // data[19 + 3*(n-1)] is Gn-1
        // data[20 + 3*(n-1)] is Bn-1

        m_validity = true;

        if (count > specSize(n))
        {
            // this is actually a violation to the JFIF spec
            m_validity = false;

            if (count == (specSize(n) + 4))
            {
                const std::string tmp((const char*)data + specSize(n), 4);

                if (tmp == "AMPF") { m_validity = true; } // Apple Multi Picture Format
            }
        }
    }
    else { m_validity = false; }
}

void jpeg::App1Segment::m_parse(const uint8_t* data, size_t count)
{
    // clamp byte count to segment size
    if (m_info.segmentSize() < count) { count = m_info.segmentSize(); }



    std::string identifier;

    if ((count >= 10) && (data[8] == 0) && (data[9] == 0)) { identifier = (const char*)data + 4; }

    if ((count == m_info.segmentSize()) && (identifier == "Exif"))
    {
        tiff::File tiff;

        if (tiff.parse(data + 10, count - 10) < 0) { printf("\033[91mTIFF parser error\033[39m\n"); }

        // version
        // make
        // model
        // datetime original
    }
    else { m_validity = false; }
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
