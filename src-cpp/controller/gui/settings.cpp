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

#include "QFileDialog"
#include "QMessageBox"
#include "controller/gui/config.hpp"
#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"
#include "controller/gui/global.hpp"

#include "../../view/ui_settings.h"
#include "settings.hpp"

namespace Controller
{

namespace Gui
{

Settings::Settings(QWidget *parent):
    QWidget(parent),
    m_ui(nullptr),
    m_thread(nullptr)
{}

Settings::~Settings()
{
    cleanUp();
}

// public member functions
u8 Settings::init()
{
    spdlog::debug("{}:{} Settings::init",
        LOG_FILE_PATH(__FILE__), __LINE__);

    m_ui = new (std::nothrow) Ui::Settings; 
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    m_ui->setupUi(this);

    m_thread = new (std::nothrow) SettingsThread(this);
    if (!m_thread)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        cleanUp();
        return 1;
    }

    for (auto it = Global::config.profiles.begin();
        it != Global::config.profiles.end();
        ++it)
    {
        m_ui->profiles->addItem(QString::fromStdString(it->first));
    }

    m_ui->profiles->setCurrentIndex(
        m_ui->profiles->findText(
            QString::fromStdString(Global::config.lastProfile)));

    setupProfile(Global::config.lastProfile);

    connect(m_ui->load,
        &QPushButton::clicked,
        this,
        &Settings::onLoadClicked
    );

    connect(m_ui->mode,
        &QComboBox::currentIndexChanged,
        this,
        &Settings::onModeCurrentIndexChanged
    );

    connect(m_ui->create,
        &QPushButton::clicked,
        this,
        &Settings::onCreateClicked
    );

    connect(m_ui->open,
        &QPushButton::clicked,
        this,
        &Settings::onOpenClicked
    );

    connect(m_ui->save,
        &QPushButton::clicked,
        this,
        &Settings::onSaveClicked
    );

    connect(m_ui->clear,
        &QPushButton::clicked,
        this,
        &Settings::onClearClicked
    );

    connect(m_ui->reset,
        &QPushButton::clicked,
        this,
        &Settings::onResetClicked
    );

    connect(m_ui->connect,
        &QPushButton::clicked,
        this,
        &Settings::onConnectClicked
    );
    
    connect(m_thread,
        &SettingsThread::doConnectDone,
        this,
        &Settings::onConnectDone
    );

    connect(m_thread,
        &SettingsThread::doDisconnectDone,
        this,
        &Settings::onDisconnectDone
    );

    m_ui->reset->setEnabled(Global::config.currentProfile.list() != nullptr);
    m_ui->connect->setEnabled(Global::config.currentProfile.list() == nullptr);
    return 0;
}

// private slots
void Settings::onLoadClicked(bool)
{
    spdlog::debug("{}:{} Settings::onLoadClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    setupProfile(m_ui->profiles->currentText());
}

void Settings::onModeCurrentIndexChanged(int in)
{
    spdlog::debug("{}:{} Settings::onModeCurrentIndexChanged",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    m_ui->create->setEnabled(in);
    m_ui->open->setEnabled(in);
}

void Settings::onCreateClicked(bool)
{
    spdlog::debug("{}:{} Settings::onCreateClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);

    QString res = QFileDialog::getSaveFileName(
        this,
        "Select file to create",
        QDir::homePath(),
        "*.db"
    );

    if (res.isEmpty())
    {
        spdlog::warn("{}:{} No file selected",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    m_ui->target->setText(res);
}

void  Settings::onOpenClicked(bool)
{
    spdlog::debug("{}:{} Settings::onOpenClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    QString res = QFileDialog::getOpenFileName(
        this,
        "Select file to create",
        QDir::homePath(),
        "*.db"
    );

    if (res.isEmpty())
    {
        spdlog::warn("{}:{} No file selected",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    m_ui->target->setText(res);
}

void Settings::onSaveClicked(bool)
{
    spdlog::debug("{}:{} Settings::onSaveClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (m_ui->name->text().isEmpty())
    {
        spdlog::warn("{}:{} No profile name",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    if (m_ui->target->text().isEmpty())
    {
        spdlog::warn("{}:{} No target",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return;
    }

    std::string profileName = m_ui->name->text().toStdString();
    Global::config.profiles[profileName].setMode(
        static_cast<u8>(m_ui->mode->currentIndex()));
    Global::config.profiles[profileName].setPort(
        m_ui->port->value());
    Global::config.profiles[profileName].setTarget(
        m_ui->target->text().toStdString());
    Global::config.lastProfile = profileName;
}

void Settings::onClearClicked(bool)
{
    spdlog::debug("{}:{} Settings::onClearClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    m_ui->name->clear();
    m_ui->mode->setCurrentIndex(1);
    m_ui->port->setValue(0);
    m_ui->target->clear();
}

void Settings::onResetClicked(bool)
{
    spdlog::debug("{}:{} Settings::onResetClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (
        QMessageBox::question(this,
            "Disconnect/Reset",
            "Are you sure to disconnect/reset? Unsaved session "
            "will be killed.",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::No
        ) == QMessageBox::No
    )
    {
        return;
    }

    emit enableUI(false);
    if (m_thread->startDisconnect())
    {
        QMessageBox::critical(this, "Error", "Fail to disconnect");
        spdlog::error("{}:{} Fail to disconnect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        emit enableUI(true);
    }
}

void Settings::onConnectClicked(bool)
{
    spdlog::debug("{}:{} Settings::onConnectClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    emit enableUI(false);
    Global::config.currentProfile.setMode(
        static_cast<u8>(m_ui->mode->currentIndex()));
    Global::config.currentProfile.setPort(
        m_ui->port->value());
    Global::config.currentProfile.setTarget(
        m_ui->target->text().toStdString());
    
    if (m_thread->startConnect())
    {
        QMessageBox::critical(this, "Error", "Fail to connect");
        spdlog::error("{}:{} Fail to connect",
            LOG_FILE_PATH(__FILE__), __LINE__);
        emit enableUI(true);
        return;
    }
}

void Settings::onConnectDone(u8 in)
{
    spdlog::debug("{}:{} Settings::onConnectDone",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (in)
    {
        QMessageBox::critical(this, "Error", "Fail to connect");
    }
    
    emit enableUI(true);
    m_ui->connect->setEnabled(in != 0);
    m_ui->reset->setEnabled(in == 0);
}

void Settings::onDisconnectDone()
{
    spdlog::debug("{}:{} Settings::onDisconnectDone",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    emit enableUI(true);
}

// private member functions
void Settings::setupProfile(const QString &in)
{
    spdlog::debug("{}:{} Settings::setupProfile",
        LOG_FILE_PATH(__FILE__), __LINE__);
    std::string out;
    Global::QStrToStdStr(in, out);
    setupProfile(out);
}

void Settings::setupProfile(const std::string &in)
{
    spdlog::debug("{}:{} Settings::setupProfile",
        LOG_FILE_PATH(__FILE__), __LINE__);

    ConnectionProfile profile;
    Global::config.profiles[in].copy(profile);
    Global::config.lastProfile = in;

    m_ui->name->setText(QString::fromStdString(in));
    m_ui->mode->setCurrentIndex(profile.mode());
    m_ui->port->setValue(profile.port());
    m_ui->target->setText(QString::fromStdString(profile.target()));

    m_ui->reset->setEnabled(profile.list() != nullptr);
    m_ui->connect->setEnabled(profile.list() == nullptr);
}

void Settings::cleanUp()
{
    spdlog::debug("{}:{} Settings::cleanUp",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (m_ui)
    {
        delete m_ui;
        m_ui = nullptr;
    }

    if (m_thread)
    {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
}

} // namespace Gui

} // namespace Controller
