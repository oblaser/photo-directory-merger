/*
author          Oliver Blaser
date            10.01.2023
copyright       GNU GPLv3 - Copyright (c) 2023 Oliver Blaser
*/

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "util.h"

#include <omw/omw.h>


namespace
{
}



OMW_CONSTEXPR_ON_STDSTRING std::string omw_::rmLeadingZeros(const std::string& str)
{
    std::string r = str;

    if (r.length() > 0)
    {
        while ((r[0] == '0') && (r.length() > 1)) r.erase(0, 1);
    }

    return r;
}

void omw_::rmLeadingZeros(std::string& str)
{
    if (str.length() > 0)
    {
        while ((str[0] == '0') && (str.length() > 1)) str.erase(0, 1);
    }
}

omw::stringVector_t omw_::split(const std::string& str, char delimiter, omw::stringVector_t::size_type maxTokenCount)
{
    omw::stringVector_t r(0);

    if (maxTokenCount > 0)
    {
        const omw::stringVector_t::size_type n = maxTokenCount - 1;
        omw::string::size_type pos = 0;

        while (pos < omw::string::npos)
        {
            if (r.size() < n)
            {
                const omw::string::size_type end = str.find(delimiter, pos);
                r.push_back(str.substr(pos, end - pos));
                pos = end;
                if (pos < omw::string::npos) ++pos;
            }
            else
            {
                r.push_back(str.substr(pos));
                pos = omw::string::npos;
            }
        }
    }

    return r;
}

omw::stringVector_t omw_::splitLen(const std::string& str, omw::string::size_type tokenLength, omw::stringVector_t::size_type maxTokenCount)
{
    omw::stringVector_t r(0);

    if (maxTokenCount > 0)
    {
        const omw::string::size_type len = str.length();
        omw::string::size_type pos = 0;

        for (omw::stringVector_t::size_type iToken = 0; (iToken < maxTokenCount) && (pos < len); ++iToken)
        {
            r.push_back(str.substr(pos, tokenLength));
            pos += tokenLength;
        }

        if (pos < len)
        {
            r[r.size() - 1] += str.substr(pos);
        }
    }

    return r;
}
