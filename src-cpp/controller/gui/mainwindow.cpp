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

#include <new>

#include "QAction"
#include "QCloseEvent"
#include "QMenu"
#include "QMessageBox"
#include "QStandardPaths"
#include "QSystemTrayIcon"

#include "controller/gui/config.hpp"
#include "spdlog/spdlog.h"

#include "../../controller/global/global.hpp"

#include "about.hpp"
#include "global.hpp"
#include "settings.hpp"

#include "../../view/ui_mainwindow.h"
#include "mainwindow.hpp"

namespace Controller
{

namespace Gui
{

// implement class MainWindow
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_ui(nullptr),
    m_icon(nullptr),
    m_menu(nullptr),
    m_show(nullptr),
    m_exit(nullptr),
    m_lastAction(nullptr)
{}

MainWindow::~MainWindow()
{
    cleanUp();
}

u8 MainWindow::init()
{
    spdlog::debug("{}:{} MainWindow::init",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    // parse config first
    Config::parse();

    m_ui = new (std::nothrow) Ui::MainWindow;
    setWindowTitle("Flex Flow");
    QIcon icon(":/original-icon.png");
    setWindowIcon(icon);

    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        cleanUp();
        return 1;
    }

    m_ui->setupUi(this);
    
    setupMenuBar();

    if (setupTrayIcon(icon))
    {
        spdlog::error("{}:{} Fail to setup tray icon",
            LOG_FILE_PATH(__FILE__), __LINE__);
        cleanUp();
        return 1;
    }

    // setup initial central widget
    Settings *s = new (std::nothrow) Settings(this);
    if (!s)
    {
        spdlog::error("Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        QMessageBox::critical(this, "Error", "Fail to allocate memory");
        return 1;
    }

    if (s->init())
    {
        spdlog::error("Fail to initialize",
            LOG_FILE_PATH(__FILE__), __LINE__);
        QMessageBox::critical(this, "Error", "Fail to initialize");
        delete s;
        return 1;
    }
    
    setCentralWidget(s);

    connect(s,
        &Settings::enableUI,
        this,
        &MainWindow::setEnabled
    );

    m_lastAction = m_ui->settings;
    m_lastAction->setEnabled(false);
    return 0;
}

// protected member functions
void MainWindow::closeEvent(QCloseEvent *e)
{
    spdlog::debug("{}:{} MainWindow::closeEvent",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        e->ignore();
        hide();
        return;
    }

    onExitTriggered();
}

// private slots
#define SET_CENTRAL_WIDGET(T, LAST_ACTION) \
    spdlog::debug("{}:{} SET_CENTRAL_WIDGET", \
        LOG_FILE_PATH(__FILE__), __LINE__); \
    \
    m_lastAction->setEnabled(true); \
    \
    T *t = new (std::nothrow) T(this); \
    if (!t) \
    { \
        spdlog::error("Fail to allocate memory", \
            LOG_FILE_PATH(__FILE__), __LINE__); \
        QMessageBox::critical(this, "Error", "Fail to allocate memory"); \
        return; \
    } \
    \
    if (t->init()) \
    { \
        spdlog::error("Fail to initialize", \
            LOG_FILE_PATH(__FILE__), __LINE__); \
        QMessageBox::critical(this, "Error", "Fail to initialize"); \
        delete t; \
        return; \
    } \
    \
    QWidget *w = takeCentralWidget(); \
    delete w; \
    setCentralWidget(t); \
    m_lastAction = LAST_ACTION; \
    m_lastAction->setEnabled(false);

void MainWindow::onSettingsTriggered()
{
    spdlog::debug("{}:{} MainWindow::onSettingsTriggered",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    SET_CENTRAL_WIDGET(Settings, m_ui->settings);

    connect(t,
        &Settings::enableUI,
        this,
        &MainWindow::setEnabled
    );
}

void MainWindow::onAboutTriggered()
{
    spdlog::debug("{}:{} MainWindow::onAboutTriggered",
        LOG_FILE_PATH(__FILE__), __LINE__);
    SET_CENTRAL_WIDGET(About, m_ui->about);
}

void MainWindow::onExitTriggered()
{
    spdlog::debug("{}:{} MainWindow::onExitTriggered",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    if (
        QMessageBox::question(this,
            "Exit",
            "Are you sure to exit? All process "
            "in queue will be killed.",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::No
        ) == QMessageBox::Yes
    )
    {
        QApplication::quit();
    }
}

// tray icon
#ifndef __APPLE__
void MainWindow::onIconActivated(QSystemTrayIcon::ActivationReason r)
{
    spdlog::debug("{}:{} MainWindow::onIconActivated",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (r == QSystemTrayIcon::Trigger)
    {
        show();
    }
    else if (r == QSystemTrayIcon::Context)
    {
        m_menu->exec(QCursor::pos());
    }
}
#endif // __APPLE__

// private member functions
void MainWindow::setupMenuBar()
{
    spdlog::debug("{}:{} MainWindow::setupMenu",
        LOG_FILE_PATH(__FILE__), __LINE__);
    
    connect(m_ui->settings,
        &QAction::triggered,
        this,
        &MainWindow::onSettingsTriggered
    );

    connect(m_ui->about,
        &QAction::triggered,
        this,
        &MainWindow::onAboutTriggered);
    
    connect(m_ui->exit,
        &QAction::triggered,
        this,
        &MainWindow::onExitTriggered
    );
}

u8 MainWindow::setupTrayIcon(QIcon &icon)
{
    spdlog::debug("{}:{} MainWindow::setupTrayIcon",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_icon = new (std::nothrow) QSystemTrayIcon(this);
        if (!m_icon)
        {
            spdlog::error("Fail to allocate memory",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_icon->setIcon(icon);
        m_icon->setToolTip("Flex Flow");

        m_menu = new (std::nothrow) QMenu(this);
        if (!m_menu)
        {
            spdlog::error("Fail to allocate memory",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_show = new (std::nothrow) QAction("Show", this);
        if (!m_show)
        {
            spdlog::error("Fail to allocate memory",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_exit = new (std::nothrow) QAction("Exit", this);
        if (!m_exit)
        {
            spdlog::error("Fail to allocate memory",
                LOG_FILE_PATH(__FILE__), __LINE__);
            return 1;
        }

        m_menu->addAction(m_show);
        m_menu->addAction(m_exit);

        m_icon->setContextMenu(m_menu);
        m_icon->show();

        // signals
#ifndef __APPLE__
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &MainWindow::onIconActivated);
#endif // __APPLE__

        connect(m_show,
                &QAction::triggered,
                this,
                &MainWindow::show);
        
        connect(m_exit,
                &QAction::triggered,
                this,
                &MainWindow::onExitTriggered);
    }

    return 0;
}

void MainWindow::cleanUp()
{
    spdlog::debug("{}:{} MainWindow::cleanUp",
        LOG_FILE_PATH(__FILE__), __LINE__);

    if (m_ui)
    {
        delete m_ui;
        m_ui = nullptr;
    }

    if (m_icon)
    {
        delete m_icon;
        m_icon = nullptr;
    }

    if (m_menu)
    {
        delete m_menu;
        m_menu = nullptr;
    }

    if (m_show)
    {
        delete m_show;
        m_show = nullptr;
    }

    if (m_exit)
    {
        delete m_exit;
        m_exit = nullptr;
    }
}

} // namespace Gui

} // namespace Controller