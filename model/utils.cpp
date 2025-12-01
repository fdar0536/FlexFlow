/*
 * Simple Task Queue
 * Copyright (c) 2023-present fdar0536
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

#include <regex>
#include <mutex>

#include "spdlog/spdlog.h"

#ifdef _WIN32
#include "windows.h"
#endif

#include "utils.hpp"

namespace Model
{

namespace Utils
{

static std::regex ipRegex = std::regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                                       "{3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

static std::mutex consoleMutex;

void writeLastError(const char *file, int line)
{
#ifdef _WIN32
    if (!file)
    {
        return;
    }

    LPSTR msgBuf = nullptr;
    DWORD errID = GetLastError();
    if (!errID)
    {
        return;
    }

    size_t errSize = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuf, 0, NULL);

    spdlog::error("{}:{} {}\n", file, line, std::string(msgBuf, errSize));
    LocalFree(msgBuf);
#else
    static_cast<void>(file);
    static_cast<void>(line);
#endif
}

void writeConsole(const std::string &in)
{
    std::unique_lock<std::mutex> lock(consoleMutex);
    fmt::print("{}", in.c_str());
}

u8 verifyIP(const std::string &in)
{
    if (!std::regex_match(in, ipRegex))
    {
        return 1;
    }

    return 0;
}

} // end namespace Utils

} // end namespace Model
