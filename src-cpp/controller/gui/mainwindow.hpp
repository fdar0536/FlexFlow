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

#ifndef _CONTROLLER_GUI_MAINWINDOW_HPP_
#define _CONTROLLER_GUI_MAINWINDOW_HPP_

#include "QMainWindow"
#include "QSystemTrayIcon"

#include "controller/global/defines.h"

namespace Ui
{
    class MainWindow;
} // namespace Ui

class QMenu;
class QAction;

namespace Controller
{

namespace Gui
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    u8 init();

protected:

    void closeEvent(QCloseEvent *e) override;

private slots:

    void onSettingsTriggered();

    void onAboutTriggered();

    void onExitTriggered();

    // tray icon
#ifndef __APPLE__
    void onIconActivated(QSystemTrayIcon::ActivationReason);
#endif // __APPLE__

private:

    Ui::MainWindow *m_ui;

    QSystemTrayIcon *m_icon;

    QMenu *m_menu;

    QAction *m_show;

    QAction *m_exit;

    QAction *m_lastAction;

    void setupMenuBar();

    u8 setupTrayIcon(QIcon &);

    void cleanUp();
};

} // namespace Gui

} // namespace Controller

#endif // _CONTROLLER_GUI_MAINWINDOW_HPP_
