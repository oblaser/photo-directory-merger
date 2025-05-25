/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <vector>

#include "tiff-parser.h"



tiff::File::File(const uint8_t* data, size_t count)
    : m_validity(false), m_byteOrderBE(true)
{
    parse(data, count);
}

int tiff::File::parse(const uint8_t* data, size_t count)
{
    m_validity = false;

    if (count < 8) { return (-1); }

    const std::string bom((const char*)data, 2);

    if (bom == "MM") { m_byteOrderBE = true; }
    else if (bom == "II") { m_byteOrderBE = false; }
    else { return (-1); }

    if (0x002A != m_decode16(data + 2)) { return (-1); } // TIFF marker 0x002A = 42

    const uint32_t dirPointer = m_decode32(data + 4);

    m_validity = true;



    int cntTags = 0;

    if (dirPointer != 0)
    {
#warning "TODO"
        cntTags = -1;
    }

    return cntTags;
}

#include <stdexcept>
uint16_t tiff::File::m_decode16(const uint8_t* data) const
{
    uint16_t r = 0;

    if (m_byteOrderBE)
    {
        r = data[0];
        r <<= 8;
        r |= data[1];
    }
    else { throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" LE not implemented")); }

    return r;
}

uint32_t tiff::File::m_decode32(const uint8_t* data) const
{
    uint16_t r = 0;

    if (m_byteOrderBE) { throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" BE not implemented")); }
    else { throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" LE not implemented")); }

    return r;
}

uint64_t tiff::File::m_decode64(const uint8_t* data) const
{
    uint16_t r = 0;

    if (m_byteOrderBE) { throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" BE not implemented")); }
    else { throw std::runtime_error(__PRETTY_FUNCTION__ + std::string(" LE not implemented")); }

    return r;
}
