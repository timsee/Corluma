/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QMessageBox>

#include "lightinfolistwidget.h"
#include "utils/qt.h"


LightInfoListWidget::LightInfoListWidget(QWidget *parent) : QWidget(parent) {
    mTopWidget = new cor::TopWidget("Light Info", ":images/closeX.png", this);
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
    mDeleteButton->setFixedWidth(int(this->width() * 0.5f));
    mDeleteButton->setEnabled(false);
    mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");

    mMainLayout = new QVBoxLayout(this);

    mMainLayout->addWidget(mTopWidget,    2);
    mMainLayout->addWidget(mScrollArea,   14);
    mMainLayout->addWidget(mDeleteButton, 1, Qt::AlignHCenter);

    mLastKey = "";
}

void LightInfoListWidget::updateHues(std::list<HueLight> lights) {
    for (auto light : lights) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto widget : mHueWidgets) {
            if (widget->light().uniqueID() == light.uniqueID()) {
                widgetIndex = i;
                widget->updateLight(light);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            hue::HueInfoWidget *widget = new hue::HueInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(lightInfoWidgetClicked(QString)));
            connect(widget, SIGNAL(changedName(EProtocolType, QString, QString)), this, SLOT(nameChanged(EProtocolType, QString, QString)));
            mHueWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }

    resize(true);
}

void LightInfoListWidget::updateControllers(std::list<nano::LeafController> controllers) {
    for (auto controller : controllers) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto widget : mNanoleafWidgets) {
            if (widget->controller().serialNumber == controller.serialNumber) {
                widgetIndex = i;
                widget->updateController(controller);
            }
            ++i;
        }
        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            nano::LeafControllerInfoWidget *widget = new nano::LeafControllerInfoWidget(controller, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(lightInfoWidgetClicked(QString)));
            connect(widget, SIGNAL(changedName(EProtocolType, QString, QString)), this, SLOT(nameChanged(EProtocolType, QString, QString)));
            mNanoleafWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }

    resize(true);
}


void LightInfoListWidget::updateLights(std::list<cor::Light> lights) {
    for (auto light : lights) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto widget : mArduCorWidgets) {
            if (widget->light().uniqueID() == light.uniqueID()) {
                widgetIndex = i;
                widget->updateLight(light);
            }
            ++i;
        }
        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            ArduCorInfoWidget *widget = new ArduCorInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(lightInfoWidgetClicked(QString)));
            mArduCorWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }

    resize(true);
}

void LightInfoListWidget::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(int(size.width() * 0.125f),
                          int(size.height() * 0.125f),
                          int(size.width() * 0.75f),
                          int(size.height() * 0.75f));
    }
    // resize scroll area
    mScrollAreaWidget->setFixedWidth(int(mScrollArea->width() * 0.9f));
    QSize widgetSize(int(this->width()  * 0.9f),
                     int(this->height() / 3.66f));
    int yPos = 0;
    //TODO: make a better system for resizing
    // draw widgets in content region
    for (auto widget : mHueWidgets) {
        widget->setHeight(widgetSize.height());
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widget->height());
        yPos += widget->height();
    }
    for (auto widget : mNanoleafWidgets) {
        widget->setHeight(widgetSize.height());
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widget->height());
        yPos += widget->height();
    }
    for (auto widget : mArduCorWidgets) {
        widget->setHeight(widgetSize.height());
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widget->height());
        yPos += widget->height();
    }
    mDeleteButton->setFixedWidth(int(this->width() * 0.5f));
    mScrollAreaWidget->setFixedHeight(yPos);
}


void LightInfoListWidget::deleteButtonPressed(bool) {
    QMessageBox::StandardButton reply;
    QString lightName;
    EProtocolType type = EProtocolType::MAX;
    for (auto widget : mHueWidgets) {
        if (widget->key() == mLastKey) {
            lightName = widget->light().name;
            type = EProtocolType::hue;
        }
    }

    for (auto widget : mArduCorWidgets) {
        if (widget->key() == mLastKey) {
            lightName = widget->light().name;
            type = EProtocolType::arduCor;
        }
    }

    for (auto widget : mNanoleafWidgets) {
        if (widget->key() == mLastKey) {
            lightName = widget->controller().name;
            type = EProtocolType::nanoleaf;
        }
    }
    if (type != EProtocolType::MAX) {
        QString text;
        switch (type) {
            case EProtocolType::arduCor:
                text = "Delete " + lightName + "? This will remove this light and all others using the same arduino or raspberry pi from the app memory.";
                break;
            case EProtocolType::hue:
                text = "Delete " + lightName + "? This will remove it from the Hue Bridge.";
                break;
            case EProtocolType::nanoleaf:
                text = "Delete " + lightName + "? This will remove it from the app memory.";
                break;
            case EProtocolType::MAX:
                text = "LIGHT NOT FOUND";
                break;
        }
        reply = QMessageBox::question(this, "Delete?", text,
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // signal to remove from app
            emit deleteLight(mLastKey);
        }
    }
}

void LightInfoListWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void LightInfoListWidget::closePressed(bool) {
    emit pressedClose();
}

void LightInfoListWidget::lightInfoWidgetClicked(QString key) {
    bool shouldEnableDelete = true;

   // qDebug() << " clicked " << key;
    for (auto widget : mHueWidgets) {
        if (widget->checked()) {
            widget->setChecked(false);
            widget->hideDetails(true);
        }
    }
    for (auto widget : mNanoleafWidgets) {
        if (widget->checked()) {
            widget->setChecked(false);
            widget->hideDetails(true);
        }
    }
    for (auto widget : mArduCorWidgets) {
        if (widget->checked()) {
            widget->setChecked(false);
            widget->hideDetails(true);
        }
    }
    for (auto widget : mHueWidgets) {
        if (widget->key().compare(key) == 0) {
            if (mLastKey.compare(key) == 0) {
                widget->hideDetails(true);
                widget->setChecked(false);
                mLastKey = "";
            } else {
                widget->hideDetails(false);
                widget->setChecked(true);
                mLastKey = key;
            }
        }
    }
    for (auto widget : mNanoleafWidgets) {
        if (widget->key().compare(key) == 0) {
            if (mLastKey.compare(key) == 0) {
                widget->hideDetails(true);
                widget->setChecked(false);
                mLastKey = "";
            } else {
                widget->hideDetails(false);
                widget->setChecked(true);
                mLastKey = key;
            }
        }
    }

    for (auto widget : mArduCorWidgets) {
        if (widget->key().compare(key) == 0) {
            if (mLastKey.compare(key) == 0) {
                widget->hideDetails(true);
                widget->setChecked(false);
                mLastKey = "";
            } else {
                shouldEnableDelete = true;
                widget->hideDetails(false);
                widget->setChecked(true);
                mLastKey = key;
            }
        }
    }

    if (mLastKey.compare("") == 0) {
        mDeleteButton->setEnabled(false);
        mDeleteButton->setStyleSheet("background-color:rgb(45,30,30);");
    } else if (shouldEnableDelete){
        mDeleteButton->setEnabled(true);
        mDeleteButton->setStyleSheet("background-color:rgb(110,30,30);");
    }

    resize(false);
}
