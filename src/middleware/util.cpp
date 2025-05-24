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



OMW_STDSTRING_CONSTEXPR std::string omw_::rmLeadingZeros(const std::string& str)
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
