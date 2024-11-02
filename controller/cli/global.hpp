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

#ifndef _CONTROLLER_CLI_GLOBAL_HPP_
#define _CONTROLLER_CLI_GLOBAL_HPP_

#include <atomic>
#include <string>
#include <vector>

#include "cxxopts.hpp"

#include "controller/cli/args.hpp"
#include "controller/cli/config.hpp"

namespace Controller
{

namespace CLI
{

namespace Global
{

extern std::atomic<bool> keepRunning;

extern Controller::CLI::Config config;

extern Controller::CLI::Args args;

} // end namespace Global

} // end namesapce CLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_GLOBAL_HPP_
