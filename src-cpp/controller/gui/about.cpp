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

#include "QMessageBox"
#include "spdlog/spdlog.h"

#include "controller/global/global.hpp"

#include "../../view/ui_about.h"
#include <config.h>
#include "about.hpp"

namespace Controller
{

namespace Gui
{

// implement class About
About::About(QWidget *parent):
    QWidget(parent),
    m_ui(nullptr)
{}

About::~About()
{
    if (m_ui)
    {
        delete m_ui;
        m_ui = nullptr;
    }
}

u8 About::init()
{
    spdlog::debug("{}:{} About::init",
        LOG_FILE_PATH(__FILE__), __LINE__);

    m_ui = new (std::nothrow) Ui::About;
    if (!m_ui)
    {
        spdlog::error("{}:{} Fail to allocate memory",
            LOG_FILE_PATH(__FILE__), __LINE__);
        return 1;
    }

    m_ui->setupUi(this);

    m_ui->version->setText(FF_VERSION);
    m_ui->branch->setText(FF_BRANCH);
    m_ui->commit->setText(FF_COMMIT);
    QPixmap pixmap(":/original-icon.png");
    pixmap = pixmap.scaled(300, 300,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    m_ui->icon->setPixmap(pixmap);

    connect(m_ui->aboutQt,
        &QPushButton::clicked,
        this,
        &About::onAboutQtClicked
    );
    return 0;
}

// private slots
void About::onAboutQtClicked(bool)
{
    spdlog::debug("{}:{} About::onAboutQtClicked",
        LOG_FILE_PATH(__FILE__), __LINE__);

    QMessageBox::aboutQt(this);
}

} // namespace Gui

} // namespace Controller