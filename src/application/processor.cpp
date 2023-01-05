/*
author          Oliver Blaser
date            04.01.2023
copyright       GNU GPLv3 - Copyright (c) 2023 Oliver Blaser
*/

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "middleware/util.h"
#include "processor.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/string.h>


#define ERROR_PRINT(msg)            \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) printError(msg);    \
}

#define WARNING_PRINT(msg)          \
{                                   \
    rcnt.incWarnings();             \
    if (!quiet) printWarning(msg);  \
}

#define ERROR_PRINT_EC_THROWLINE(msg, EC_x) \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) printError(msg);    \
    r = EC_x;                       \
    throw (int)(__LINE__);          \
}


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
    enum ERRORCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
    {
        EC_OK = 0,
        EC_ERROR = 1,

        EC__begin_ = 79,

        EC_OUTDIR_NOTEMPTY = EC__begin_,
        EC_INOUTDIR_EQ,
        EC_OUTDIR_NOTCREATED,

        EC_USER_ABORT, // not actually returned

        EC__end_,

        EC__max_ = 113
    };
    static_assert(EC__end_ <= EC__max_, "too many error codes defined");

    // 
    // "### normal "quoted bright" white"
    // "### normal @just bright@ white"
    // 
    void printFormattedText(const std::string& text)
    {
        bool format = false;

        if (text.length() > 5)
        {
            if ((text[0] == '#') &&
                (text[1] == '#') &&
                (text[2] == '#')
                )
            {
                format = true;
            }
        }

        if (format)
        {
            bool on = false;

            size_t i = 3;

            while (i < text.length())
            {
                if (text[i] == '\"')
                {
                    if (on)
                    {
                        cout << omw::defaultForeColor;
                        cout << text[i];
                        on = false;
                    }
                    else
                    {
                        cout << text[i];
                        cout << omw::fgBrightWhite;
                        on = true;
                    }
                }
                else if (text[i] == '@')
                {
                    if (on)
                    {
                        cout << omw::defaultForeColor;
                        on = false;
                    }
                    else
                    {
                        cout << omw::fgBrightWhite;
                        on = true;
                    }
                }
                else cout << text[i];

                ++i;
            }

            cout << omw::defaultForeColor;
        }
        else cout << text;
    }

    void printFormattedLine(const std::string& text)
    {
        printFormattedText(text);
        cout << endl;
    }

    constexpr int ewiWidth = 10;
    void printError(const std::string& text)
    {
        cout << omw::fgBrightRed << std::left << std::setw(ewiWidth) << "error:" << omw::defaultForeColor;
        printFormattedText(text);
        cout << endl;
    }
    void printInfo(const std::string& text)
    {
        cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor;
        printFormattedText(text);
        cout << endl;
    }
    void printWarning(const std::string& text)
    {
        cout << omw::fgBrightYellow << std::left << std::setw(ewiWidth) << "warning:" << omw::defaultForeColor;
        printFormattedText(text);
        cout << endl;
    }

    void printTitle(const std::string& title)
    {
        //cout << omw::fgBrightWhite << title << omw::normal << endl;
        cout << title << endl;
    }



    bool equivalent(const std::vector<fs::path>& inDirs, const fs::path& outDir)
    {
        bool r = false;

        for (size_t i = 0; (i < inDirs.size()) && !r; ++i)
        {
            if (fs::exists(inDirs[i])) r = fs::equivalent(inDirs[i], outDir);
        }

        return r;
    }



#ifdef PRJ_DEBUG
    const std::string magentaDebugStr = "\033[95mDEBUG\033[39m";
#endif



    typedef enum SCHEME
    {
        unknown = 0,
        huawai,     // IMG_YYYYMMDD_hhmmss
        samsung,    // YYYYMMDD_hhmmss
        winphone    // WP_YYYYMMDD_hh_mm_ss_Pro
    } scheme_t;

    omw::string toString(const scheme_t& scheme)
    {
        omw::string r = "ERROR";

        switch (scheme)
        {
        case SCHEME::unknown:
            r = "Unknown";
            break;

        case SCHEME::huawai:
            r = "Huawai";
            break;

        case SCHEME::samsung:
            r = "Samsung";
            break;

        case SCHEME::winphone:
            r = "Winphone";
            break;

        default:
            r = "ERROR";
            break;
        }

        return r;
    }

    bool schemeIsHuawai(const omw::stringVector_t& tokens)
    {
        bool r = false;

        if (tokens.size() >= 3)
        {
            if ((tokens[0] == "IMG") &&
                (tokens[1].length() == 8) && omw::isUInteger(tokens[1]) &&
                (tokens[2].length() == 6) && omw::isUInteger(tokens[2]))
            {
                r = true;
            }
        }

        return r;
    }

    bool schemeIsSamsung(const omw::stringVector_t& tokens)
    {
        bool r = false;

        if (tokens.size() >= 2)
        {
            if ((tokens[0].length() == 8) && omw::isUInteger(tokens[0]) &&
                (tokens[1].length() == 6) && omw::isUInteger(tokens[1]))
            {
                r = true;
            }
        }

        return r;
    }

    bool schemeIsWinPhone(const omw::stringVector_t& tokens)
    {
        bool r = false;

        if (tokens.size() >= 6)
        {
            if ((tokens[0] == "WP") &&
                (tokens[1].length() == 8) && omw::isUInteger(tokens[1]) &&
                (tokens[2].length() == 2) && omw::isUInteger(tokens[2]) &&
                (tokens[3].length() == 2) && omw::isUInteger(tokens[3]) &&
                (tokens[4].length() == 2) && omw::isUInteger(tokens[4]) &&
                (tokens[5] == "Pro"))
            {
                r = true;
            }
        }

        return r;
    }

    scheme_t detectScheme(const fs::path& inDir, double* pRate = nullptr)
    {
        scheme_t r = SCHEME::unknown;

        if (fs::exists(inDir))
        {
            std::vector<omw::string> stemFilenames;

            for (const fs::directory_entry& entry : fs::directory_iterator(inDir))
            {
                if (entry.is_regular_file()) stemFilenames.push_back(entry.path().stem().u8string());
            }

            constexpr size_t k = 30;
            size_t blockSize = stemFilenames.size() / k;
            if ((blockSize == 0) || (stemFilenames.size() <= k)) blockSize = 1;

#if defined(PRJ_DEBUG) && 0
            cout << omw::fgBrightBlack << "@" << __FUNCTION__ << " blockSize: " << blockSize << ", stemFilenames.size(): " << stemFilenames.size() << omw::fgDefault << endl;
#endif

            std::vector<omw::string> analyze;
            for (size_t i = 0; i < stemFilenames.size(); ++i)
            {
                if ((i % blockSize) == 0) analyze.push_back(stemFilenames[i]);
            }

            stemFilenames.clear();
            stemFilenames.shrink_to_fit();

            size_t cnt_huawai = 0;
            size_t cnt_samsung = 0;
            size_t cnt_winphone = 0;

            for (size_t i = 0; i < analyze.size(); ++i)
            {
                const auto tokens = analyze[i].split('_', 7); // maxNTokens + 1
                if (schemeIsHuawai(tokens)) ++cnt_huawai;
                if (schemeIsSamsung(tokens)) ++cnt_samsung;
                if (schemeIsWinPhone(tokens)) ++cnt_winphone;
            }

            std::array<size_t, 3> cnt = { cnt_huawai, cnt_samsung, cnt_winphone };
            std::sort(cnt.begin(), cnt.end(), std::greater<size_t>());

            const double rate = (double)(cnt[0]) / (double)(analyze.size());
            if (pRate) *pRate = rate;

            if ((cnt[0] != cnt[1]) && (rate >= 0.75))
            {
                if (cnt[0] == cnt_huawai) r = SCHEME::huawai;
                else if (cnt[0] == cnt_samsung) r = SCHEME::samsung;
                else if (cnt[0] == cnt_winphone) r = SCHEME::winphone;
                //else if (cnt[0] == cnt_) r = SCHEME::;
                else
                {
                    r = SCHEME::unknown;
                    // maybe print something
                }
            }
            else
            {
                r = SCHEME::unknown;
                // maybe print something
            }
        }
        else
        {
            r = SCHEME::unknown;
            // maybe print something
        }

        if (pRate && (r == SCHEME::unknown)) *pRate = 1;

        return r;
    }

    int process_huawai(const std::string& inDir, const std::string& outDir, const app::Flags& flags)
    {
        // TODO implement
        return (-1);
    }

    int process_samsung(const std::string& inDir, const std::string& outDir, const app::Flags& flags)
    {
        // TODO implement
        return (-1);
    }

    int process_wp(const std::string& inDir, const std::string& outDir, const app::Flags& flags)
    {
        // TODO implement
        return (-1);
    }



#pragma region library
    int cliChoice(const std::string& q, int def = 0, char first = 'y', char second = 'n')
    {
        int r = 0;
        const omw::string a(1, first);
        const omw::string b(1, second);
        omw::string data;

        do
        {
            std::cout << q << " [" << (def == 1 ? a.toUpper_ascii() : a) << "/" << (def == 2 ? b.toUpper_ascii() : b) << "] ";
            std::getline(std::cin, data);

            if (data.toLower_ascii() == a) r = 1;
            else if (data.toLower_ascii() == b) r = 2;
            else if (data.length() == 0) r = def;
            else r = 0;
        }
        while (r == 0);

        return r;
    }

    omw::string to_string(uint64_t val, int base, const char* digits)
    {
        omw::string r = "";

        if (val == 0) r += digits[0];

        while (val != 0)
        {
            r = digits[val % base] + r; // use reverse() instead
            val /= base;
        }

        return r;
    }
#pragma endregion
}



int app::process(const std::vector<std::string>& inDirs, const std::string& outDir, const app::Flags& flags)
{
    int r = EC_OK; // set to OK because of catch(...) and foreach inDirs

    const bool quiet = flags.quiet;
    bool ___verbose = flags.verbose;
    const bool& verbose = ___verbose;
    if (quiet) ___verbose = false;

    try
    {
        util::ResultCounter rcnt = 0;
        size_t nSucceeded = 0;

        std::vector<fs::path> ___inDirPaths(inDirs.size());
        const std::vector<fs::path>& inDirPaths = ___inDirPaths;
        const fs::path outDirPath = outDir;
        for (size_t i = 0; i < inDirs.size(); ++i) ___inDirPaths.at(i) = inDirs[i];



        ///////////////////////////////////////////////////////////
        // check/create out dir
        ///////////////////////////////////////////////////////////

        if (fs::exists(outDir))
        {
            if (::equivalent(inDirPaths, outDirPath)) ERROR_PRINT_EC_THROWLINE("an INDIR and the OUTDIR are equivalent", EC_INOUTDIR_EQ);

            if (!fs::is_empty(outDir))
            {
                if (flags.force)
                {
                    if (verbose)
                    {
                        rcnt.incWarnings();
                        printWarning("using non empty OUTDIR");
                    }
                }
                else
                {
                    const std::string msg = "###OUTDIR \"" + outDir + "\" is not empty";

                    if (verbose)
                    {
                        printInfo(msg);

                        if (2 == cliChoice("use non empty OUTDIR?"))
                        {
                            r = EC_USER_ABORT;
                            throw (int)(__LINE__);
                        }
                    }
                    else ERROR_PRINT_EC_THROWLINE(msg, EC_OUTDIR_NOTEMPTY);
                }
            }
        }
        else
        {
            fs::create_directories(outDirPath);

            if (!fs::exists(outDir)) ERROR_PRINT_EC_THROWLINE("could not create OUTDIR", EC_OUTDIR_NOTCREATED);
        }


        ///////////////////////////////////////////////////////////
        // process
        ///////////////////////////////////////////////////////////

#ifndef PRJ_DEBUG
#error "not implemented"
#endif

        double rate = 0;
        scheme_t scheme;

        for (size_t i_inDir = 0; i_inDir < inDirs.size(); ++i_inDir)
        {
            const util::ResultCounter::counter_type nErrorsOld = rcnt.errors();
            const auto& inDir = inDirs[i_inDir];

            scheme = detectScheme(inDir, &rate);
            printFormattedLine("###\"" + inDir + "\" " + toString(scheme) + (scheme == SCHEME::unknown ? "" : " (" + std::to_string((int)round(rate * 100)) + "%)"));

            if (fs::exists(inDir))
            {
                if (!fs::is_empty(inDir))
                {
                    if(!quiet) printInfo("processing not yet implemented");
                }
                else WARNING_PRINT("INDIR is empty");
            }
            else ERROR_PRINT("INDIR does not exist");

            if (rcnt.errors() == nErrorsOld) ++nSucceeded;
        }

        ///////////////////////////////////////////////////////////
        // end
        ///////////////////////////////////////////////////////////

        if (!quiet)
        {
            cout << "========";

            cout << "  " << omw::fgBrightWhite;
            cout << nSucceeded << "/" << inDirs.size();
            cout << omw::normal << " succeeded";

            cout << ", ";
            if (rcnt.errors() != 0) cout << omw::fgBrightRed;
            cout << rcnt.errors();
            if (rcnt.errors() != 0) cout << omw::normal;
            cout << " error";
            if (rcnt.errors() != 1) cout << "s";

            cout << ", ";
            if (rcnt.warnings() != 0) cout << omw::fgBrightYellow;
            cout << rcnt.warnings();
            if (rcnt.warnings() != 0) cout << omw::normal;
            cout << " warning";
            if (rcnt.warnings() != 1) cout << "s";

            cout << " ========" << endl;
        }

        //if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;

        if (((nSucceeded == inDirs.size()) && (rcnt.errors() != 0)) ||
            ((nSucceeded != inDirs.size()) && (rcnt.errors() == 0)))
        {
            r = EC_OK;
            throw (int)(__LINE__);
        }

        if (nSucceeded != inDirs.size()) r = EC_ERROR;
    }
    catch (std::filesystem::filesystem_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            printError("fatal error");
            cout << "    path1: " << ex.path1() << endl;
            cout << "    path2: " << ex.path2() << endl;
            cout << "    cat:   " << ex.code().category().name() << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << ex.code().message() << endl;
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (std::system_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            printError("fatal error");
            cout << "    cat:   " << ex.code().category().name() << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << ex.code().message() << endl;
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (std::exception& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            printError("fatal error");
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (const int& ex)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) printError("fatal error (" + std::to_string(ex) + ")");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }
    catch (...)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) printError("unspecified fatal error");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }

    if (r == EC_USER_ABORT) r = EC_OK;

    return r;
}
