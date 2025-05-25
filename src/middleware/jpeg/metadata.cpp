/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "metadata.h"
#include "middleware/jpeg/marker.h"
#include "middleware/jpeg/parser.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/encoding.h>


#define LOG_PRINT_SEGMENTS (1)


namespace fs = std::filesystem;



jpeg::Metadata jpeg::readMetadata(const fs::path& filePath)
{
    uint8_t buffer[64 * 1024];

    std::fstream ifs;

    ifs.exceptions(std::ios::badbit | std::ios::failbit);
    ifs.open(filePath, std::ios::in | std::ios::binary);

    ifs.exceptions(std::ios::badbit);
    ifs.read((char*)buffer, sizeof(buffer));

    ifs.exceptions(std::ios::badbit | std::ios::failbit);
    ifs.close();

#if PRJ_DEBUG && LOG_PRINT_SEGMENTS
    printf("\n\n%s\n", filePath.u8string().c_str());
#endif



    const uint8_t* p = buffer;
    const uint8_t* const end = buffer + sizeof(buffer);

    while (p < end)
    {
        // forward until segment mask prefix is found
        while ((*p != JPEG_MARKER_PREFIX) && (p < end)) { ++p; }
        if (p >= end) { break; }

        const jpeg::SegmentInfo info(p, end - p);

#if PRJ_DEBUG && LOG_PRINT_SEGMENTS
        if ((info.type() == SegmentType::APP0))
        {
            if (p > buffer) { printf("\n"); }

            std::string segmentTypeStringPretty = toString(info.type());
            if (info.isApp0Segment()) { segmentTypeStringPretty += "-JFIF"; }
            if (info.isApp0ExtensionSegment()) { segmentTypeStringPretty += "-JFXX"; }

            printf("@%04zx %s %zu 0x%04zx\n", (size_t)(p - buffer), segmentTypeStringPretty.c_str(), info.size(), info.size());
            if (info.size() > 0)
            {
                const size_t s = ((info.size() == SIZE_MAX) ? 128 : (2 + info.size())); // segment size
                const size_t d = sizeof(buffer) - (size_t)(p - buffer);                 // remaining data size
                const size_t pretty = 1024;

                size_t n = s;
                if (n > d) { n = d; }
                if (n > pretty) { n = pretty; }

                util::hexDump(p, n);
                if (n < s) { printf("...\n"); }
            }
        }
#endif // LOG_PRINT_SEGMENTS

        if (info.isApp0Segment())
        {
            const jpeg::App0Segment segment(p, end - p);

#if PRJ_DEBUG && LOG_PRINT_SEGMENTS
            printf("JFIF v%s, thumbnail: %ix%i, %s\n", segment.jfifVersion().toString().c_str(), (int)segment.thumbnailWidth(), (int)segment.thumbnailHeight(),
                   (segment.isValid() ? "valid" : "invalid"));
#endif // LOG_PRINT_SEGMENTS
        }


        p += 2 + info.size();

        if (info.type() == jpeg::SegmentType::SOS) { break; }
    }



    return Metadata();
}
