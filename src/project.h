/*
author          Oliver Blaser
date            20.11.2022
copyright       GNU GPLv3 - Copyright (c) 2022 Oliver Blaser
*/

#ifndef IG_PROJECT_H
#define IG_PROJECT_H

#include <omw/defs.h>
#include <omw/version.h>


namespace prj
{
    const char* const appDirName_windows = "phodime";
    const char* const appDirName_unix = "phodime";

#ifdef OMW_PLAT_WIN
    const char* const appDirName = appDirName_windows;
#else
    const char* const appDirName = appDirName_unix;
#endif

    const char* const appName = "Photo Directory Merger";
    const char* const exeName = "phodime"; // eq to the linker setting

    const char* const website = "https://github.com/oblaser/photo-directory-merger";

    const omw::Version version(1, 0, 1, "alpha");
}


#ifdef OMW_DEBUG
#define PRJ_DEBUG (1)
#else
#undef PRJ_DEBUG
#endif


#endif // IG_PROJECT_H
