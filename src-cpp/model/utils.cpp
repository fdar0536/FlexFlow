/*
 * Flex Flow
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

#include <filesystem>
#include <regex>
#include <mutex>

#include "controller/global/global.hpp"
#include "spdlog/spdlog.h"

#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif

#include "utils.hpp"

namespace Model
{

namespace Utils
{

static std::regex ipRegex = std::regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                                       "{3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

static std::mutex consoleMutex;

void writeLastError(const std::string_view &file, int line)
{
    spdlog::debug("{}:{} writeLastError", LOG_FILE_PATH(__FILE__), __LINE__);
#ifdef _WIN32
    LPSTR msgBuf = nullptr;
    DWORD errID = GetLastError();
    if (!errID)
    {
        spdlog::error("{}:{} GetLastError failed", LOG_FILE_PATH(__FILE__), __LINE__);
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

    spdlog::error("{}:{} {}", file, line, std::string(msgBuf, errSize));
    LocalFree(msgBuf);
#else
    static_cast<void>(file);
    static_cast<void>(line);
#endif
}

void writeConsole(const std::string &in)
{
    spdlog::debug("{}:{} writeConsole", LOG_FILE_PATH(__FILE__), __LINE__);
    std::unique_lock<std::mutex> lock(consoleMutex);
    fmt::print("{}", in.c_str());
}

u8 verifyIP(const std::string &in)
{
    spdlog::debug("{}:{} verifyIP", LOG_FILE_PATH(__FILE__), __LINE__);
    if (!std::regex_match(in, ipRegex))
    {
        return 1;
    }

    return 0;
}

bool isAdmin()
{
    spdlog::debug("{}:{} isAdmin", LOG_FILE_PATH(__FILE__), __LINE__);
#ifdef _WIN32
    PSID sid;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&auth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &sid))
    {
        return true;
    }

    BOOL res;
    if (!CheckTokenMembership(nullptr, sid, &res))
    {
        return true;
    }

    FreeSid(sid);
    return res;
#else
    if (!system("sudo -v -n &>/dev/null"))
    {
        // system("sudo -v -n &>/dev/null") == 0
        return true;
    }

    return (geteuid() == 0);
#endif
}

// for dir
u8 verifyDir(const std::string &in)
{
    spdlog::debug("{}:{} verifyDir", LOG_FILE_PATH(__FILE__), __LINE__);

    if (in.empty())
    {
        spdlog::error("{}:{} \"in\" is empty.", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::error_code ec;
    if (!std::filesystem::exists(in, ec))
    {
        if (!std::filesystem::create_directory(in, ec))
        {
            spdlog::error("{}:{} Fail to create directory {}.",
                          LOG_FILE_PATH(__FILE__), __LINE__, in);
            return 1;
        }

        return 0;
    }

    if (!std::filesystem::is_directory(in, ec))
    {
        spdlog::error("{}:{} {} is not directory.",
                      LOG_FILE_PATH(__FILE__), __LINE__, in);
        return 1;
    }

    return 0;
}

u8 verifyFile(const std::string &in)
{
    spdlog::debug("{}:{} verifyFile", LOG_FILE_PATH(__FILE__), __LINE__);

    if (in.empty())
    {
        spdlog::error("{}:{} \"in\" is empty.", LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    std::error_code ec;
    if (!std::filesystem::exists(in, ec))
    {
        spdlog::error("{}:{} {} is not exist.",
            LOG_FILE_PATH(__FILE__), __LINE__, in);
        return 1;
    }

    if (!std::filesystem::is_regular_file(in, ec))
    {
        spdlog::error("{}:{} {} is not regular file.",
                      LOG_FILE_PATH(__FILE__), __LINE__, in);
        return 1;
    }

    return 0;
}

void deleteDirectoryContents(const std::string& dir_path)
{
    spdlog::debug("{}:{} deleteDirectoryContents",
        LOG_FILE_PATH(__FILE__), __LINE__);

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        std::filesystem::remove_all(entry.path(), ec);
}

void convertPath(std::string &toConvert)
{
    spdlog::debug("{}:{} convertPath", LOG_FILE_PATH(__FILE__), __LINE__);

#ifdef _WIN32
    std::string from = "\\";
    std::string to = "/";
    size_t start_pos = 0;
    while((start_pos = toConvert.find(from, start_pos)) != std::string::npos)
    {
        toConvert.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
#else
    static_cast<void>(toConvert);
#endif
}

} // end namespace Utils

} // end namespace Model
