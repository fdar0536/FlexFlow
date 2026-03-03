/*
 * Flex Flow
 * Copyright (c) 2026-present fdar0536
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

#ifndef _CONTROLLER_GUI_CONFIG_HPP_
#define _CONTROLLER_GUI_CONFIG_HPP_

#include <memory>
#include <string>

#include "controller/global/defines.h"
#include "model/dao/iconnect.hpp"
#include "model/dao/iqueuelist.hpp"

namespace Controller
{

namespace Gui
{

class ConnectionProfile
{
public:

    ConnectionProfile& operator=(const ConnectionProfile &) = delete;

    void copy(ConnectionProfile &);

    u8 mode();

    void setMode(u8);

    u16 port();

    void setPort(u16);

    std::string target();

    void setTarget(const std::string &);

    std::shared_ptr<Model::DAO::IQueueList> list();

    void setList(std::shared_ptr<Model::DAO::IQueueList>);

private:

    u8 m_mode = BACKEND_SQLITE;
    u16 m_port = 0;
    std::string m_target = "";
    
    std::shared_ptr<Model::DAO::IQueueList> m_list;
    std::mutex m_mutex;
};

class Config
{
public:

    ~Config();
    
    static void parse();

    ConnectionProfile currentProfile;

    std::string lastProfile = "";
    std::unordered_map<std::string, ConnectionProfile> profiles;

private:

    static void writeDefault();

    std::string m_configFile = "";
};

} // namespace Gui

} // namespace Controller

#endif // _CONTROLLER_GUI_CONFIG_HPP
