#include "huesettingspage.h"
#include "ui_huesettingspage.h"

HueSettingsPage::HueSettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HueSettingsPage)
{
    ui->setupUi(this);

    ui->standardButton->setCheckable(true);
    connect(ui->standardButton, SIGNAL(clicked(bool)), this, SLOT(standardButtonPressed(bool)));

    ui->hueButton->setCheckable(true);
    connect(ui->hueButton, SIGNAL(clicked(bool)), this, SLOT(hueButtonPressed(bool)));
}

HueSettingsPage::~HueSettingsPage()
{
    delete ui;
}


void HueSettingsPage::standardButtonPressed(bool clicked) {
    Q_UNUSED(clicked);

    ui->standardButton->setChecked(true);
    ui->hueButton->setChecked(false);

    emit settingsPageIsStandard(true);
}

void HueSettingsPage::hueButtonPressed(bool clicked) {
    Q_UNUSED(clicked);

    ui->standardButton->setChecked(false);
    ui->hueButton->setChecked(true);

    emit settingsPageIsStandard(false);
}


// ----------------------------
// Protected
// ----------------------------


void HueSettingsPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    ui->standardButton->setChecked(false);
    ui->hueButton->setChecked(true);
}
