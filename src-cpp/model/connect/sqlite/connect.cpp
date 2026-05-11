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

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "connect.hpp"

namespace Model
{

namespace Connect
{

namespace SQLite
{

std::shared_ptr<Token> connect(std::string &target)
{
    spdlog::debug("{}:{} Model::Connect::SQLite::connect",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (target.empty())
    {
        spdlog::error("{}:{} target is empty.", LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    std::string basePath = target;
    Utils::convertPath(basePath);
    if (basePath.at(basePath.length() - 1) == '/')
    {
        basePath = basePath.substr(0, basePath.length() - 1);
    }

    if (Utils::verifyDir(basePath))
    {
        spdlog::error("{}:{} Fail to verify basePath.",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    target = basePath;
    Token *token = new (std::nothrow) Token;
    if (!token)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return nullptr;
    }

    return std::shared_ptr<Token>(token);
}

} // end namespace SQLite

} // end namespace Connect

} // end namespace Model
