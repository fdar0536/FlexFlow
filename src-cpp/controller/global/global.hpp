/*
 * Flex Flow
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

#ifndef _CONTROLLER_GLOBAL_GLOBAL_HPP_
#define _CONTROLLER_GLOBAL_GLOBAL_HPP_

#include <string>

#include "defines.h"

namespace Model
{
    namespace DAO
    {
        class IQueueList;
    }
} // namespace Model

constexpr
std::string_view relative_path(std::string_view path, std::string_view prefix)
{
    if (path.starts_with(prefix))
    {
        path.remove_prefix(prefix.length());
    }

    return path;
}

// Your logging macro uses this function:
#define LOG_FILE_PATH(x) relative_path(x, PROJECT_ROOT_DIR)

namespace Controller
{

namespace Global
{

u8 consoleInit();

void consoleFin();

bool isAdmin();

u8 spdlogInit(const std::string &);

u8 sqliteInit(Model::DAO::IQueueList **out, const std::string &target);

u8 grpcInit(Model::DAO::IQueueList **out, const std::string &target, const i32 port);

} // end namesapce Global

} // end namespace Controller

#endif // _CONTROLLER_GLOBAL_GLOBAL_HPP_
