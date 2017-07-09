/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "listdevicesgroupwidget.h"
#include <QPainter>
#include <QStyleOption>

ListDevicesGroupWidget::ListDevicesGroupWidget(const QString& name,
                                               std::list<SLightDevice> devices,
                                               QString key,
                                               CommLayer *comm,
                                               DataLayer *data,
                                               bool hideEdit,
                                               QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());
    mCheckedDevices = 0;
    mData = data;
    mComm = comm;

    setup(name, key, hideEdit);

    mClearAllButton = new QPushButton(this);
    mClearAllButton->setStyleSheet("border: none;");
    connect(mClearAllButton, SIGNAL(clicked(bool)), this, SLOT(clearButtonClicked(bool)));
    mClearAllButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mClearAllPixmap = QPixmap(":/images/clearAllIcon.png");
    mClearAllButton->setIconSize(QSize(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio));
    mClearAllButton->setIcon(QIcon((mClearAllPixmap)));
    mClearAllButton->setHidden(true);

    mSelectAllButton = new QPushButton(this);
    mSelectAllButton->setStyleSheet("border: none;");
    connect(mSelectAllButton, SIGNAL(clicked(bool)), this, SLOT(selectAllButtonClicked(bool)));
    mSelectAllButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mSelectAllPixmap = QPixmap(":/images/selectAllIcon.png");
    mSelectAllButton->setIconSize(QSize(mMinimumHeight * mIconRatio, mMinimumHeight * mIconRatio));
    mSelectAllButton->setIcon(QIcon((mSelectAllPixmap)));
    mSelectAllButton->setHidden(true);

    mTopLayout = new QHBoxLayout();
    mTopLayout->addWidget(mName);
    mTopLayout->addWidget(mClearAllButton);
    mTopLayout->addWidget(mSelectAllButton);
    mTopLayout->addWidget(mEditButton);
    mTopLayout->addWidget(mHiddenStateIcon);

    mTopLayout->setStretch(0, 10);
    mTopLayout->setStretch(1, 2);
    mTopLayout->setStretch(2, 2);
    mTopLayout->setStretch(3, 2);
    mTopLayout->setStretch(4, 2);

    updateDevices(devices);

    mLayout->addLayout(mTopLayout);
    mLayout->addWidget(mWidget);
}

void ListDevicesGroupWidget::updateDevices(std::list<SLightDevice> devices, bool removeIfNotFound) {
    mDevices = devices;

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
            SLightDevice existingDevice = existingWidget->device();
            if (compareLightDevice(inputDevice, existingDevice)) {
                foundDevice = true;
                EColorGroup group = inputDevice.colorGroup;
                existingWidget->updateWidget(inputDevice, mData->colorGroup(group));
            }
            ++x;
        }

        //----------------
        // Create Widget, if not found
        //----------------
        if (!foundDevice) {
            // TODO: remove edge case...
            if (inputDevice.color.isValid()) {
                QString name;
                if (inputDevice.type == ECommType::eHue) {
                    SHueLight hue = mComm->hueLightFromLightDevice(inputDevice);
                    name = hue.name;
                } else {
                    name = inputDevice.name;
                }
                if (name.size() > 0) {
                    ListDeviceWidget *widget = new ListDeviceWidget(inputDevice,
                                                                    name,
                                                                    mData->colorGroup(inputDevice.colorGroup),
                                                                    false,
                                                                    mWidgetSize,
                                                                    this);
                    connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                    //widget->setFixedSize(mWidgetSize);
                    insertWidgetIntoGrid(widget);
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
                if (compareLightDevice(device, existingWidget->device())) {
                    found = true;
                }
            }
            if (!found) {
                removeWidgetFromGrid(existingWidget);
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

void ListDevicesGroupWidget::setCheckedDevices(std::list<SLightDevice> devices) {
    int numOfDevices = 0;
    for (auto&& existingWidget : mWidgets) {
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
        Q_ASSERT(widget);

        SLightDevice widgetDevice = widget->device();
        bool found = false;
        for (auto&& device : devices) {
            if (compareLightDevice(device, widgetDevice)) {
                numOfDevices++;
                found = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!found) {
            widget->setHighlightChecked(false);
        }
    }
    mCheckedDevices = numOfDevices;
    repaint();
}

void ListDevicesGroupWidget::updateRightHandButtons() {
    resizeRightHandIcon(mSelectAllPixmap, mSelectAllButton);
    resizeRightHandIcon(mClearAllPixmap, mClearAllButton);
}

void ListDevicesGroupWidget::clearButtonClicked(bool) {
    emit clearAllClicked(mKey);
}

void ListDevicesGroupWidget::selectAllButtonClicked(bool) {
    if (!mShowButtons) {
        setShowButtons(true);
    }
    emit selectAllClicked(mKey);
}

void ListDevicesGroupWidget::enterEvent(QEvent *) {
    if (!mHideEdit) {
        mEditButton->setHidden(false);
    }
    mSelectAllButton->setHidden(false);
    mClearAllButton->setHidden(false);
}

void ListDevicesGroupWidget::leaveEvent(QEvent *) {
    if (!mHideEdit) {
        mEditButton->setHidden(true);
    }
    mSelectAllButton->setHidden(true);
    mClearAllButton->setHidden(true);
}

void ListDevicesGroupWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect nameRect = mName->rect();
    QRect topRect(nameRect.x(), nameRect.y(), this->width(), nameRect.height());
    if (mCheckedDevices) {
        painter.fillRect(topRect, QBrush(computeHighlightColor()));
    } else {
        painter.fillRect(topRect, QBrush(QColor(32, 31, 31)));
    }

    QBrush blueBrush = QBrush(QColor(61, 142, 201));
    QBrush blackBrush = QBrush(QColor(32, 31, 31));

    // now loop through and draw the higlights on all the boxes
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        QPoint position = widgetPosition(mWidgets[i]);
        QRect widgetRect = QRect(position.x() * mWidgetSize.width(),
                                 topRect.height() + position.y() * mWidgetSize.height(),
                                 mWidgetSize.width(),
                                 mWidgetSize.height());
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(mWidgets[i]);
        Q_ASSERT(widget);
        if (widget->checked()) {
            painter.fillRect(widgetRect, blueBrush);
        } else {
            painter.fillRect(widgetRect, blackBrush);
        }
    }
}

QColor ListDevicesGroupWidget::computeHighlightColor() {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(32, 31, 31);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    float amountOfBlue = (float)mCheckedDevices / (float)mWidgets.size();
    return QColor(amountOfBlue * difference.red() + pureBlack.red(),
                  amountOfBlue * difference.green() + pureBlack.green(),
                  amountOfBlue * difference.blue() + pureBlack.blue());
}

void ListDevicesGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mShowButtons);
}

