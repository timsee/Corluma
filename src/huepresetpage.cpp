/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "huepresetpage.h"
#include "ui_huepresetpage.h"

HuePresetPage::HuePresetPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HuePresetPage)
{
    ui->setupUi(this);
}

HuePresetPage::~HuePresetPage()
{
    delete ui;
}
