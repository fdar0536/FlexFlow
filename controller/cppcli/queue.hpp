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

#ifndef _CONTROLLER_CLI_QUEUE_HPP_
#define _CONTROLLER_CLI_QUEUE_HPP_

#include <functional>
#include <memory>
#include <unordered_map>

#include "cxxopts.hpp"

#include "controller/global/defines.h"
#include "model/dao/iqueue.hpp"

namespace Controller
{

namespace CppCLI
{

class Queue
{
public:

    ~Queue();

    u8 init();

    i32 run(const std::string &, Model::DAO::IQueue *);

private:

    Model::DAO::IQueue *m_queue;

    std::unordered_map<std::string, std::function<i32(void)>> m_funcs;

    i32 printList(u8, const std::string &, const std::vector<int> &);

    cxxopts::Options m_listOpts = cxxopts::Options("list", "list items in this queue");

    i32 list();

    cxxopts::Options m_detailsOpts = cxxopts::Options("details", "print task details for given id");

    i32 details();

    cxxopts::Options m_clearOpts = cxxopts::Options("clear", "clear task list");

    i32 clear();

    cxxopts::Options m_currentOpts = cxxopts::Options("current", "print current task");

    i32 current();

    cxxopts::Options m_addOpts = cxxopts::Options("add", "add task to this queue");

    i32 add();

    cxxopts::Options m_removeOpts = cxxopts::Options("remove", "remove task to this queue");

    i32 remove();

    cxxopts::Options m_isRunningOpts = cxxopts::Options("isRunning", "check this queue is running or not");

    i32 isRunning();

    cxxopts::Options m_startOpts = cxxopts::Options("start", "start this queue");

    i32 start();

    cxxopts::Options m_stopOpts = cxxopts::Options("stop", "stop this queue");

    i32 stop();

    cxxopts::Options m_outputOpts = cxxopts::Options("output", "stdout of current task");

    i32 output();

}; // end class Queue

} // end namespace CppCLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_QUEUE_HPP_
