/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "editgrouppage.h"
#include "ui_editcollectionpage.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

EditGroupPage::EditGroupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditCollectionPage) {
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(ui->resetButton, SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(ui->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));

    connect(ui->nameEdit, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

    connect(ui->deviceList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(deviceListClicked(QListWidgetItem*)));

    QScroller::grabGesture(ui->deviceList->viewport(), QScroller::LeftMouseButtonGesture);

}

EditGroupPage::~EditGroupPage() {
    delete ui;
}

void EditGroupPage::showGroup(QString key, std::list<SLightDevice> groupDevices, std::list<SLightDevice> devices, bool isMood) {
    mOriginalName = key;
    mNewName = key;
    mOriginalDevices = groupDevices;
    ui->nameEdit->setText(key);
    mIsMood = isMood;
    if (mIsMood) {
        ui->helpLabel->setText("Edit the Mood...");
    } else {
        ui->helpLabel->setText("Edit the Collection...");
    }

    for (auto&& device : devices) {
        bool widgetFound = false;
        for (auto&& widget : mWidgets) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                widgetFound = true;
                widget->updateWidget(device, mData->colorGroup(device.colorGroup));
                widget->setChecked(shouldSetChecked(device, groupDevices));
            }
        }

        // no widget found for this device,
        if (!widgetFound) {
            QString name;
            if (device.type == ECommType::eHue) {
                SHueLight hue = mComm->hueLightFromLightDevice(device);
                name = hue.name;
            } else {
                name = device.name;
            }

            ListDeviceWidget *widget = new ListDeviceWidget(device, name, mData->colorGroup(device.colorGroup));
            widget->setChecked(shouldSetChecked(device, groupDevices));

            int index = ui->deviceList->count();
            ui->deviceList->addItem(widget->key());
            ui->deviceList->setItemWidget(ui->deviceList->item(index), widget);
            mWidgets.push_back(widget);
        }
    }
    ui->deviceList->sortItems();
    repaint();
}

void EditGroupPage::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(size.width() * 0.125f, size.height() * 0.125f, size.width() * 0.75f, size.height() * 0.75f);
    QSize widgetSize(this->width(), this->height() / 8);
    for (int i = 0; i < ui->deviceList->count(); ++i) {
        QListWidgetItem *item = ui->deviceList->item(i);
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
        Q_ASSERT(widget);
        widget->setMinimumSize(widgetSize);
        widget->setMaximumSize(widgetSize);
        item->setSizeHint(widgetSize);
    }
}




// ----------------------------
// Slots
// ----------------------------

void EditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    QString text = "Delete the " + mOriginalName + " group?";
    reply = QMessageBox::question(this, "Delete?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      mGroups->removeGroup(mOriginalName);
      emit pressedClose();
    }
}

void EditGroupPage::closePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
        }
    }
    emit pressedClose();
}

void EditGroupPage::deviceListClicked(QListWidgetItem* item) {
     ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
     widget->setChecked(widget->checked());
}

void EditGroupPage::resetPressed(bool) {
    for (auto&& widget : mWidgets) {
        bool deviceFound = false;
        for (auto&& device : mOriginalDevices) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                deviceFound = true;
                widget->setChecked(true);
            }
        }
        if (!deviceFound) widget->setChecked(false);
    }
}

// ----------------------------
// Protected
// ----------------------------


void EditGroupPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    //mRenderThread->start(mRenderInterval);
}


void EditGroupPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);

    //mRenderThread->stop();
}

void EditGroupPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void EditGroupPage::lineEditChanged(const QString& newText) {
    qDebug() << "line edit changted" << newText;
    mNewName = newText;
}

// ----------------------------
// Private
// ----------------------------

void EditGroupPage::saveChanges() {
    if (mIsMood) {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewMood(mNewName, createCollection());
    } else {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewCollection(mNewName, createCollection());
    }
}

std::list<SLightDevice> EditGroupPage::createCollection() {
   std::list<SLightDevice> list;
   for (int i = 0; i < ui->deviceList->count(); ++i) {
       QListWidgetItem *item = ui->deviceList->item(i);
       ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
       Q_ASSERT(widget);
       if (widget->checked()) {
           list.push_back(widget->device());
       }
   }
   return list;
}

std::list<SLightDevice> EditGroupPage::createMood() {
    std::list<SLightDevice> list;
    return list;
}

bool EditGroupPage::checkForChanges() {
    if (!(mNewName.compare(mOriginalName) == 0)) {
        return true;
    }
    // check all checked devices are part of original group
    for (int i = 0; i < ui->deviceList->count(); ++i) {
        QListWidgetItem *item = ui->deviceList->item(i);
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
        Q_ASSERT(widget);
        if (widget->checked()) {
            bool foundDevice = false;
            for (auto&& device : mOriginalDevices) {
                if (compareLightDevice(widget->device(), device)) {
                    foundDevice = true;
                }
            }
            if (!foundDevice) return true;
        }
    }
    // check all given devices are checked
    for (auto&& device : mOriginalDevices) {
        for (int i = 0; i < ui->deviceList->count(); ++i) {
            QListWidgetItem *item = ui->deviceList->item(i);
            ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
            Q_ASSERT(widget);
            if (compareLightDevice(widget->device(), device)) {
                if (!widget->checked()) return true;
            }
        }
    }
    return false;
}

bool EditGroupPage::shouldSetChecked(const SLightDevice& device, const std::list<SLightDevice>& groupDevices) {
    for (auto&& collectionDevice : groupDevices) {
        if (compareLightDevice(device, collectionDevice)) {
            return true;
        }
    }
    return false;
}
