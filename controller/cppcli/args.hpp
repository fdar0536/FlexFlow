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

#ifndef _CONTROLLER_CLI_ARGS_HPP_
#define _CONTROLLER_CLI_ARGS_HPP_

#include <vector>
#include <string>

#include "controller/global/defines.hpp"

namespace Controller
{

namespace CppCLI
{

class Args
{
public:

    Args();

    ~Args();

    u8 init();

    char **argv() const;

    i32 argc() const;

    std::vector<std::string> args() const;

    u8 getArgs(const std::string &);

private:

    std::vector<std::string> m_args;

    char **m_argv;

    size_t m_argvLen;

    void cleanArgv();

}; // end class Args

} // end namespace CppCLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_ARGS_HPP_
