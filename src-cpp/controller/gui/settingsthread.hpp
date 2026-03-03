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

#ifndef _CONTROLLER_GUI_SETTINGSTHREAD_HPP_
#define _CONTROLLER_GUI_SETTINGSTHREAD_HPP_

#include "QThread"

#include "controller/global/defines.h"

namespace Controller
{

namespace Gui
{

class SettingsThread : public QThread
{

    Q_OBJECT

public:

    explicit SettingsThread(QObject *parent = nullptr);

    ~SettingsThread();

    u8 startConnect();

    u8 startDisconnect();

signals:

    void doDisconnectDone();

    void doConnectDone(u8);

protected:

    void run() override;

private:

    void doConnect();

    void doDisconnect();

    using HandlerFunc = void (SettingsThread::*)();

    using Mode = enum
    {
        CONNECT,
        DISCONNECT,
        INVALID
    };

    Mode m_mode = INVALID;

    std::mutex m_mutex;

    HandlerFunc handler[2] =
    {
        &SettingsThread::doConnect,
        &SettingsThread::doDisconnect
    };

}; // class SettingsThread

} // namespace Gui

} // namespace Controller

#endif // _CONTROLLER_GUI_SETTINGSTHREAD_HPP_