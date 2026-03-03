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

#include "QApplication"
#include "controller/global/defines.h"
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

#include "controller/gui/mainwindow.hpp"

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    UNUSED(context);
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type)
    {
        case QtDebugMsg:    spdlog::debug(localMsg.constData()); break;
        case QtInfoMsg:     spdlog::info(localMsg.constData()); break;
        case QtWarningMsg:  spdlog::warn(localMsg.constData()); break;
        case QtCriticalMsg: spdlog::error(localMsg.constData()); break;
        case QtFatalMsg:    spdlog::critical(localMsg.constData()); break;
    }
}

int main(int argc, char **argv)
{
    spdlog::cfg::load_env_levels();
    QApplication app(argc, argv);
    qInstallMessageHandler(qtMessageHandler);

    Controller::Gui::MainWindow w;
    if (w.init())
    {
        return 1;
    }

    w.show();
    return app.exec();
}
