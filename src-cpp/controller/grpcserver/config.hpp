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

#ifndef _CONTROLLER_GRPCSERVER_CONFIG_HPP_
#define _CONTROLLER_GRPCSERVER_CONFIG_HPP_

#include <string>

#include "spdlog/common.h"

#include "controller/global/defines.h"

namespace Controller
{

namespace GRPCServer
{

class Config
{
public:

    Config();

    ~Config();

    static u8 parse(Config *in, int argc, char **argv);

    static u8 parse(Config *, const std::string &);

    std::string dbPath;

    std::string logPath = "";

    u16 listenPort = 12345;

    std::string listenIP = "127.0.0.1";

    i32 logLevel = static_cast<i32>(spdlog::level::level_enum::info);

private:

    static void printVersion();
};

} // end namespace GRPCServer

} // end namespace Model

#endif // _CONTROLLER_GRPCSERVER_CONFIG_HPP_
