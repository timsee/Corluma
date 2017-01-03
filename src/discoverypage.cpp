#include "discoverypage.h"
#include "ui_discoverypage.h"

DiscoveryPage::DiscoveryPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiscoveryPage)
{
    ui->setupUi(this);
}

DiscoveryPage::~DiscoveryPage()
{
    delete ui;
}
