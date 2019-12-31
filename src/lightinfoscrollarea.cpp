#include "lightinfoscrollarea.h"

#include <QScrollBar>
#include <QScroller>

LightInfoScrollArea::LightInfoScrollArea(QWidget* parent)
    : QScrollArea(parent),
      mScrollAreaWidget(new QWidget(this)),
      mScrollLayout(new QVBoxLayout(mScrollAreaWidget)),
      mCurrentProtocol{EProtocolType::hue} {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    setWidget(mScrollAreaWidget);

    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);
}



void LightInfoScrollArea::updateHues(std::vector<HueLight> lights) {
    for (const auto& light : lights) {
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
            hue::HueInfoWidget* widget = new hue::HueInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(clickedLight(QString)));
            connect(widget,
                    SIGNAL(changedName(EProtocolType, QString, QString)),
                    parentWidget(),
                    SLOT(nameChanged(EProtocolType, QString, QString)));
            mHueWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }
}

void LightInfoScrollArea::updateControllers(std::vector<nano::LeafController> controllers) {
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
            nano::LeafControllerInfoWidget* widget =
                new nano::LeafControllerInfoWidget(controller, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(clickedLight(QString)));
            connect(widget,
                    SIGNAL(changedName(EProtocolType, QString, QString)),
                    parentWidget(),
                    SLOT(nameChanged(EProtocolType, QString, QString)));
            mNanoleafWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }
}


void LightInfoScrollArea::updateLights(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
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
            ArduCorInfoWidget* widget = new ArduCorInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(clickedLight(QString)));
            mArduCorWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }
}


void LightInfoScrollArea::clickedLight(const QString& key) {
    bool shouldEnableDelete = true;

    switch (mCurrentProtocol) {
        case EProtocolType::hue:
            for (auto widget : mHueWidgets) {
                if (widget->checked()) {
                    widget->setChecked(false);
                    widget->hideDetails(true);
                }
            }
            for (auto widget : mHueWidgets) {
                if (widget->key() == key) {
                    if (mLastHueKey == key) {
                        shouldEnableDelete = false;
                        widget->hideDetails(true);
                        widget->setChecked(false);
                        mLastHueKey = "";
                    } else {
                        widget->hideDetails(false);
                        widget->setChecked(true);
                        mLastHueKey = key;
                    }
                }
            }
            break;
        case EProtocolType::nanoleaf:
            for (auto widget : mNanoleafWidgets) {
                if (widget->checked()) {
                    widget->setChecked(false);
                    widget->hideDetails(true);
                }
            }

            for (auto widget : mNanoleafWidgets) {
                if (widget->key() == key) {
                    if (mLastNanoleafKey == key) {
                        shouldEnableDelete = false;
                        widget->hideDetails(true);
                        widget->setChecked(false);
                        mLastNanoleafKey = "";
                    } else {
                        widget->hideDetails(false);
                        widget->setChecked(true);
                        mLastNanoleafKey = key;
                    }
                }
            }

            break;
        case EProtocolType::arduCor:
            for (auto widget : mArduCorWidgets) {
                if (widget->checked()) {
                    widget->setChecked(false);
                    widget->hideDetails(true);
                }
            }

            for (auto widget : mArduCorWidgets) {
                if (widget->key() == key) {
                    shouldEnableDelete = false;
                    if (mLastArduCorKey == key) {
                        widget->hideDetails(true);
                        widget->setChecked(false);
                        mLastArduCorKey = "";
                    } else {
                        widget->hideDetails(false);
                        widget->setChecked(true);
                        mLastArduCorKey = key;
                    }
                }
            }
            break;
        default:
            break;
    }
    emit lightClicked(key, shouldEnableDelete);
}


void LightInfoScrollArea::resize() {
    QSize widgetSize(width(), int(height() / 2));
    int widgetHeightY = 0;
    // TODO: make a better system for resizing
    // draw widgets in content region
    if (mCurrentProtocol == EProtocolType::hue) {
        for (auto widget : mHueWidgets) {
            widget->setVisible(true);
            if (widget->detailsHidden()) {
                widget->setFixedHeight(widgetSize.height() / 2);
            } else {
                widget->setFixedHeight(widgetSize.height());
            }
            widget->setGeometry(0, widgetHeightY, widgetSize.width(), widget->height());
            widgetHeightY += widget->height();
        }
    } else {
        for (auto widget : mHueWidgets) {
            widget->setVisible(false);
        }
    }
    if (mCurrentProtocol == EProtocolType::nanoleaf) {
        for (auto widget : mNanoleafWidgets) {
            widget->setVisible(true);
            if (widget->detailsHidden()) {
                widget->setFixedHeight(widgetSize.height() / 2);
            } else {
                widget->setFixedHeight(widgetSize.height());
            }
            widget->setGeometry(0, widgetHeightY, widgetSize.width(), widget->height());
            widgetHeightY += widget->height();
        }
    } else {
        for (auto widget : mNanoleafWidgets) {
            widget->setVisible(false);
        }
    }
    if (mCurrentProtocol == EProtocolType::arduCor) {
        for (auto widget : mArduCorWidgets) {
            widget->setVisible(true);
            if (widget->detailsHidden()) {
                widget->setFixedHeight(widgetSize.height() / 2);
            } else {
                widget->setFixedHeight(widgetSize.height());
            }
            widget->setGeometry(0, widgetHeightY, widgetSize.width(), widget->height());
            widgetHeightY += widget->height();
        }
    } else {
        for (auto widget : mArduCorWidgets) {
            widget->setVisible(false);
        }
    }

    setMinimumWidth(parentWidget()->minimumSizeHint().width() + verticalScrollBar()->width());
    mScrollAreaWidget->setFixedSize(geometry().width() - verticalScrollBar()->width()
                                        - contentsMargins().left() - contentsMargins().right(),
                                    widgetHeightY);
}

void LightInfoScrollArea::changeProtocol(EProtocolType protocol) {
    mCurrentProtocol = protocol;
    if (key() == "") {
        emit lightClicked(key(), false);
    } else {
        emit lightClicked(key(), true);
    }
}

std::pair<EProtocolType, QString> LightInfoScrollArea::lookupCurrentLight() {
    QString lightName;
    EProtocolType type = EProtocolType::MAX;

    switch (mCurrentProtocol) {
        case EProtocolType::hue:
            for (auto widget : mHueWidgets) {
                if (widget->key() == mLastHueKey) {
                    lightName = widget->light().name;
                    type = EProtocolType::hue;
                }
            }
            break;
        case EProtocolType::nanoleaf:
            for (auto widget : mNanoleafWidgets) {
                if (widget->key() == mLastNanoleafKey) {
                    lightName = widget->key();
                    type = EProtocolType::nanoleaf;
                }
            }
            break;
        case EProtocolType::arduCor:

            for (auto widget : mArduCorWidgets) {
                if (widget->key() == mLastNanoleafKey) {
                    lightName = widget->light().name;
                    type = EProtocolType::arduCor;
                }
            }
            break;
        default:
            break;
    }

    return std::make_pair(type, lightName);
}
