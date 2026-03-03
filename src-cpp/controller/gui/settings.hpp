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

#ifndef _CONTROLLER_GUI_SETTINGS_HPP_
#define _CONTROLLER_GUI_SETTINGS_HPP_

#include "QWidget"

#include "controller/global/defines.h"

#include "settingsthread.hpp"

namespace Ui
{
    class Settings;
} // namespace Ui

namespace Controller
{

namespace Gui
{

class Settings : public QWidget
{
    Q_OBJECT

public:
    
    explicit Settings(QWidget *parent = nullptr);

    ~Settings();

    u8 init();

signals:

    void enableUI(bool);

private slots:

    void onLoadClicked(bool);

    void onModeCurrentIndexChanged(int);

    void onCreateClicked(bool);

    void onOpenClicked(bool);

    void onSaveClicked(bool);

    void onClearClicked(bool);

    void onResetClicked(bool);

    void onConnectClicked(bool);

    void onConnectDone(u8);

    void onDisconnectDone();

private:

    Ui::Settings *m_ui;

    SettingsThread *m_thread;

    void setupProfile(const QString &);

    void setupProfile(const std::string &);

    void cleanUp();

};

} // namespace Gui

} // namespace Controller

#endif // _CONTROLLER_GUI_SETTINGS_HPP_