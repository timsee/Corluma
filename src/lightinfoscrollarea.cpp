#include "lightinfoscrollarea.h"
#include "utils/qt.h"

#include <QScrollBar>
#include <QScroller>

LightInfoScrollArea::LightInfoScrollArea(QWidget* parent)
    : QScrollArea(parent),
      mScrollAreaWidget(new QWidget(this)),
      mScrollLayout(new QVBoxLayout(mScrollAreaWidget)) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    setWidget(mScrollAreaWidget);

    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);
}

void LightInfoScrollArea::updateHues(std::vector<HueMetadata> lights) {
    for (auto widget : mHueWidgets) {
        mScrollLayout->removeWidget(widget);
        delete widget;
    }

    // clear vector
    mHueWidgets.clear();

    // sort alphabetically
    auto sortedLights = lights;
    auto lambda = [](const HueMetadata& a, const HueMetadata& b) -> bool {
        return a.name() < b.name();
    };
    std::sort(sortedLights.begin(), sortedLights.end(), lambda);
    for (const auto& light : sortedLights) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto widget : mHueWidgets) {
            if (widget->metadata().uniqueID() == light.uniqueID()) {
                widgetIndex = i;
                widget->updateLight(light);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            hue::HueInfoWidget* widget = new hue::HueInfoWidget(light, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(cor::LightID)), this, SLOT(clickedLight(cor::LightID)));
            connect(widget,
                    SIGNAL(clickedDelete(cor::LightID, QString)),
                    parentWidget(),
                    SLOT(deleteButtonPressed(cor::LightID, QString)));

            connect(widget,
                    SIGNAL(clickedChangeName(cor::LightID, QString)),
                    parentWidget(),
                    SLOT(changeNamePressed(cor::LightID, QString)));

            mHueWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }
    resize();
}

void LightInfoScrollArea::addLight(const HueMetadata& light) {
    /// get all the metadata that currently exists
    std::vector<HueMetadata> metadataVector(1, light);
    for (auto widget : mHueWidgets) {
        metadataVector.push_back(widget->metadata());
    }
    updateHues(metadataVector);
}

void LightInfoScrollArea::deleteLight(const cor::LightID& lightID) {
    hue::HueInfoWidget* widgetToDelete = nullptr;
    for (auto widget : mHueWidgets) {
        if (widget->key() == lightID.toString()) {
            widgetToDelete = widget;
            break;
            mScrollLayout->removeWidget(widget);

            delete widget;
            return;
        }
    }
    if (widgetToDelete != nullptr) {
        mScrollLayout->removeWidget(widgetToDelete);
        // find widget in the vector
        auto result = std::find(mHueWidgets.begin(), mHueWidgets.end(), widgetToDelete);
        if (result != mHueWidgets.end()) {
            mHueWidgets.erase(result);
        }
        delete widgetToDelete;
    }
    resize();
}


void LightInfoScrollArea::clickedLight(const cor::LightID& key) {
    bool shouldEnableDelete = true;
    for (auto widget : mHueWidgets) {
        if (widget->checked()) {
            widget->setChecked(false);
            widget->hideDetails(true);
        }
    }
    for (auto widget : mHueWidgets) {
        if (widget->key() == key.toString()) {
            if (mLastHueKey == key.toString()) {
                shouldEnableDelete = false;
                widget->hideDetails(true);
                widget->setChecked(false);
                mLastHueKey = "";
            } else {
                widget->hideDetails(false);
                widget->setChecked(true);
                mLastHueKey = key.toString();
            }
        }
    }
    emit lightClicked(cor::LightID(key), shouldEnableDelete);
}


void LightInfoScrollArea::resize() {
    QSize widgetSize(width(), int(height() / 2));
    int widgetHeightY = 0;
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

    setMinimumWidth(parentWidget()->minimumSizeHint().width() + verticalScrollBar()->width());
    mScrollAreaWidget->setFixedSize(
        cor::guardAgainstNegativeSize(geometry().width() - verticalScrollBar()->width()
                                      - contentsMargins().left() - contentsMargins().right()),
        widgetHeightY);
}


QString LightInfoScrollArea::lookupCurrentLight() {
    for (auto widget : mHueWidgets) {
        if (widget->key() == mLastHueKey) {
            return widget->metadata().name();
        }
    }
    return {};
}
