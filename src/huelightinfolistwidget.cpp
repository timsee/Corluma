/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

#include "huelightinfolistwidget.h"
#include "corlumautils.h"

HueLightInfoListWidget::HueLightInfoListWidget(QWidget *parent) : QWidget(parent) {
    mTopWidget = new CorlumaTopWidget("Hue Lights", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    mTopWidget->setFontPoint(20);

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    mScrollArea->setWidget(mScrollAreaWidget);

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);

    mDeleteButton = new QPushButton("Delete", this);
    mDeleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    connect(mDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonPressed(bool)));
    mDeleteButton->setFixedWidth(this->width() * 0.5f);
    mDeleteButton->setEnabled(false);
    mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");

    mMainLayout = new QVBoxLayout(this);

    mMainLayout->addWidget(mTopWidget,    2);
    mMainLayout->addWidget(mScrollArea,   14);
    mMainLayout->addWidget(mDeleteButton, 1, Qt::AlignHCenter);

    mLastKey = "";
}

void HueLightInfoListWidget::updateLights(std::list<SHueLight> lights) {
    for (auto light : lights) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto widget : mWidgets) {
            if (widget->light().deviceIndex == light.deviceIndex) {
                widgetIndex = i;
                widget->updateLight(light);
            }
            ++i;
        }
        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            HueLightInfoWidget *widget = new HueLightInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(lightInfoWidgetClicked(QString)));
            connect(widget, SIGNAL(changedName(QString, QString)), this, SLOT(nameChanged(QString, QString)));
            mWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }

    resize(true);
}

void HueLightInfoListWidget::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(size.width() * 0.125f,
                          size.height() * 0.125f,
                          size.width() * 0.75f,
                          size.height() * 0.75f);
    }
    // resize scroll area
    mScrollAreaWidget->setFixedWidth(mScrollArea->width() * 0.9f);
    QSize widgetSize(this->width()  * 0.9f, this->height() / 3.66);
    uint32_t yPos = 0;
    //TODO: make a better system for resizing
    // draw widgets in content region
    for (auto widget : mWidgets) {
        widget->setHeight(widgetSize.height());
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widget->height());
        yPos += widget->height();
    }
    mDeleteButton->setFixedWidth(this->width() * 0.5f);
    mScrollAreaWidget->setFixedHeight(yPos);
}


void HueLightInfoListWidget::deleteButtonPressed(bool) {
    QMessageBox::StandardButton reply;
    SHueLight light;
    for (auto widget : mWidgets) {
        if (widget->key().compare(mLastKey) == 0) {
            light = widget->light();
        }
    }
    QString text = "Delete " + light.name + "? This will remove it from the Hue Bridge.";
    reply = QMessageBox::question(this, "Delete?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // remove from widget TODO
//        for (auto widget : mWidgets) {
//            if (widget->key().compare(mLastKey) == 0) {
//                light = widget->light();
//                break;
//            }
//        }
        // signal to remove from app
        emit hueDeleted(mLastKey);
    }
}

void HueLightInfoListWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void HueLightInfoListWidget::closePressed(bool) {
    emit pressedClose();
}

void HueLightInfoListWidget::lightInfoWidgetClicked(QString key) {
   // qDebug() << " clicked " << key;
    for (auto widget : mWidgets) {
        if (widget->checked()) {
            widget->setChecked(false);
            widget->hideDetails(true);
        }
    }
    for (auto widget : mWidgets) {
        if (widget->key().compare(key) == 0) {
            if (mLastKey.compare(key) == 0) {
                widget->hideDetails(true);
                widget->setChecked(false);
                mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");
                mLastKey = "";
            } else {
                widget->hideDetails(false);
                widget->setChecked(true);
                mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");
                mLastKey = key;
            }
        }
    }

    if (mLastKey.compare("") == 0) {
        mDeleteButton->setEnabled(false);
    } else {
        mDeleteButton->setEnabled(true);
    }

    resize(false);
}
