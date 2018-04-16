/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "listdevicesgroupwidget.h"
#include <QPainter>
#include <QStyleOption>

ListDevicesGroupWidget::ListDevicesGroupWidget(const cor::LightGroup& group,
                                               QString key,
                                               CommLayer *comm,
                                               DataLayer *data,
                                               QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());
    mData = data;
    mComm = comm;
    mSelectAllIsClear = false;

    setup(group.name, key, EListType::eGrid, true);

    mSelectAllButton = new QPushButton(this);
    mSelectAllButton->setStyleSheet("border: none;");
    connect(mSelectAllButton, SIGNAL(clicked(bool)), this, SLOT(selectAllButtonClicked(bool)));
    mSelectAllButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mSelectAllPixmap = QPixmap(":/images/selectAllIcon.png");
    mClearAllPixmap = QPixmap(":/images/clearAllIcon.png");
    mSelectAllButton->setIconSize(QSize(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio));
    mSelectAllButton->setIcon(QIcon((mSelectAllPixmap)));
    mSelectAllButton->setHidden(true);

    mTopLayout = new QHBoxLayout();
    mTopLayout->addWidget(mName);
    mTopLayout->addWidget(mSelectAllButton);
    mTopLayout->addWidget(mEditButton);
    mTopLayout->addWidget(mHiddenStateIcon);

    mTopLayout->setStretch(0, 12);
    mTopLayout->setStretch(2, 2);
    mTopLayout->setStretch(3, 2);
    mTopLayout->setStretch(4, 2);

    mGroup = group;
    updateDevices(mGroup.devices);

    mLayout->addLayout(mTopLayout);
    mLayout->addWidget(mWidget);
}

void ListDevicesGroupWidget::updateDevices(std::list<cor::Light> devices, bool removeIfNotFound) {
    mGroup.devices = devices;
    for (auto&& inputDevice : devices) {
        bool foundDevice = false;
        // check if device widget exists
        uint32_t x = 0;
        for (auto&& widget : mWidgets) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            //----------------
            // Update Widget, if it already exists
            //----------------
            cor::Light existingDevice = existingWidget->device();
            if (compareLight(inputDevice, existingDevice)) {
                foundDevice = true;
                existingWidget->updateWidget(inputDevice, mData->palette(inputDevice.palette));
            }
            ++x;
        }

        //----------------
        // Create Widget, if not found
        //----------------

        if (!foundDevice) {
            // TODO: remove edge case...
            if ((inputDevice.commType() != ECommType::eHue && inputDevice.isReachable)
                    || inputDevice.commType() == ECommType::eHue) {
                if (inputDevice.color.isValid()) {
                    ListDeviceWidget *widget = new ListDeviceWidget(inputDevice,
                                                                    mData->palette(inputDevice.palette),
                                                                    false,
                                                                    mWidgetSize,
                                                                    this);
                    connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                    connect(widget, SIGNAL(switchToggled(QString,bool)), this, SLOT(handleToggledSwitch(QString, bool)));
                    insertWidget(widget);
                }
            }
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        for (auto&& widget : mWidgets) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            bool found = false;
            for (auto device : devices) {
                if (compareLight(device, existingWidget->device())) {
                    found = true;
                }
            }
            if (!found) {
                removeWidget(existingWidget);
                break;
            }
        }
    }

    //----------------
    // Set right arrow
    //----------------
    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
    }
}

QSize ListDevicesGroupWidget::preferredSize() {
    int height = mMinimumHeight;
    if (mShowButtons && mWidgets.size() > 0) {
        int widgetHeight = std::max(mName->height(), mMinimumHeight);
        height = (mWidgets.size() / 2 * widgetHeight) + (mWidgets.size() % 2 * widgetHeight) + mMinimumHeight;
    }
    return QSize(this->parentWidget()->width(), height);
}

void ListDevicesGroupWidget::setShowButtons(bool show) {
    mShowButtons = show;
    for (auto&& device : mWidgets) {
        if (mShowButtons) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }

    if (mShowButtons) {
        mHiddenStateIcon->setPixmap(mOpenedPixmap);
        this->setFixedHeight(preferredSize().height());
    } else {
        mHiddenStateIcon->setPixmap(mClosedPixmap);
        mName->setFixedHeight(mMinimumHeight);
        this->setFixedHeight(mMinimumHeight);
    }

    emit buttonsShown(mKey, mShowButtons);
}

void ListDevicesGroupWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (auto&& existingWidget : mWidgets) {
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
        Q_ASSERT(widget);

        cor::Light widgetDevice = widget->device();
        bool found = false;
        for (auto&& device : devices) {
            if (compareLight(device, widgetDevice)) {
                numOfDevices++;
                found = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!found) {
            widget->setHighlightChecked(false);
        }
    }

    handleSelectAllButton();
    repaint();
}

void ListDevicesGroupWidget::resizeInteralWidgets() {
    handleSelectAllButton();
}

void ListDevicesGroupWidget::handleSelectAllButton() {
    if (checkedDevices().size() > 0) {
        resizeRightHandIcon(mClearAllPixmap, mSelectAllButton);
        mSelectAllIsClear = true;
        mSelectAllButton->setVisible(true);
    } else if (mShowButtons) {
        resizeRightHandIcon(mSelectAllPixmap, mSelectAllButton);
        mSelectAllIsClear = false;
        mSelectAllButton->setVisible(true);
    } else {
        mSelectAllButton->setVisible(false);
    }
}

void ListDevicesGroupWidget::selectAllButtonClicked(bool) {
    handleSelectAllButton();
    emit allButtonPressed(mKey, !mSelectAllIsClear);
}

void ListDevicesGroupWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect nameRect = mName->rect();
    QRect topRect(nameRect.x(), nameRect.y(), this->width(), nameRect.height());
    if (checkedDevices().size()) {
        painter.fillRect(topRect, QBrush(computeHighlightColor()));
    } else {
        painter.fillRect(topRect, QBrush(QColor(32, 31, 31)));
    }
}

QColor ListDevicesGroupWidget::computeHighlightColor() {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(32, 31, 31);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    float amountOfBlue = (float)checkedDevices().size() / (float)mWidgets.size();
    return QColor(amountOfBlue * difference.red() + pureBlack.red(),
                  amountOfBlue * difference.green() + pureBlack.green(),
                  amountOfBlue * difference.blue() + pureBlack.blue());
}

void ListDevicesGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mShowButtons);
}

const std::list<cor::Light> ListDevicesGroupWidget::checkedDevices() {
    std::list<cor::Light> devices;
    for (auto&& widget : mWidgets) {
        ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.push_back(existingWidget->device());
        }
    }
    return devices;
}

