/*
author          Oliver Blaser
date            24.05.2025
copyright       GPL-3.0 - Copyright (c) 2025 Oliver Blaser
*/

#ifndef IG_JPEG_TIFFPARSER_H
#define IG_JPEG_TIFFPARSER_H

#include <cstddef>
#include <cstdint>
#include <vector>


namespace tiff {

enum TAG_ID : uint16_t
{

    ID_MAKE = 0x010F,
    ID_MODEL = 0x0110,
    ID_SOFTWARE = 0x0131,
    ID_DATETIME = 0x0132, // TIFF: of image creation / Exif: of when the file was changed
    ID_HOSTCOMPUTER = 0x013C,
    ID_SUBIFD = 0x014A,

    // Exif
    ID_EXIFIFD = 0x8769,
    ID_GPSIFD = 0x8825,
    ID_EXIFINTEROPIFD = 0xA005,

    // Exif IFD
    ID_EXIFVERSION = 0x9000,
    ID_DATETIMEORIGINAL = 0x9003,  // of original data generation
    ID_DATETIMEDIGITIZED = 0x9004, // of digital data generation
};

bool isNestedIfd(uint16_t tagId);

enum VALUE_TYPE : uint16_t
{
    TYPE_BYTE = 0x0001,      // unsigned 8bit
    TYPE_ASCII = 0x0002,     // 7bit ASCII string, null terminated (count includes
                             // the null terminator, but not potential padding bytes)
    TYPE_SHORT = 0x0003,     // unsigned 16bit
    TYPE_LONG = 0x0004,      // unsigned 32bit
    TYPE_RATIONAL = 0x0005,  // two LONGs, the first is the numerator, the second the denominator (frac = num/denom)
    TYPE_SBYTE = 0x0006,     // signed 8bit
    TYPE_UNDEFINED = 0x0007, // anything encoded in 8bit words
    TYPE_SSHORT = 0x0008,    // signed 16bit
    TYPE_SLONG = 0x0009,     // signed 32bit
    TYPE_SRATIONAL = 0x000A, // two SLONGs, see `TYPE_RATIONAL`
    TYPE_FLOAT = 0x000B,     // IEEE single precision floating point number
    TYPE_DOUBLE = 0x000C,    // IEEE double precision floating point number
    TYPE_SUBIFD = 0x000D,
};

enum class ByteOrder
{
    undefined,
    be, // big endian, Motorola
    le, // little endian, Intel
};

class Encoding
{
public:
    Encoding()
        : m_byteOrder(ByteOrder::undefined)
    {}

    Encoding(const ByteOrder& byteOrder)
        : m_byteOrder(byteOrder)
    {}

    virtual ~Encoding() {}

    const ByteOrder& byteOrder() const { return m_byteOrder; }

protected:
    void m_setByteOrder(const ByteOrder& byteOrder) { m_byteOrder = byteOrder; }

    uint16_t m_decode16(const uint8_t* data) const;
    uint32_t m_decode32(const uint8_t* data) const;
    uint64_t m_decode64(const uint8_t* data) const;

private:
    ByteOrder m_byteOrder;
};

class Common
{
public:
    Common() {}
    virtual ~Common() {}

    virtual void clear() = 0;
    virtual bool isValid() const = 0;
};

class Directory;

class DirectoryContainer : public Encoding
{
public:
    DirectoryContainer()
        : Encoding(), m_validity(false), m_directories()
    {}

    explicit DirectoryContainer(const ByteOrder& byteOrder)
        : Encoding(byteOrder), m_validity(false), m_directories()
    {}

    virtual ~DirectoryContainer() {}

    const std::vector<Directory>& directories() const { return m_directories; }

protected:
    bool m_validity;

    int m_parseIfds(const uint8_t* data, size_t count, uint32_t offs);
    void m_clearDirectories() { m_directories.clear(); }

private:
    std::vector<Directory> m_directories;
};

class Tag : public Common,
            public DirectoryContainer
{
public:
    static size_t sizeOfType(uint16_t valueType);
    static bool valueIsOffset(uint16_t id, uint16_t valueType, uint32_t valueCount);

public:
    Tag() = delete;

    explicit Tag(ByteOrder byteOrder)
        : Common(), DirectoryContainer(byteOrder), m_id(), m_valueType(), m_valueCount(), m_value(), m_data()
    {
        this->clear();
    }

    Tag(const uint8_t* data, size_t count, uint32_t offs, ByteOrder byteOrder)
        : Common(), DirectoryContainer(byteOrder), m_id(), m_valueType(), m_valueCount(), m_value(), m_data()
    {
        parse(data, count, offs);
    }

    virtual ~Tag() {}

    int parse(const uint8_t* data, size_t count, uint32_t offs);

    virtual void clear();

    uint16_t id() const { return m_id; }
    uint16_t valueType() const { return m_valueType; }
    uint32_t valueCount() const { return m_valueCount; }
    uint32_t value() const { return m_value; }
    const std::vector<uint8_t>& data() const { return m_data; }

    bool valueIsOffset() const { return valueIsOffset(m_id, m_valueType, m_valueCount); }

    bool isDirectory() const { return tiff::isNestedIfd(m_id); }

    virtual bool isValid() const { return m_validity; }

private:
    uint16_t m_id;
    uint16_t m_valueType;
    uint32_t m_valueCount;
    uint32_t m_value; // the actual value or an offset to the data/IFD
    std::vector<uint8_t> m_data;
};

class Directory : public Common,
                  public Encoding
{
public:
    Directory() = delete;

    explicit Directory(ByteOrder byteOrder)
        : Common(), Encoding(byteOrder), m_validity(false), m_offs(0), m_tagCount(), m_next(), m_tags()
    {
        this->clear();
    }

    Directory(const uint8_t* data, size_t count, uint32_t offs, ByteOrder byteOrder)
        : Common(), Encoding(byteOrder), m_validity(false), m_offs(0), m_tagCount(), m_next(), m_tags()
    {
        parse(data, count, offs);
    }

    virtual ~Directory() {}

    int parse(const uint8_t* data, size_t count, uint32_t offs);

    virtual void clear();

    uint32_t offs() const { return m_offs; }
    uint16_t tagCount() const { return m_tagCount; }
    uint32_t next() const { return m_next; }
    const std::vector<Tag>& tags() const { return m_tags; }

    virtual bool isValid() const { return m_validity; }

protected:
    bool m_validity;

private:
    uint32_t m_offs;
    uint16_t m_tagCount;
    uint32_t m_next;
    std::vector<Tag> m_tags;
};

class File : public Common,
             public DirectoryContainer
{
public:
    File()
        : Common(), DirectoryContainer()
    {
        this->clear();
    }

    File(const uint8_t* data, size_t count)
        : Common(), DirectoryContainer()
    {
        parse(data, count);
    }

    virtual ~File() {}

    /**
     * Returns the number of read tags even if not all tag values could have been read from `data`.
     * In this case the validity is set to false.
     *
     * @param data Pointer to the buffer holding the TIFF file data
     * @param count Number of data bytes
     * @return Number of read tags on success, -1 if not all tags could have been read from `data`
     */
    int parse(const uint8_t* data, size_t count);

    virtual void clear();

    virtual bool isValid() const { return m_validity; }
};

} // namespace tiff


#endif // IG_JPEG_TIFFPARSER_H
