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


namespace {}



//======================================================================================================================
// hex dump

#include <ctype.h>
#include <stdio.h>

/**
 * @brief Converts the data buffer bytes to printable characters.
 *
 * Writes 16 characters and a terminating null to `buffer`.
 *
 * @param buffer Destination string buffer
 * @param p First element to parse, points into the source data buffer
 * @param end First element after the source data buffer
 */
static void hexDump_dataToString(char* buffer, const uint8_t* p, const uint8_t* end)
{
    size_t i = 0;

    while ((i < 16) && (p < end))
    {
        const char c = (char)(*p);

        if (isprint(c)) { buffer[i] = c; }
        else { buffer[i] = '.'; }

        ++p;
        ++i;
    }

    while (i < 16)
    {
        buffer[i] = ' ';
        ++i;
    }

    buffer[i] = 0;
}

void util::hexDump(const uint8_t* data, size_t count)
{
    if (!data) { count = 0; }

    const uint8_t* const end = (data + count);

    for (size_t i = 0; i < count; ++i)
    {
        const int byte = *(data + i);
        const size_t row = (i / 16);
        const size_t col = (i % 16);

        if (col == 0)
        {
            if (i == 0) { printf("%05zx ", i); }
            else
            {
                char str[17];
                hexDump_dataToString(str, data + 16 * (row - 1), end);
                printf("  | %s\n%05zx ", str, i);
            }
        }
        else if (col == 8) { printf(" "); }

        printf(" %02x", byte);
    }

    if (count == 0) { printf("%05x ", 0); }



    size_t lastRowSize = (count % 16);
    if ((lastRowSize == 0) && (count != 0)) { lastRowSize = 16; }
    const size_t remaining = (16 - lastRowSize);

    if (remaining >= 8) { printf(" "); }
    for (size_t i = 0; i < remaining; ++i) { printf("   "); }

    char str[17];
    hexDump_dataToString(str, end - lastRowSize, end);
    printf("  | %s", str);

    printf("\n");
}

// hex dump
//======================================================================================================================



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
