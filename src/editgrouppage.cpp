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

struct sortListDeviceWidget {
  bool operator() (ListDeviceWidget *i, ListDeviceWidget *j) {
      if (i->key().compare(j->key()) < 0) {
          return true;
      } else {
          return false;
      }
  }
} listDeviceSort;

EditGroupPage::EditGroupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditCollectionPage) {
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(ui->resetButton, SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(ui->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePressed(bool)));

    connect(ui->nameEdit, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

    QScroller::grabGesture(ui->deviceList->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    ui->deviceList->setWidget(mScrollAreaWidget);

    mLayout = new QVBoxLayout(mScrollAreaWidget);
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mLayout);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    ui->deviceList->setStyleSheet("background-color:rgba(33,32,32,255);");
}

EditGroupPage::~EditGroupPage() {
    delete ui;
}

void EditGroupPage::showGroup(QString key, std::list<SLightDevice> groupDevices, std::list<SLightDevice> devices, bool isMood) {
    mOriginalName = key;
    mNewName = key;
    mOriginalDevices = groupDevices;
    ui->nameEdit->setText(key);
    ui->saveButton->setEnabled(false);
    mIsMood = isMood;
    if (mIsMood) {
        ui->helpLabel->setText("Edit the Mood...");
    } else {
        ui->helpLabel->setText("Edit the Collection...");
    }
    updateDevices(groupDevices, devices);
    repaint();
}

void EditGroupPage::updateDevices(std::list<SLightDevice> groupDevices, std::list<SLightDevice> devices) {

    for (auto&& device : devices) {
        bool widgetFound = false;
        for (auto&& widget : mWidgets) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                widgetFound = true;
                widget->updateWidget(device, mData->colorGroup(device.colorGroup));
                widget->setHighlightChecked(shouldSetChecked(device, groupDevices));
            }
        }

        // no widget found for this device,
        if (!widgetFound) {
            ListDeviceWidget *widget = new ListDeviceWidget(device,
                                                            mData->colorGroup(device.colorGroup),
                                                            true,
                                                            QSize(this->width() * 0.9f, this->height() / 8),
                                                            mScrollAreaWidget);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(listDeviceWidgetClicked(QString)));
            widget->setHighlightChecked(shouldSetChecked(device, groupDevices));
            mWidgets.push_back(widget);
        }
    }

    // sort widgets
    std::sort(mWidgets.begin(), mWidgets.end(), listDeviceSort);

    // add sorted widgets into layout
    for (auto widget : mWidgets) {
        mLayout->addWidget(widget);
    }

    resize(false);
}

void EditGroupPage::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(size.width() * 0.125f,
                          size.height() * 0.125f,
                          size.width() * 0.75f,
                          size.height() * 0.75f);
    }
    QSize widgetSize(this->width(), this->height() / 8);
    uint32_t yPos = 0;
    // draw widgets in content region
    for (auto widget : mWidgets) {
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widgetSize.height());
        yPos += widgetSize.height();
    }
}




// ----------------------------
// Slots
// ----------------------------

void EditGroupPage::listDeviceWidgetClicked(QString key) {
    ListDeviceWidget *widget;
    bool widgetFound = false;

    for (auto w : mWidgets) {
        if (w->key().compare(key) == 0) {
            widget = w;
            widgetFound = true;
        }
    }
    if (widgetFound) {
         widget->setHighlightChecked(!widget->checked());

         if (checkForChanges()) {
             ui->saveButton->setEnabled(true);
         } else {
             ui->saveButton->setEnabled(false);
         }
    }
}

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
          ui->saveButton->setEnabled(true);
        }
    }
    emit pressedClose();
}

void EditGroupPage::resetPressed(bool) {
    for (auto&& widget : mWidgets) {
        bool deviceFound = false;
        for (auto&& device : mOriginalDevices) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                deviceFound = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!deviceFound) widget->setHighlightChecked(false);
    }
}

void EditGroupPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
          // make new original devices
          mOriginalDevices = createCollection();
          // make new original name
          mOriginalName = mNewName;
          ui->saveButton->setEnabled(false);
        }
    }
}

// ----------------------------
// Protected
// ----------------------------


void EditGroupPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    mRenderThread->start(mRenderInterval);
}


void EditGroupPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);

    mRenderThread->stop();
}

void EditGroupPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void EditGroupPage::lineEditChanged(const QString& newText) {
    mNewName = newText;

    if (checkForChanges()) {
        ui->saveButton->setEnabled(true);
    } else {
        ui->saveButton->setEnabled(false);
    }
}

void EditGroupPage::renderUI() {

}

// ----------------------------
// Private
// ----------------------------

void EditGroupPage::saveChanges() {

    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    bool nameIsValid = false;
    if (mNewName.size() > 0
            && !(mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0)) {
            nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::list<SLightDevice> newDevices;
    if (mIsMood) {
        newDevices = createMood();
    } else {
        newDevices = createCollection();
    }

    bool devicesAreValid = true;
    if (newDevices.size() > 0) {
        for (auto& device : newDevices) {
            if (device.controller.compare("") == 0
                    || device.index == 0) {
                devicesAreValid = false;
            }
        }
    } else {
        devicesAreValid = false;
    }

    if (!nameIsValid || !devicesAreValid) {
        qDebug() << "Not saving this group: " << mNewName;
        qDebug() << "---------------------";
        for (auto& device : newDevices) {
            device.PRINT_DEBUG();
        }
        qDebug() << "---------------------";

        // pop up warning that it isn't saving
        QMessageBox msgBox;
        msgBox.setText("Trying to save invalid group, no changes will be made.");
        msgBox.exec();
        // close edit page anyway.
        emit pressedClose();
        return;
    }

    //---------------------------------
    // Save if passing checks
    //---------------------------------
    if (mIsMood) {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewMood(mNewName, newDevices);
    } else {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewCollection(mNewName, newDevices);
    }
}

std::list<SLightDevice> EditGroupPage::createCollection() {
   std::list<SLightDevice> devices;
   for (uint32_t i = 0; i < mWidgets.size(); ++i) {
       if (mWidgets[i]->checked()) {
           devices.push_back(mWidgets[i]->device());
       }
   }
   return devices;
}

std::list<SLightDevice> EditGroupPage::createMood() {
    std::list<SLightDevice> list;
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        if (mWidgets[i]->checked()) {
            list.push_back(mWidgets[i]->device());
        }
    }
    return list;
}

bool EditGroupPage::checkForChanges() {
    if (!(mNewName.compare(mOriginalName) == 0)) {
        return true;
    }

    // check all checked devices are part of original group
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        if (mWidgets[i]->checked()) {
            bool foundDevice = false;
            for (auto&& device : mOriginalDevices) {
                if (compareLightDevice(mWidgets[i]->device(), device)) {
                    foundDevice = true;
                }
            }
            if (!foundDevice) {
                qDebug() << "all devices are part of original group";
                return true;
            }
        }
    }
    // check all given devices are checked
    for (auto&& device : mOriginalDevices) {
        for (uint32_t i = 0; i < mWidgets.size(); ++i) {
            if (compareLightDevice(mWidgets[i]->device(), device)) {
                if (!mWidgets[i]->checked())  {
                    qDebug() << "all given deviecs are checked";
                    return true;
                }
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
