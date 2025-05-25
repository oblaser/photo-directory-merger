/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_TIFFPARSER_H
#define IG_JPEG_TIFFPARSER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


namespace tiff {

class Tag
{
public:
    Tag() {}

    virtual ~Tag() {}
};

class Directory
{
public:
    Directory() {}

    virtual ~Directory() {}
};

class File
{
public:
    File()
        : m_validity(false), m_byteOrderBE(true)
    {}

    File(const uint8_t* data, size_t count);

    virtual ~File() {}

    /**
     * Returns the number of read tags even if not all tag values could be read from `data`.
     * In this case the validity is set to false.
     *
     * @param data Pointer to the buffer holding the TIFF file data
     * @param count Number of data bytes
     * @return Number of read tags on success, -1 if not all tags could be read from `data`
     */
    int parse(const uint8_t* data, size_t count);

    bool isValid() const { return m_validity; }

protected:
    bool m_validity;
    bool m_byteOrderBE;

    uint16_t m_decode16(const uint8_t* data) const;
    uint32_t m_decode32(const uint8_t* data) const;
    uint64_t m_decode64(const uint8_t* data) const;
};

} // namespace tiff


#endif // IG_JPEG_TIFFPARSER_H
