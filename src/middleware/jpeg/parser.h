/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_PARSER_H
#define IG_JPEG_PARSER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


namespace jpeg {

enum class SegmentType
{
    none,
    unknown,

    SOI,
    DHT,
    SOF,
    SOS,
    EOI,

    // JFIF
    APP0,

    // EXIF
    APP1,
    APP2,
    DQT,
    DRI,
};

class SegmentInfo
{
public:
    SegmentInfo()
        : m_type(SegmentType::none), m_size(0)
    {}

    SegmentInfo(const SegmentType& type, size_t size)
        : m_type(type), m_size(size)
    {}

    SegmentInfo(const uint8_t* data, size_t count);

    virtual ~SegmentInfo() {}

    const SegmentType& type() const { return m_type; }
    size_t size() const { return m_size; }

private:
    SegmentType m_type;
    size_t m_size;

    void m_setSize(const uint8_t* data, size_t count);
};

class GenericSegment
{
public:
    GenericSegment()
        : m_info(), m_data()
    {}

    GenericSegment(const uint8_t* data, size_t count)
        : m_info(data, count), m_data(data + 2, data + count)
    {}

    virtual ~GenericSegment() {}

    const SegmentInfo& info() const { return m_info; }
    const std::vector<uint8_t>& data() const { return m_data; }

private:
    SegmentInfo m_info;
    std::vector<uint8_t> m_data;
};

std::string toString(const SegmentType& segmentType);

} // namespace jpeg


#endif // IG_JPEG_PARSER_H
