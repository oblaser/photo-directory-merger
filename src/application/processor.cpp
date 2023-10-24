/*
author          Oliver Blaser
date            26.02.2023
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
#include <omw/vector.h>


#define IMPLEMENT_FLAGS()           \
const bool quiet = flags.quiet;     \
bool ___verbose = flags.verbose;    \
const bool& verbose = ___verbose;   \
if (quiet) ___verbose = false;

#define ERROR_PRINT(msg)            \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) printError(msg);    \
}

#define INFO_PRINT(msg)         \
{                               \
    if (!quiet) printInfo(msg); \
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
    void printInfo()
    {
        cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor;
    }
    void printInfo(const std::string& text)
    {
        printInfo();
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
        while ((r != 1) && (r != 2));

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



    bool equivalent(const std::vector<fs::path>& inDirs, const fs::path& outDir)
    {
        bool r = false;

        for (size_t i = 0; (i < inDirs.size()) && !r; ++i)
        {
            if (fs::exists(inDirs[i])) r = fs::equivalent(inDirs[i], outDir);
        }

        return r;
    }

    omw::string getDirName(const fs::path& dir)
    {
        omw::string r;
        
        if (dir.has_filename()) r = dir.filename().u8string();
        else r = dir.parent_path().filename().u8string();

        return r;
    }



#ifdef PRJ_DEBUG
    const std::string magentaDebugStr = "\033[95mDEBUG\033[39m";
#endif



    constexpr char inFileDelimiter = '_';

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

    constexpr size_t nTokensHuawai = 3;
    constexpr size_t nTokensSamsung = 2;
    constexpr size_t nTokensWinPhone = 6;
    constexpr size_t nTokensMax = nTokensWinPhone;

    bool schemeIsHuawai(const omw::stringVector_t& tokens)
    {
        bool r = false;

        if (tokens.size() >= nTokensHuawai)
        {
            if (((tokens[0] == "IMG") || (tokens[0] == "VID") || (tokens[0] == "PANO")) &&
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

        if (tokens.size() >= nTokensSamsung)
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

        if (tokens.size() >= nTokensWinPhone)
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

    scheme_t detectScheme(const omw::stringVector_t& tokens)
    {
        scheme_t r = SCHEME::unknown;

        const bool huawai = schemeIsHuawai(tokens);
        const bool samsung = schemeIsSamsung(tokens);
        const bool wp = schemeIsWinPhone(tokens);

        if (huawai && !samsung && !wp) r = SCHEME::huawai;
        else if (!huawai && samsung && !wp) r = SCHEME::samsung;
        else if (!huawai && !samsung && wp) r = SCHEME::winphone;
        // else nop

        return r;
    }

    // returns SCHEME::unknown if the rate is too small
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
                const auto tokens = analyze[i].split('_', nTokensMax + 1);
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

    constexpr char outFileDelimiter = '-';
    constexpr char outFileDelimiter_opt = '_';

    // YYYYMMDD-hhmmss-NAME[_...]
    std::string outFileStem(const scheme_t& scheme, const omw::stringVector_t& tokens, const std::string& inDirName)
    {
        std::string r;
        size_t nTokens;

        switch (scheme)
        {
        case SCHEME::huawai:
            r = tokens[1] + outFileDelimiter + tokens[2] + outFileDelimiter + inDirName;
            nTokens = nTokensHuawai;
            break;

        case SCHEME::samsung:
            r = tokens[0] + outFileDelimiter + tokens[1] + outFileDelimiter + inDirName;
            nTokens = nTokensSamsung;
            break;

        case SCHEME::winphone:
            r = tokens[1] + outFileDelimiter + tokens[2] + tokens[3] + tokens[4] + outFileDelimiter + inDirName + outFileDelimiter + tokens[0];
            nTokens = nTokensWinPhone;
            break;

        default:
            throw (int)(__LINE__);
            break;
        }

        for (size_t i = nTokens; i < tokens.size(); ++i)
        {
            r += (outFileDelimiter_opt + tokens[i]);
        }

        return r;
    }

    util::FileCounter process(const scheme_t& scheme, const std::string& inDir, const std::string& inDirName, const std::string& outDir, const app::Flags& flags, util::ResultCounter& rcnt)
    {
        IMPLEMENT_FLAGS();

        util::FileCounter rFileCnt;

        if (scheme == SCHEME::unknown) throw (int)(__LINE__);

        for (const fs::directory_entry& entry : fs::directory_iterator(inDir))
        {
            if (entry.is_regular_file())
            {
                rFileCnt.addTotal();

                const fs::path inFile = (fs::path(entry.path())).make_preferred();
                const auto inFileStemTokens = omw_::split(inFile.stem().u8string(), inFileDelimiter);

                if (scheme == detectScheme(inFileStemTokens))
                {
                    const auto outFileName = outFileStem(scheme, inFileStemTokens, inDirName) + inFile.extension().u8string();
                    const fs::path outFile = outDir / fs::path(outFileName);

#if defined(PRJ_DEBUG) && 0
                    printFormattedLine("###\"" + inFile.u8string() + "\" -> \"" + outFile.u8string() + "\"");
#endif
                    const bool outFileExists = fs::exists(outFile);
                    bool perform = true;
                    fs::copy_options opt = fs::copy_options::none;

                    if (outFileExists && flags.force)
                    {
                        opt = fs::copy_options::overwrite_existing;
                        if (verbose) WARNING_PRINT("###overwriting destination file \"" + outFile.u8string() + "\"");
                    }
                    else if (outFileExists && verbose)
                    {
                        printInfo("###destination file \"" + outFile.u8string() + "\" exists");
                        if (cliChoice("overwrite destination file?") == 1) opt = fs::copy_options::overwrite_existing;
                        else perform = false;
                    }
                    else if(outFileExists)
                    {
                        perform = false;
                        ERROR_PRINT("###destination file \"" + outFile.u8string() + "\" exists");
                    }

                    if (perform)
                    {
                        std::error_code ec;
                        const bool copied = fs::copy_file(inFile, outFile, opt, ec);

                        if ((copied && !(ec.value() == 0)) ||
                            (!copied && (ec.value() == 0)))
                        {
                            throw (int)(__LINE__);
                        }

                        if (copied) rFileCnt.addCopied();
                        else
                        {
                            ERROR_PRINT("###failed to copy file \"" + inFile.u8string() + "\" to \"" + outFile.u8string() + "\"");
                            if (verbose) printInfo(ec.message());
                        }
                    }
                }
                else
                {
                    ERROR_PRINT("###scheme mismatch on file \"" + inFile.u8string() + "\", file not copied");

                    if (verbose)
                    {
                        std::string outFileName = inFile.stem().u8string() + outFileDelimiter + inDirName + inFile.extension().u8string();
                        const fs::path outFile = (fs::path(outDir) / outFileName).make_preferred();

                        printInfo();
                        cout << "you may use: " << omw::fgBrightWhite;
#if defined(OMW_PLAT_UNIX)
                        cout << "cp";
#elif defined(OMW_PLAT_WIN)
                        cout << "copy";
#else
                        cout << "<COPY>";
#endif // OMW_PLAT_x
                        cout << " \"" + inFile.u8string() + "\" \"" + outFile.u8string() + "\"";
                        cout << omw::fgDefault << endl;
                    }
                }
            }
        }

        return rFileCnt;
    }

#if defined(PRJ_DEBUG)
    void dbg_rm_outDir(const std::string& outDir)
    {
        try
        {
            const auto n = fs::remove_all(outDir);
            cout << omw::fgBrightBlack << "rm OUTDIR: " << n << " items deleted" << omw::fgDefault << endl;
        }
        catch (const std::filesystem::filesystem_error& ex)
        {
            cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
            throw ex;
        }
        catch (const std::system_error& ex)
        {
            cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
            throw ex;
        }
        catch (const std::exception& ex)
        {
            cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
            throw ex;
        }
    }
#endif
}



int app::process(const std::vector<std::string>& inDirs, const std::string& outDir, const app::Flags& flags)
{
    int r = EC_OK; // set to OK because of catch(...) and foreach inDirs

    IMPLEMENT_FLAGS();

    try
    {
        util::FileCounter fileCnt;
        util::ResultCounter rcnt = 0;
        size_t nSucceeded = 0;
        omw::vector<std::string> postfixes;

        std::vector<fs::path> ___inDirPaths(inDirs.size());
        const std::vector<fs::path>& inDirPaths = ___inDirPaths;
        const fs::path outDirPath = outDir;
        for (size_t i = 0; i < inDirs.size(); ++i) ___inDirPaths.at(i) = inDirs[i];


#if defined(PRJ_DEBUG) && 1
        dbg_rm_outDir(outDir);
#endif


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
                    if (verbose) WARNING_PRINT("using non empty OUTDIR");
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

            if (!fs::exists(outDir)) ERROR_PRINT_EC_THROWLINE("failed to create OUTDIR", EC_OUTDIR_NOTCREATED);
        }


        ///////////////////////////////////////////////////////////
        // process
        ///////////////////////////////////////////////////////////

        double rate = 0;
        scheme_t scheme;
        omw::vector<omw::string> usedInDirNames;

        for (size_t i_inDir = 0; i_inDir < inDirs.size(); ++i_inDir)
        {
            const auto nErrorsOld = rcnt.errors();
            const auto& inDir = inDirs[i_inDir];

            if (verbose && (i_inDir > 0)) cout << endl;

            if (fs::directory_entry(inDir).is_directory())
            {
                scheme = detectScheme(inDir, &rate);
                if (!quiet) printFormattedLine("###\"" + (fs::path(inDir)).make_preferred().u8string() + "\" " + toString(scheme) + (scheme == SCHEME::unknown ? "" : " (" + std::to_string((int)round(rate * 100)) + "%)"));

                if (scheme != SCHEME::unknown)
                {
                    if (fs::exists(inDir))
                    {
                        if (!fs::is_empty(inDir))
                        {
                            const auto inDirName = getDirName(inDir);

                            if (!usedInDirNames.contains(inDirName))
                            {
                                usedInDirNames.push_back(inDirName);
                                const auto tmpFileCnt = ::process(scheme, inDir, inDirName, outDir, flags, rcnt);
                                if (verbose) printInfo("###copied @" + std::to_string(tmpFileCnt.copied()) + "/" + std::to_string(tmpFileCnt.total()) + "@ files");
                                fileCnt.add(tmpFileCnt);
                            }
                            else ERROR_PRINT("INDIR name was already used, no files copied");
                        }
                        else WARNING_PRINT("INDIR is empty");
                    }
                    else ERROR_PRINT("INDIR does not exist");
                }
                else ERROR_PRINT("unknown scheme");
            }
            else
            {
                if (!quiet) printFormattedLine("###\"" + inDir + "\"");
                ERROR_PRINT("INDIR is not a directory");
            }

            if (rcnt.errors() == nErrorsOld) ++nSucceeded;
        }

        ///////////////////////////////////////////////////////////
        // end
        ///////////////////////////////////////////////////////////

        if (!quiet)
        {
            if (verbose && (inDirs.size() > 1)) cout << endl;

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

            //if (verbose) printFormattedLine("###copied @" + std::to_string(fileCnt.copied()) + "/" + std::to_string(fileCnt.total()) + "@ files");
            if (verbose) printFormattedLine("copied " + std::to_string(fileCnt.copied()) + "/" + std::to_string(fileCnt.total()) + " files");
        }

        if (((nSucceeded == inDirs.size()) && (rcnt.errors() != 0)) ||
            ((nSucceeded != inDirs.size()) && (rcnt.errors() == 0)))
        {
            r = EC_OK;
            throw (int)(__LINE__);
        }

        //if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;

        if (nSucceeded != inDirs.size()) r = EC_ERROR;
    }
    catch (const std::filesystem::filesystem_error& ex)
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
    catch (const std::system_error& ex)
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
    catch (const std::exception& ex)
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
