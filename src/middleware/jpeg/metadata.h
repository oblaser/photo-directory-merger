/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_METADATA_H
#define IG_JPEG_METADATA_H

#include <cstddef>
#include <cstdint>
#include <filesystem>


namespace jpeg {

class Metadata
{
public:
    Metadata() {}
    virtual ~Metadata() {}
};

Metadata readMetadata(const std::filesystem::path& filePath);

#ifdef _DEBUG
void analyseAndPrintMeta(const std::filesystem::path& filePath);
#endif

} // namespace jpeg


#endif // IG_JPEG_METADATA_H
