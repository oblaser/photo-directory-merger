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

#include <omw/encoding.h>


namespace fs = std::filesystem;



static void printAllSegments(const uint8_t* data, size_t count);



jpeg::Metadata jpeg::readMetadata(const fs::path& filePath)
{
    uint8_t buffer[512 * 1024];

    std::fstream ifs;
    ifs.exceptions(std::ios::badbit | std::ios::failbit);
    ifs.open(filePath, std::ios::in | std::ios::binary);
    ifs.read((char*)buffer, sizeof(buffer));
    ifs.close();

    printf("\n\n%s\n", filePath.u8string().c_str());
    printAllSegments(buffer, sizeof(buffer));

    return Metadata();
}

#ifdef _DEBUG
void jpeg::analyseAndPrintMeta(const fs::path& filePath)
{
    const auto data = readMetadata(filePath);

    // ...
}
#endif



void printAllSegments(const uint8_t* data, size_t count)
{
    const uint8_t* p = data;
    const uint8_t* const end = data + count;

    while (p < end)
    {
        // forward until segment mask prefix is found
        while ((*p != JPEG_MARKER_PREFIX) && (p < end)) { ++p; }
        if (p >= end) { break; }

        const jpeg::SegmentInfo info(p, end - p);

        if (p > data) { printf("\n"); }
        printf("@%04zx %s %zu 0x%04zx\n", (size_t)(p - data), toString(info.type()).c_str(), info.size(), info.size());
        if (info.size() > 0)
        {
            const size_t s = ((info.size() == SIZE_MAX) ? 128 : (2 + info.size())); // segment size
            const size_t d = count - (size_t)(p - data);                            // remaining data size
            const size_t pretty = 1024;

            size_t n = s;
            if (n > d) { n = d; }
            if (n > pretty) { n = pretty; }

            util::hexDump(p, n);
            if (n < s) { printf("...\n"); }
        }

        p += 2 + info.size();

        if (info.type() == jpeg::SegmentType::SOS) { break; }
    }
}
