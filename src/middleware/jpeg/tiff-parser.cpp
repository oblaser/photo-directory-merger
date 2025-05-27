/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "tiff-parser.h"


#define LOG_EN (0)


#if (defined(_DEBUG) && LOG_EN) || 1
#include <stdio.h>
#define LOG_ERR_PARSE_OUTOFRANGE(_type, _offs) printf("\033[91mcan't parse " _type " at 0x%04x, out of range\033[39m\n", (uint32_t)(_offs))
#define LOG_ERR_PARSE_INSUFFICIENT(_type, _offs, _n) \
    printf("\033[91mcan't parse " _type " at [0x%04x 0x%04x), insufficient data\033[39m\n", (uint32_t)(_offs), (uint32_t)((_offs) + (_n)))
#else
#define LOG_ERR_PARSE_OUTOFRANGE(...)   (void)0
#define LOG_ERR_PARSE_INSUFFICIENT(...) (void)0
#endif



uint16_t tiff::Common::m_decode16(const uint8_t* data) const
{
    uint16_t r = 0;

    switch (m_byteOrder)
    {
    case ByteOrder::undefined:
        throw std::runtime_error("undefined byte order");
        break;

    case ByteOrder::be:
        r = data[0];
        r <<= 8;
        r |= data[1];
        break;

    case ByteOrder::le:
        r = data[1];
        r <<= 8;
        r |= data[0];
        break;
    }

    return r;
}

uint32_t tiff::Common::m_decode32(const uint8_t* data) const
{
    uint32_t r = 0;

    switch (m_byteOrder)
    {
    case ByteOrder::undefined:
        throw std::runtime_error("undefined byte order");
        break;

    case ByteOrder::be:
        r = data[0];
        r <<= 8;
        r |= data[1];
        r <<= 8;
        r |= data[2];
        r <<= 8;
        r |= data[3];
        break;

    case ByteOrder::le:
        r = data[3];
        r <<= 8;
        r |= data[2];
        r <<= 8;
        r |= data[1];
        r <<= 8;
        r |= data[0];
        break;
    }

    return r;
}

uint64_t tiff::Common::m_decode64(const uint8_t* data) const
{
    uint64_t r = 0;

    switch (m_byteOrder)
    {
    case ByteOrder::undefined:
        throw std::runtime_error("undefined byte order");
        break;

    case ByteOrder::be:
        r = data[0];
        r <<= 8;
        r |= data[1];
        r <<= 8;
        r |= data[2];
        r <<= 8;
        r |= data[3];
        r <<= 8;
        r |= data[4];
        r <<= 8;
        r |= data[5];
        r <<= 8;
        r |= data[6];
        r <<= 8;
        r |= data[7];
        break;

    case ByteOrder::le:
        r = data[7];
        r <<= 8;
        r |= data[6];
        r <<= 8;
        r |= data[5];
        r <<= 8;
        r |= data[4];
        r <<= 8;
        r |= data[3];
        r <<= 8;
        r |= data[2];
        r <<= 8;
        r |= data[1];
        r <<= 8;
        r |= data[0];
        break;
    }

    return r;
}



size_t tiff::Tag::sizeOfType(uint16_t valueType)
{
    size_t size;

    switch (valueType)
    {
    case TYPE_BYTE:
    case TYPE_ASCII:
    case TYPE_SBYTE:
    case TYPE_UNDEFINED:
        size = 1;
        break;

    case TYPE_SHORT:
    case TYPE_SSHORT:
        size = 2;
        break;

    case TYPE_LONG:
    case TYPE_SLONG:
    case TYPE_FLOAT:
    case TYPE_SUBIFD:
        size = 4;
        break;

    case TYPE_RATIONAL:
    case TYPE_SRATIONAL:
    case TYPE_DOUBLE:
        size = 8;
        break;

    default:
        size = 0;
        break;
    }

    return size;
}

bool tiff::Tag::valueIsOffset(uint16_t id, uint16_t valueType, uint32_t valueCount)
{
    return ((id == ID_SUBIFD) || (id == ID_EXIFIFD) || (id == ID_GPSIFD) || (id == ID_EXIFINTEROPIFD) || ((sizeOfType(valueType) * valueCount) > 4));
}

int tiff::Tag::parse(const uint8_t* data, size_t count, uint32_t offs)
{
    this->clear();

    if ((count - offs) < 12)
    {
        LOG_ERR_PARSE_OUTOFRANGE("TAG", offs);
        return (-1);
    }

    m_validity = true;

    m_id = m_decode16(data + offs + 0);
    m_valueType = m_decode16(data + offs + 2);
    m_valueCount = m_decode32(data + offs + 4);
    m_value = m_decode32(data + offs + 8);

    if (0 == sizeOfType(m_valueType))
    {
#if defined(_DEBUG) && LOG_EN
        printf("\033[91mcan't parse tag with unknown value type 0x%04x\033[39m\n", (int)m_valueType);
#endif
        m_validity = false;
        return (-1);
    }


    size_t n;
    size_t valueOffs;

    if (valueIsOffset())
    {
        valueOffs = m_value; // value holds the data pointer/offset

        if (valueOffs < count)
        {
            n = m_valueCount * sizeOfType(m_valueType);

            if ((count - valueOffs) < n)
            {
                LOG_ERR_PARSE_INSUFFICIENT("TAG_VALUE", valueOffs, n);

                m_validity = false;
                n = (count - valueOffs);
            }
        }
        else
        {
            LOG_ERR_PARSE_OUTOFRANGE("TAG_VALUE", m_value);

            m_validity = false;

            valueOffs = offs + 8;
            n = 0;
        }
    }
    else
    {
        valueOffs = offs + 8;
        n = 4;
    }

    const uint8_t* const begin = data + valueOffs;
    const uint8_t* const end = begin + n;
    m_data.assign(begin, end);
    const int nBytesRead = (int)n;

    return nBytesRead;
}

void tiff::Tag::clear()
{
    m_validity = false;

    m_id = 0;
    m_valueType = 0;
    m_valueCount = 0;
    m_value = 0;

    m_data.clear();
    m_data.shrink_to_fit();
}



int tiff::Directory::parse(const uint8_t* data, size_t count, uint32_t offs)
{
    this->clear();

    if ((count - offs) < 2)
    {
        LOG_ERR_PARSE_OUTOFRANGE("IFD", offs);
        return (-1);
    }

    m_tagCount = m_decode16(data + offs + 0);
    const size_t n = m_tagCount;

    if ((count - offs) < (2 + (n * 12) + 4))
    {
        LOG_ERR_PARSE_OUTOFRANGE("IFD", offs);
        return (-1);
    }

    m_next = m_decode32(data + offs + 2 + (n * 12));

    m_validity = true;

    int cntTags = 0;
    Tag tag(this->byteOrder());

    for (size_t i = 0; i < n; ++i)
    {
        const uint32_t tagOffs = offs + 2 + (i * 12);

        if (tagOffs < count)
        {
            const int res = tag.parse(data, count, tagOffs);

#if defined(_DEBUG) && LOG_EN
            printf("tag 0x%04x, ID: 0x%04x, value: 0x%08x %u, res: %i\n", tagOffs, tag.id(), tag.value(), tag.value(), res);
#endif

            if (res > 0) { ++cntTags; }
            if (!tag.isValid()) { m_validity = false; }

            m_tags.push_back(tag);
        }
#if defined(_DEBUG) && LOG_EN
        else { printf("\033[91mtag #%zu at 0x%04x is out of accessible range\033[39m\n", i, tagOffs); }
#endif
    }

    return cntTags;
}

void tiff::Directory::clear()
{
    m_validity = false;

    m_tagCount = 0;
    m_next = 0;

    m_tags.clear();
}



int tiff::File::parse(const uint8_t* data, size_t count)
{
    this->clear();

    if (count < 8) { return (-1); }

    const std::string bom((const char*)data, 2);

    if (bom == "MM") { m_setByteOrder(ByteOrder::be); }
    else if (bom == "II") { m_setByteOrder(ByteOrder::le); }
    else { return (-1); }

    if (0x002A != m_decode16(data + 2)) { return (-1); } // TIFF marker 0x002A = 42

    uint32_t offs = m_decode32(data + 4);

    m_validity = true;



    int cntTags = 0;

    if (offs != 0)
    {
        Directory ifd(this->byteOrder());

        do {
            if (offs >= count)
            {
                LOG_ERR_PARSE_OUTOFRANGE("IFD", offs);
                m_validity = false;
                break;
            }

            const int res = ifd.parse(data, count, offs);

#if defined(_DEBUG) && LOG_EN
            printf("IFD 0x%04x, next: 0x%04x, res: %i\n", offs, ifd.next(), res);
#endif

            if (res > 0) { cntTags += res; }
            if (!ifd.isValid()) { m_validity = false; }

            m_directories.push_back(ifd);

            offs = ifd.next();
        }
        while (offs);
    }

    return cntTags;
}

void tiff::File::clear()
{
    m_validity = false;

    m_directories.clear();
}
