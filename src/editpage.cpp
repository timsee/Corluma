/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "editpage.h"

#include <QMessageBox>
#include <QScroller>

#include "utils/qt.h"

EditPage::EditPage(QWidget* parent, CommLayer* comm, GroupData* parser)
    : QWidget(parent),
      mComm(comm),
      mGroups(parser) {
    mTopMenu = new EditPageTopMenu(this);
    mTopMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mTopMenu->closeButton(), SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(mTopMenu->resetButton(), SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(mTopMenu->deleteButton(), SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
    connect(mTopMenu->saveButton(), SIGNAL(clicked(bool)), this, SLOT(savePressed(bool)));
    connect(mTopMenu->nameEdit(),
            SIGNAL(textChanged(QString)),
            this,
            SLOT(lineEditChanged(QString)));

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
}

void EditPage::updateDevices(const std::vector<cor::Light>& checkedDevices,
                             const std::vector<cor::Light>& devices) {
    mSimpleGroupWidget->updateDevices(devices,
                                      cor::EWidgetType::full,
                                      EOnOffSwitchState::hidden,
                                      true,
                                      false);
    mSimpleGroupWidget->setCheckedDevices(checkedDevices);
}

// ----------------------------
// Slots
// ----------------------------

void EditPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    auto messages = deleteMessages();
    QString name = messages.first;
    QString text = messages.second;
    reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mGroups->removeGroup(name);
        // delete from hue bridge, if applicable.
        mComm->deleteHueGroup(name);
        emit pressedClose();
    }
}

void EditPage::closePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            auto saveSuccessful = saveChanges();
            if (!saveSuccessful) {
                QMessageBox msgBox;
                msgBox.setText("Saving unsuccessful, no changes will be made.");
                msgBox.exec();
            }
        }
    }
    emit pressedClose();
}

void EditPage::resetPressed(bool) {
    reset();
}

void EditPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            auto saveSuccessful = saveChanges();
            if (!saveSuccessful) {
                QMessageBox msgBox;
                msgBox.setText("Saving unsuccessful, no changes will be made.");
                msgBox.exec();
            }
            // close
            emit pressedClose();
        }
    }
}


// ----------------------------
// Protected
// ----------------------------


void EditPage::resize() {
    auto yPos = 0;
    mTopMenu->setGeometry(0, yPos, width(), height() / 5);
    yPos += mTopMenu->height();

    mSimpleGroupWidget->setGeometry(0, yPos, width(), height() * 4 / 5);

    mSimpleGroupWidget->resizeWidgets();
}

void EditPage::resizeEvent(QResizeEvent*) {
    resize();
}

void EditPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void EditPage::lineEditChanged(const QString& newText) {
    changeName(newText);

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void EditPage::renderUI() {}

// ----------------------------
// Private
// ----------------------------

bool EditPage::shouldSetChecked(const cor::Light& device,
                                const std::vector<cor::Light>& groupDevices) {
    for (const auto& collectionDevice : groupDevices) {
        if (device.uniqueID() == collectionDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}

void EditPage::clickedDevice(const QString&) {
    // call the highlight button
    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void EditPage::pushIn() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));
    raise();
    setVisible(true);
    mRenderThread->start(mRenderInterval);
}

void EditPage::pushOut() {
    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
    mRenderThread->stop();
}
