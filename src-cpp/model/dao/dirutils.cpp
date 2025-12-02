/*
 * Simple Task Queue
 * Copyright (c) 2023-2024 fdar0536
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

#include "spdlog/spdlog.h"

#include "dirutils.hpp"

namespace Model
{

namespace DAO
{

namespace DirUtils
{

u8 verifyDir(const std::string &in)
{
    if (in.empty())
    {
        spdlog::error("{}:{} \"in\" is empty.", __FILE__, __LINE__);
        return 1;
    }

    std::error_code ec;
    if (!std::filesystem::exists(in, ec))
    {
        if (!std::filesystem::create_directory(in, ec))
        {
            spdlog::error("{}:{} Fail to create directory {}.",
                          __FILE__, __LINE__, in);
            return 1;
        }

        return 0;
    }

    if (!std::filesystem::is_directory(in, ec))
    {
        spdlog::error("{}:{} {} is not directory.",
                      __FILE__, __LINE__, in);
        return 1;
    }

    return 0;
}

u8 verifyFile(const std::string &in)
{
    if (in.empty())
    {
        spdlog::error("{}:{} \"in\" is empty.", __FILE__, __LINE__);
        return 1;
    }

    std::error_code ec;
    if (!std::filesystem::exists(in, ec))
    {
        spdlog::error("{}:{} {} is not exist.", __FILE__, __LINE__, in);
        return 1;
    }

    if (!std::filesystem::is_regular_file(in, ec))
    {
        spdlog::error("{}:{} {} is not regular file.",
                      __FILE__, __LINE__, in);
        return 1;
    }

    return 0;
}

void deleteDirectoryContents(const std::string& dir_path)
{
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        std::filesystem::remove_all(entry.path(), ec);
}

void convertPath(std::string &toConvert)
{
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


} // end namespace DirUtils

} // end namespace DAO

} // end namespace Model
