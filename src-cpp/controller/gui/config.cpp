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

#include <fstream>
#include <mutex>

#include "QDir"
#include "QStandardPaths"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "controller/global/global.hpp"
#include "controller/gui/global.hpp"

#include "config.hpp"

namespace YAML
{
    template<>
    struct convert<Controller::Gui::ConnectionProfile>
    {
        static bool decode(const Node& node,
            Controller::Gui::ConnectionProfile& rhs)
        {
            if(!node.IsMap()) return false;
            rhs.setMode(node["mode"].as<u8>());
            rhs.setPort(node["port"].as<u16>());
            rhs.setTarget(node["target"].as<std::string>());
            return true;
        }

        static Node encode(
            Controller::Gui::ConnectionProfile& rhs)
        {
            Node node;
            node["mode"] = static_cast<int>(rhs.mode());
            node["port"] = static_cast<int>(rhs.port());
            node["target"] = rhs.target();
            return node;
        }
    };
} // namespace YAML

namespace Controller
{

namespace Gui
{

// ConnectionProfile
void ConnectionProfile::copy(ConnectionProfile &in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_mode = in.mode();
    m_port = in.port();
    m_target = in.target();
    m_list = in.list();
}

u8 ConnectionProfile::mode()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_mode;
}

void ConnectionProfile::setMode(u8 in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_mode = in;
}

u16 ConnectionProfile::port()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_port;
}

void ConnectionProfile::setPort(u16 in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_port = in;
}

std::string ConnectionProfile::target()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_target;
}

void ConnectionProfile::setTarget(const std::string &in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_target = in;
}

std::shared_ptr<Model::DAO::IQueueList> ConnectionProfile::list()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_list;
}

void ConnectionProfile::setList(std::shared_ptr<Model::DAO::IQueueList> in)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_list = in;
}

// Config
// public
Config::~Config()
{
    try
    {
        YAML::Node config;
        config["lastProfile"] = lastProfile;
        YAML::Node newProfiles;
        for (auto it = profiles.begin();
            it != profiles.end();
            ++it)
        {
            YAML::Node newNode;
            newNode["mode"] = it->second.mode();
            newNode["port"] = it->second.port();
            newNode["target"] = it->second.target();
            newProfiles[it->first] = newNode;
        }

        config["profiles"] = newProfiles;
        std::ofstream fout(m_configFile);
        fout << config;
        fout.close();
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to write config",
            LOG_FILE_PATH(__FILE__), __LINE__);
    }
}

// static member functions
void Config::parse()
{
    spdlog::debug("{}:{} Config::parse",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (paths.isEmpty())
    {
        spdlog::warn("{}:{} No config path found, use default",
            LOG_FILE_PATH(__FILE__), __LINE__);
        writeDefault();
        return;
    }

    Global::config.profiles.clear();
    Global::config.lastProfile = "";

    try
    {
        Global::QStrToStdStr(
            (paths.at(0) + "/flexflow.yaml"),
            Global::config.m_configFile);

        spdlog::debug("{}:{} Config file: {}",
            LOG_FILE_PATH(__FILE__), __LINE__, Global::config.m_configFile.c_str());

        YAML::Node config = YAML::LoadFile(
            Global::config.m_configFile
        );

        if (!config["profiles"])
        {
            spdlog::warn("{}:{} No profiles found, use default",
                LOG_FILE_PATH(__FILE__), __LINE__);
            writeDefault();
            return;
        }

        Global::config.lastProfile = config["currentProfile"].as<std::string>();
        /*
        Global::config.profiles =
        config["profiles"].as<std::unordered_map<std::string,
        ConnectionProfile>>();
        */
        
        for (auto it = config["profiles"].begin();
            it != config["profiles"].end();
            ++it)
        {
            ConnectionProfile profile;
            YAML::Node newNode = it->second.as<YAML::Node>();
            profile.setMode(newNode["mode"].as<u8>());
            profile.setPort(newNode["port"].as<u16>());
            profile.setTarget(newNode["target"].as<std::string>());
            Global::config.profiles[it->first.as<std::string>()].copy(profile);
        }
    }
    catch (...)
    {
        spdlog::warn("{}:{} Fail to parse config, use default",
            LOG_FILE_PATH(__FILE__), __LINE__);
        writeDefault();
    }
}

void Config::writeDefault()
{
    spdlog::debug("{}:{} Config::writeDefault",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    Global::config.lastProfile = "default";
    Global::config.profiles.clear();
    Global::config.profiles["default"].setMode(BACKEND_SQLITE);
    Global::config.profiles["default"].setPort(0);

    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    std::string target = Global::config.profiles["default"].target();
    if (paths.isEmpty())
    {
        spdlog::warn("{}:{} No app local data path found, use default",
            LOG_FILE_PATH(__FILE__), __LINE__);
        
        Global::QStrToStdStr(
            (QDir::homePath() + "/.flexflow.db"),
            target);
            Global::config.profiles["default"].setTarget(target);
        return;
    }

    Global::QStrToStdStr(
            (paths.at(0) + "/.flexflow.db"),
            target);
            Global::config.profiles["default"].setTarget(target);
}

} // namespace Gui

} // namespace Controller
