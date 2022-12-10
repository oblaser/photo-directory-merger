/*
author          Oliver Blaser
date            20.11.2022
copyright       GNU GPLv3 - Copyright (c) 2022 Oliver Blaser
*/

#ifndef IG_APP_CLIARG_H
#define IG_APP_CLIARG_H

#include <string>
#include <vector>

#include "project.h"

#include <omw/omw.h>

namespace argstr
{
    // add new args to
    // - app::OptionList::checkOpt()
    // - Args::containsXY() const
    // - help text

    const char* const force = "-f";
    const char* const help = "-h";
    const char* const help_alt = "--help";
    const char* const noColor = "--no-color";
    const char* const quiet = "-q";
    const char* const verbose = "-v";
    const char* const version = "--version";
}

namespace app
{
    class FileList : public std::vector<omw::string>
    {
    public:
        FileList() {}
        virtual ~FileList() {}

        virtual void add(const omw::string& file) { push_back(file); }

        inline omw::string getFile(size_t idx) const;

        bool isValid() const;
    };

    class OptionList : public std::vector<omw::string>
    {
    public:
        OptionList();
        virtual ~OptionList() {}

        virtual void add(const omw::string& opt);

        virtual bool contains(const omw::string& arg) const;

        omw::string unrecognized() const;

        bool isValid() const { return m_isValid; }

    private:
        size_t m_unrecognizedIdx;
        bool m_isValid;

        void addOpt(const omw::string& opt);
        bool checkOpt(const omw::string& opt) const;
    };

    class Args
    {
    public:
        Args() {}
        Args(int argc, char** argv) { parse(argc, argv); }
        virtual ~Args() {}

        void parse(int argc, char** argv);
        void add(const omw::string& arg);

        std::vector<std::string> inDirs() const;
        std::string outDir() const;

        OptionList& options() { return m_options; }
        const OptionList& options() const { return m_options; }
        bool containsForce() const { return m_options.contains(argstr::force); }
        bool containsHelp() const { return (m_options.contains(argstr::help) || m_options.contains(argstr::help_alt)); }
        bool containsNoColor() const { return m_options.contains(argstr::noColor); }
        bool containsQuiet() const { return m_options.contains(argstr::quiet); }
        bool containsVerbose() const { return m_options.contains(argstr::verbose); }
        bool containsVersion() const { return m_options.contains(argstr::version); }

        size_t count() const;
        size_t size() const;

        bool isValid() const;

        const omw::string& operator[](size_t idx) const;

    private:
        FileList m_files;
        OptionList m_options;
    };
}

#endif // IG_APP_CLIARG_H
