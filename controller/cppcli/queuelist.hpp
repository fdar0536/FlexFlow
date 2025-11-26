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

#ifndef _CONTROLLER_CLI_QUEUELIST_HPP_
#define _CONTROLLER_CLI_QUEUELIST_HPP_

#include <functional>
#include <memory>
#include <unordered_map>

#include "cxxopts.hpp"

#include "controller/global/defines.hpp"
#include "model/dao/iqueuelist.hpp"

namespace Controller
{

namespace CppCLI
{

class QueueList
{
public:

    QueueList();

    u8 init();

    i32 run();

private:

    std::string m_prefix;

    std::shared_ptr<Model::DAO::IQueueList> m_queueList;

    std::unordered_map<std::string, std::function<i32(void)>> m_funcs;

    cxxopts::Options m_createOpts = cxxopts::Options("create", "create new queue");

    i32 create();

    cxxopts::Options m_deleteOpts = cxxopts::Options("delete", "delete the queue");

    i32 Delete();

    cxxopts::Options m_listOpts = cxxopts::Options("list", "list queue(s)");

    i32 list();

    cxxopts::Options m_renameOpts = cxxopts::Options("rename", "rename the queue");

    i32 rename();

    i32 enter();
}; // end class QueueList

} // end namespace CppCLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_QUEUELIST_HPP_
