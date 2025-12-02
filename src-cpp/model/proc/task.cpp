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

#include "fmt/core.h"

#include "task.hpp"

namespace Model
{

namespace Proc
{

Task::Task() :
    execName(""),
    args(std::vector<std::string>()),
    workDir(""),
    ID(0),
    exitCode(0),
    isSuccess(false)
{
    args.clear();
}

void Task::print() const
{
    fmt::println("execName: {}", execName);
    fmt::println("args: ");
    for (auto it = args.begin(); it != args.end(); ++it)
    {
        fmt::println("{}", *it);
    }
    fmt::println("");

    fmt::println("workDir: {}", workDir);
    fmt::println("ID: {}", ID);
    fmt::println("exitCode: {}", exitCode);
    fmt::println("isSuccess: {}", std::to_string(isSuccess));
}

} // end namespace Proc

} // end namespace Model
