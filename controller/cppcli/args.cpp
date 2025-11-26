/*
 * Simple Task Queue
 * Copyright (c) 2024 fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <new>
#include <iostream>

#include "spdlog/spdlog.h"

#include "controller/cppcli/global.hpp"
#include "model/utils.hpp"

#include "args.hpp"

namespace Controller
{

namespace CppCLI
{

// public member functions
Args::Args() :
    m_args(std::vector<std::string>(16)),
    m_argv(nullptr),
    m_argvLen(17)
{}

Args::~Args()
{
    cleanArgv();
    delete[] m_argv;
}

u8 Args::init()
{
    m_argv = new (std::nothrow) char *[m_args.capacity() + 1]();
    if (!m_argv)
    {
        spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
        m_argvLen = 0;
        return 1;
    }

    m_args.clear();
    return 0;
}

char **Args::argv() const
{
    return m_argv;
}

i32 Args::argc() const
{
    return static_cast<i32>(m_args.size());
}

std::vector<std::string> Args::args() const
{
    return m_args;
}

u8 Args::getArgs(const std::string &prefix)
{
    while (Global::keepRunning.load(std::memory_order_relaxed))
    {
        m_args.clear();
        fmt::print("{}", prefix);
        std::string line;
        std::getline(std::cin, line);
        if (line.empty())
        {
            continue;
        }

        std::string delimiter = " ";

        size_t pos = 0;
        std::string token;
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            token = line.substr(0, pos);
            if (!token.empty())
            {
                m_args.push_back(std::move(token));
            }

            line.erase(0, pos + delimiter.length());
        }

        m_args.push_back(std::move(line));
        break;
    }

    // clean up argv
    cleanArgv();
    size_t newSize = m_args.capacity() + 1;
    if (m_argvLen < (newSize))
    {
        if (m_argv)
        {
            delete[] m_argv;
            m_argv = nullptr;
            m_argvLen = 0;
        }

        // re-allocate
        m_argv = new (std::nothrow) char *[newSize]();
        if (!m_argv)
        {
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        m_argvLen = newSize;
    }

    // copy data into argv
    for (size_t i = 0; i < m_args.size(); ++i)
    {
        m_argv[i] = new (std::nothrow) char[m_args.at(i).size() + 1](); // +1 for '\0'
        if (!m_argv[i])
        {
            cleanArgv();
            spdlog::error("{}:{} Fail to allocate memory", __FILE__, __LINE__);
            return 1;
        }

        memcpy(m_argv[i], m_args.at(i).c_str(), m_args.at(i).size());
        m_argv[i][m_args.at(i).size()] = '\0';
    }

    m_argv[m_args.size()] = NULL;
    return 0;
}

// private member functions
void Args::cleanArgv()
{
    if (!m_argv) return;

    for (size_t i = 0; i <= m_args.capacity(); ++i)
    {
        if (m_argv[i])
        {
            delete[] m_argv[i];
            m_argv[i] = nullptr;
        }
    }
}

} // end namespace CppCLI

} // end namespace Controller
