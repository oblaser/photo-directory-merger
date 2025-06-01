/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>

#include "metadata.h"
#include "middleware/jpeg/marker.h"
#include "middleware/jpeg/parser.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/encoding.h>


#define LOG_PRINT_SEGMENTS (0)


namespace fs = std::filesystem;



std::string jpeg::DateTime::toStringIso8601(time_t t)
{
    std::string r;

    constexpr size_t bufferSize = 50;
    char buffer[bufferSize];

    const struct std::tm* tm = std::localtime(&t);

    if (tm && (std::strftime(buffer, bufferSize, "%FT%T", tm) > 0)) { r = std::string(buffer); }
    else { r = '[' + std::to_string(t) + ']'; }

    return r;
}

std::string jpeg::DateTime::toStringPhodime(time_t t)
{
    std::string r;

    constexpr size_t bufferSize = 50;
    char buffer[bufferSize];

    const struct std::tm* tm = std::localtime(&t);

    static_assert(app::outFileDelimiter == '-', "adapt format string below");

    if (tm && (std::strftime(buffer, bufferSize, "%Y%m%d-%H%M%S", tm) > 0)) { r = std::string(buffer); }
    else { r = '[' + std::to_string(t) + ']'; }

    return r;
}



void jpeg::Metadata::m_parse(const uint8_t* data, size_t count)
{
    const uint8_t* p = data;
    const uint8_t* const end = data + count;
    size_t segmentIdx = 0;

    while (p < end)
    {
        // forward until segment mask prefix is found
        while ((*p != JPEG_MARKER_PREFIX) && (p < end)) { ++p; }
        if (p >= end) { break; }

        const jpeg::SegmentInfo info(p, end - p);

#if PRJ_DEBUG && LOG_PRINT_SEGMENTS
        if ((info.type() == SegmentType::APP1) && 0)
        {
            if (p > data) { printf("\n"); }

            std::string segmentTypeStringPretty = toString(info.type());
            if (info.isApp0Segment()) { segmentTypeStringPretty += "-JFIF"; }
            if (info.isApp0ExtensionSegment()) { segmentTypeStringPretty += "-JFXX"; }

            printf("@%04zx %s %zu 0x%04zx\n", (size_t)(p - data), segmentTypeStringPretty.c_str(), info.size(), info.size());
            if (info.size() > 0)
            {
                const size_t s = ((info.size() == SIZE_MAX) ? 128 : (2 + info.size())); // segment size
                const size_t d = count - (size_t)(p - data);                            // remaining data size
                const size_t pretty = ((info.type() == SegmentType::APP1) ? 1024 : 512);

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
            std::string versionString = segment.jfifVersion().toString();
            if (!segment.jfifVersion().isValid()) { versionString += "(invalid)"; }

            printf("JFIF v%s, thumbnail: %ix%i, %s\n", versionString.c_str(), (int)segment.thumbnailWidth(), (int)segment.thumbnailHeight(),
                   (segment.isValid() ? "valid" : "invalid"));
#endif // LOG_PRINT_SEGMENTS
        }
        else if (info.isApp1Segment())
        {
            const jpeg::App1Segment segment(p, end - p);

            m_tChanged = segment.tChanged();
            m_tDigitized = segment.tDigitized();
            m_tOriginal = segment.tOriginal();

#if PRJ_DEBUG && LOG_PRINT_SEGMENTS
            const std::string tChangedStr = DateTime::toStringIso8601(segment.tChanged());
            const std::string tOriginalStr = DateTime::toStringIso8601(segment.tOriginal());
            const std::string tDigitizedStr = DateTime::toStringIso8601(segment.tDigitized());
            printf("APP1 %s\n  c %s\n  o %s\n  d %s\n", (segment.isValid() ? "valid" : "invalid"), tChangedStr.c_str(), tOriginalStr.c_str(),
                   tDigitizedStr.c_str());
#if 0
            util::hexDump(p, 16);

            printf("TIFF:\n");
            const size_t count = end - p - 10;
            constexpr size_t pretty = 512;
            util::hexDump(p + 10, (count < pretty ? count : pretty));
#endif
#endif // LOG_PRINT_SEGMENTS
        }



        ++segmentIdx;
        p += 2 + info.size();

        if (info.type() == jpeg::SegmentType::SOS) { break; }
    }
}



jpeg::Metadata jpeg::readMetadata(const fs::path& filePath)
{
#if PRJ_DEBUG && 0
    uint8_t buffer[300];
#else
    uint8_t buffer[64 * 1024];
#endif

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



#if PRJ_DEBUG && 0
    {
        printf("\n\n\n");

        auto test = [](uint8_t maj, uint8_t min) {
            const jpeg::exif::Version v(maj, min);
            printf("%02x %02x -> %s, semver: %s\n", (int)maj, (int)min, v.toString().c_str(), v.toSemverString().c_str());
        };

        test(1, 0x02);
        test(1, 0x20);
        test(1, 0x21);
        test(1, 0x03);

        printf("\n\n\n");
    }
#endif // PRJ_DEBUG



    return Metadata(buffer, sizeof(buffer));
}
