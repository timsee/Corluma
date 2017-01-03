#include "listgroupwidget.h"

ListGroupWidget::ListGroupWidget(const QString& name,
                                 const std::list<SLightDevice>& devices,
                                 const std::vector<std::vector<QColor> >& colors,
                                 QPixmap statePixmap,
                                 int width, int height,
                                 bool isMood,
                                 QWidget *parent) : QWidget(parent) {
    Q_UNUSED(width);
    Q_UNUSED(isMood);

    // setup icon
    QString reachableStlyeSheet = "background:rgba(0, 0, 0, 0%); font: bold; color: #333;";
    QString backgroundStyleSheet = "background:rgba(0, 0, 0, 0%); font: bold;";

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMaximumHeight(height);

    // setup main label
    mName = new QLabel(this);
    mName->setWordWrap(true);
    mName->setText(name);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    SLightDevice device = *devices.begin();
    if(!device.isReachable) {
        mName->setStyleSheet(reachableStlyeSheet);
    } else {
        mName->setStyleSheet(backgroundStyleSheet);
    }

    int previewNumber = 5;
    mIconData = std::vector<IconData>(previewNumber, IconData(64, 64));
    mPreviews = std::vector<QLabel*>(previewNumber, nullptr);

    for (int i = 0; i < previewNumber; ++i) {
        mPreviews[i] = new QLabel;
#ifndef MOBILE_BUILD
        mPreviews[i]->setMaximumWidth(mPreviews[i]->height());
#endif
        mPreviews[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mPreviews[i]->setStyleSheet(backgroundStyleSheet);
    }


    mStatusIcon = new QLabel(this);
    mStatusIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
#ifndef MOBILE_BUILD
    mStatusIcon->setMaximumWidth(mStatusIcon->height());
#endif
    mStatusIcon->setStyleSheet(backgroundStyleSheet);
    mStatusIcon->setPixmap(statePixmap.scaled(height * 0.5f,
                                              height * 0.5f,
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation));

    // setup layout
    mLayout = new QHBoxLayout(this);
    mLayout->addWidget(mName);
    for (auto label : mPreviews) {
        mLayout->addWidget(label);
    }
    mLayout->addWidget(mStatusIcon);


    mLayout->setContentsMargins(5,5,5,5);
    setLayout(mLayout);
    this->setStyleSheet(backgroundStyleSheet);

    int temp = 0;
    mLayout->setStretch(temp, 6);
    temp++;
    for (auto label : mPreviews) {
        Q_UNUSED(label);
        mLayout->setStretch(temp, 2);
        temp++;
    }
    mLayout->setStretch(temp, 2);


    int index = 0;
    for (auto&& device : devices) {
        if (index < previewNumber) {
            if (device.lightingRoutine <= utils::ELightingRoutineSingleColorEnd ) {
                mIconData[index].setSingleLightingRoutine(device.lightingRoutine, device.color);
                QPixmap iconRendered = mIconData[index].renderAsQPixmap();
                mPreviews[index]->setMaximumHeight(height);
                mPreviews[index]->setPixmap(iconRendered.scaled(height * 0.5f,
                                                            height * 0.5f,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::FastTransformation));
            } else {
                mIconData[index].setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors[(int)device.colorGroup]);
                QPixmap iconRendered = mIconData[index].renderAsQPixmap();
                mPreviews[index]->setPixmap(iconRendered.scaled(this->height() * 0.5f,
                                                            this->height() * 0.5f,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::FastTransformation));
            }
            index++;
        }
    }
}



