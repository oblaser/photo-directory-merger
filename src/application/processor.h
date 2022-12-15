/*
author          Oliver Blaser
date            20.11.2022
copyright       GNU GPLv3 - Copyright (c) 2022 Oliver Blaser
*/

#ifndef IG_APP_PROCESSOR_H
#define IG_APP_PROCESSOR_H

#include <string>
#include <vector>


namespace app
{
    struct Flags
    {
        Flags() = delete;

        Flags(bool force_, bool quiet_, bool verbose_)
            : force(force_), quiet(quiet_), verbose(verbose_)
        {}

        bool force;
        bool quiet;
        bool verbose;
    };

    int process(const std::vector<std::string>& inDirs, const std::string& outDir, const app::Flags& flags);
}


#endif // IG_APP_PROCESSOR_H
