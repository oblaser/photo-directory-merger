/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_METADATA_H
#define IG_JPEG_METADATA_H

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <string>


namespace app {
static constexpr char outFileDelimiter = '-';
static constexpr char outFileDelimiter_opt = '_';
}


namespace jpeg {

class DateTime
{
public:
    static std::string toStringIso8601(time_t t);
    static std::string toStringPhodime(time_t t);

public:
    DateTime()
        : m_t(0)
    {}

    DateTime(time_t t)
        : m_t(t)
    {}

    virtual ~DateTime() {}

    time_t unixTime() const { return m_t; }

    std::string toStringIso8601() const { return toStringIso8601(m_t); }
    std::string toStringPhodime() const { return toStringPhodime(m_t); }

protected:
    time_t m_t;
};

inline bool operator==(const jpeg::DateTime& a, const jpeg::DateTime& b) { return (a.unixTime() == b.unixTime()); }
inline bool operator!=(const jpeg::DateTime& a, const jpeg::DateTime& b) { return !(a == b); }
inline bool operator<(const jpeg::DateTime& a, const jpeg::DateTime& b) { return (a.unixTime() < b.unixTime()); }
inline bool operator>(const jpeg::DateTime& a, const jpeg::DateTime& b) { return (b < a); }
inline bool operator<=(const jpeg::DateTime& a, const jpeg::DateTime& b) { return !(a > b); }
inline bool operator>=(const jpeg::DateTime& a, const jpeg::DateTime& b) { return !(a < b); }


class Metadata
{
public:
    Metadata()
        : m_tChanged(-1), m_tDigitized(-410227200), m_tOriginal(INT32_MIN)
    {}

    Metadata(const uint8_t* data, size_t count)
        : m_tChanged(-1), m_tDigitized(-410227200), m_tOriginal(INT32_MIN)
    {
        m_parse(data, count);
    }

    virtual ~Metadata() {}

    const DateTime& tChanged() const { return m_tChanged; }
    const DateTime& tDigitized() const { return m_tDigitized; }
    const DateTime& tOriginal() const { return m_tOriginal; }

    void m_parse(const uint8_t* data, size_t count);

private:
    DateTime m_tChanged;
    DateTime m_tDigitized;
    DateTime m_tOriginal;
};

Metadata readMetadata(const std::filesystem::path& filePath);

} // namespace jpeg


#endif // IG_JPEG_METADATA_H
