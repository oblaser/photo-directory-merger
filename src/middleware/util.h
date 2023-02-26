/*
author          Oliver Blaser
date            26.02.2023
copyright       GNU GPLv3 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MDW_UTIL_H
#define IG_MDW_UTIL_H

#include <cstddef>
#include <cstdint>


namespace util
{
    class FileCounter
    {
    public:
        using counter_type = size_t;

    public:
        FileCounter() : m_total(0), m_copied(0) {}
        virtual ~FileCounter() {}

        FileCounter& add(counter_type total, counter_type copied) { m_total += total; m_copied += copied; return (*this); }
        FileCounter& add(const FileCounter& other) { return add(other.total(), other.copied()); }
        FileCounter& addTotal(counter_type value = 1) { m_total += value; return (*this); }
        FileCounter& addCopied(counter_type value = 1) { m_copied += value; return (*this); }

        const counter_type& total() const { return m_total; }
        const counter_type& copied() const { return m_copied; }

    private:
        counter_type m_total;
        counter_type m_copied;
    };

    class ResultCounter
    {
    public:
        using counter_type = size_t;

    public:
        ResultCounter() : m_e(0), m_w(0) {}
        ResultCounter(counter_type e, counter_type w = 0) : m_e(e), m_w(w) {}
        virtual ~ResultCounter() {}

        counter_type errors() const { return m_e; };
        counter_type warnings() const { return m_w; };

        void incErrors() { ++m_e; }
        void incWarnings() { ++m_w; }

    private:
        counter_type m_e;
        counter_type m_w;
    };
}



#include <omw/omw.h>
#include <omw/string.h>
namespace omw_
{
#if (OMW_VERSION_ID <= /*OMW_VERSION_ID_0_2_1_ALPHA_1*/ 3)

#if OMW_CPPSTD >= OMW_CPPSTD_20
#define OMW_CONSTEXPR_ON_STDSTRING constexpr
#else
#define OMW_CONSTEXPR_ON_STDSTRING 
#endif

    OMW_CONSTEXPR_ON_STDSTRING std::string rmLeadingZeros(const std::string& str);
    void rmLeadingZeros(std::string& str);

    inline OMW_CONSTEXPR_ON_STDSTRING bool contains(const std::string& str, char ch) { return (str.find(ch) != std::string::npos); }
    inline OMW_CONSTEXPR_ON_STDSTRING bool contains(const std::string& str, const char* s) { return (str.find(s) != std::string::npos); }
    inline OMW_CONSTEXPR_ON_STDSTRING bool contains(const std::string& str, const std::string& s) { return (str.find(s) != std::string::npos); }
    inline OMW_CONSTEXPR_ON_STDSTRING bool contains(const std::string& str, const std::string_view& sv) { return (str.find(sv) != std::string::npos); }

    omw::stringVector_t split(const std::string& str, char delimiter, omw::stringVector_t::size_type maxTokenCount = omw::stringVector_npos);
    omw::stringVector_t splitLen(const std::string& str, omw::string::size_type tokenLength, omw::stringVector_t::size_type maxTokenCount = omw::stringVector_npos);

#endif // OMW_VERSION_ID
}


#endif // IG_MDW_UTIL_H
