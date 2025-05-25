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

class Version
{
public:
    Version()
        : m_major(-1), m_minor(-1)
    {}

    Version(int32_t major, int32_t minor)
        : m_major(major), m_minor(minor)
    {}

    virtual ~Version() {}

    int32_t major() const { return m_major; }
    int32_t minor() const { return m_minor; }

    bool isValid() const { return ((m_major >= 0) && (m_minor >= 0)); }

    std::string toString() const;

private:
    int32_t m_major;
    int32_t m_minor;
};

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
        : m_type(SegmentType::none), m_size(0), m_app0Identifier()
    {}

    SegmentInfo(const SegmentType& type, size_t size)
        : m_type(type), m_size(size), m_app0Identifier()
    {}

    SegmentInfo(const uint8_t* data, size_t count);

    virtual ~SegmentInfo() {}

    const SegmentType& type() const { return m_type; }

    /**
     * @return Segment data size
     */
    size_t size() const { return m_size; }

    /**
     * @return Segment size
     */
    size_t segmentSize() const { return (2 + m_size); }

    bool isApp0Segment() const;
    bool isApp0ExtensionSegment() const;

private:
    SegmentType m_type;
    size_t m_size;
    std::string m_app0Identifier;

    void m_setSize(const uint8_t* data, size_t count);
};



class Segment
{
public:
    Segment()
        : m_info(), m_validity(false)
    {}

    Segment(const uint8_t* data, size_t count)
        : m_info(data, count), m_validity(false)
    {}

    virtual ~Segment() {}

    const SegmentInfo& info() const { return m_info; }

    bool isValid() const { return m_validity; }

protected:
    SegmentInfo m_info;
    bool m_validity;
};

class GenericSegment : public Segment
{
public:
    GenericSegment()
        : Segment(), m_data()
    {}

    GenericSegment(const uint8_t* data, size_t count)
        : Segment(data, count), m_data(data + 2, data + count)
    {
        m_validity = true;
    }

    GenericSegment(const uint8_t* data, size_t count, bool validity)
        : Segment(data, count), m_data(data + 2, data + count)
    {
        m_validity = validity;
    }

    virtual ~GenericSegment() {}

    const std::vector<uint8_t>& data() const { return m_data; }

protected:
    std::vector<uint8_t> m_data;
};

class App0Segment : public Segment
{
public:
    App0Segment() = delete;

    App0Segment(const uint8_t* data, size_t count)
        : Segment(data, count), m_jfifVersion(), m_thumbnailWidth(0), m_thumbnailHeight(0)
    {
        if (m_info.isApp0Segment()) { m_parse(data, count); }
    }

    virtual ~App0Segment() {}

    const Version& jfifVersion() const { return m_jfifVersion; }

    uint8_t thumbnailWidth() const { return m_thumbnailWidth; }
    uint8_t thumbnailHeight() const { return m_thumbnailHeight; }

private:
    Version m_jfifVersion;
    uint8_t m_thumbnailWidth;
    uint8_t m_thumbnailHeight;

    void m_parse(const uint8_t* data, size_t count);
};

class App1Segment : public Segment
{};

class App2Segment : public Segment
{};



std::string toString(const SegmentType& segmentType);

} // namespace jpeg


#endif // IG_JPEG_PARSER_H
