/*
 * Flex Flow
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

#include "spdlog/spdlog.h"

#include "model/utils.hpp"

#include "task.hpp"

namespace Model
{

namespace Proc
{

void printTask(const Task &task)
{
    spdlog::debug("{}:{} printTask", LOG_FILE_PATH(__FILE__), __LINE__);
    
    fmt::println("execName: {}", task.execName);
    fmt::println("args: ");
    for (auto it = task.args.begin(); it != task.args.end(); ++it)
    {
        fmt::println("{}", *it);
    }
    fmt::println("");

    fmt::println("workDir: {}", task.workDir);
    fmt::println("ID: {}", task.ID);
    fmt::println("exitCode: {}", task.exitCode);
    fmt::println("isSuccess: {}", std::to_string(task.isSuccess));
}

} // end namespace Proc

} // end namespace Model
