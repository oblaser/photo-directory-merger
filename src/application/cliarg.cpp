/*
author          Oliver Blaser
date            20.11.2022
copyright       GNU GPLv3 - Copyright (c) 2022 Oliver Blaser
*/

#include <string>
#include <utility>
#include <vector>

#include "cliarg.h"
#include "project.h"
#include "middleware/util.h"

#include <omw/intdef.h>
#include <omw/string.h>


namespace
{
}



inline omw::string app::FileList::getFile(size_t idx) const
{
    return (this->size() > idx ? this->at(idx) : "");
}

bool app::FileList::isValid() const
{
    return (this->size() >= 2);
}



app::OptionList::OptionList()
    : m_unrecognizedIdx(OMW_SIZE_MAX), m_isValid(true)
{ }

void app::OptionList::add(const omw::string& opt)
{
    if (opt.length() > 1)
    {
        if ((opt[1] != '-') && (opt.length() > 2))
        {
            for (size_t i = 1; i < opt.length(); ++i)
            {
                addOpt(omw::string("-") + opt[i]);
            }
        }
        else addOpt(opt);
    }
    else addOpt(opt);
}

bool app::OptionList::contains(const omw::string& arg) const
{
    bool r = false;

    for (size_t i = 0; (i < this->size()) && !r; ++i)
    {
        if (this->at(i) == arg) r = true;
    }

    return r;
}

omw::string app::OptionList::unrecognized() const
{
    return (m_unrecognizedIdx != OMW_SIZE_MAX ? this->at(m_unrecognizedIdx) : "");
}

void app::OptionList::addOpt(const omw::string& opt)
{
    if (!checkOpt(opt))
    {
        m_isValid = false;
        m_unrecognizedIdx = this->size();
    }

    this->push_back(opt);
}

bool app::OptionList::checkOpt(const omw::string& opt) const
{
    return (
        (opt == argstr::force) ||
        (opt == argstr::help) || (opt == argstr::help_alt) ||
        (opt == argstr::noColor) ||
        (opt == argstr::quiet) ||
        (opt == argstr::verbose) ||
        (opt == argstr::version)
        );
}



void app::Args::parse(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const omw::string arg(argv[i]);

        if (arg.length() > 0) add(arg);
    }
}

void app::Args::add(const omw::string& arg)
{
    if (arg[0] == '-') m_options.add(arg);
#ifdef OMW_PLAT_WIN
    else if (arg == "/?") m_options.add(argstr::help);
#endif
    else m_files.add(arg);
}

std::vector<std::string> app::Args::inDirs() const
{
    std::vector<std::string> r;
    
    const size_t n = m_files.size() - 1;

    if (n > 0) // needed because size is unsigned
    {
        for (size_t i = 0; i < n; ++i)
        {
            r.push_back(m_files[i]);
        }
    }

    return r;
}

std::string app::Args::outDir() const
{
    return m_files.back();
}

size_t app::Args::count() const
{
    return size();
}

size_t app::Args::size() const
{
    return (m_files.size() + m_options.size());
}

bool app::Args::isValid() const
{
    return (
        (m_files.isValid() && m_options.isValid()) ||
        (m_options.isValid() && (containsHelp() || containsVersion()))
        );
}

const omw::string& app::Args::operator[](size_t idx) const
{
    if (idx < m_options.size()) return m_options[idx];
    else return m_files[idx - m_options.size()];
}
