/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>
#include "listmoodpreviewwidget.h"
#include "utils/qt.h"

ListMoodPreviewWidget::ListMoodPreviewWidget(const cor::Mood& mood,
                                             QWidget *parent) : cor::ListItemWidget(mood.name(), parent), mMood{mood} {
    mIsChecked = false;
    mIsSelected = false;

    mKey = mMood.name();

    // setup icon
    QString backgroundStyleSheet = "background:rgba(0, 0, 0, 0%);";

    // setup main label
    mName = new QLabel(this);
    QString modifiedName;
    if (mMood.name().size() > 15) {
        modifiedName = mMood.name().mid(0, 12) + "...";
    } else {
        modifiedName = mMood.name();
    }
    mName->setText("<b>" + modifiedName + "</b>");
    mName->setEnabled(false);
    mName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mName->setStyleSheet(backgroundStyleSheet);

    mPalette = new  cor::LightVectorWidget(4, 2, this);
    mPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mPalette->enableButtonInteraction(false);
    mPalette->updateDevices(mMood.lights);

    // setup layout
    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mName, 6);
    mTopLayout->setContentsMargins(0,0,0,0);
    mFullLayout = new QVBoxLayout(this);
    mFullLayout->addLayout(mTopLayout, 1);
    mFullLayout->addWidget(mPalette, 2);

    setLayout(mFullLayout);

    this->setStyleSheet(backgroundStyleSheet);
}


void ListMoodPreviewWidget::resizeEvent(QResizeEvent *) {
    mName->setFixedSize(this->width(), this->height() / 4);
    mPalette->setFixedSize(int(this->width() * 0.75f), 2 * this->height() / 4);
}


void ListMoodPreviewWidget::enterEvent(QEvent *) {
   // mEditButton->setHidden(false);
}

void ListMoodPreviewWidget::leaveEvent(QEvent *) {
   // mEditButton->setHidden(true);
}

bool ListMoodPreviewWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
    return mIsChecked;
}

bool ListMoodPreviewWidget::setSelected(bool selected) {
    mIsSelected = selected;
    repaint();
    return mIsSelected;
}

void ListMoodPreviewWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(this->rect());

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31)));
    }

    if (mIsSelected) {
        painter.drawPath(path);
    }
}


void ListMoodPreviewWidget::mouseReleaseEvent(QMouseEvent *) {
    emit moodSelected(mMood.uniqueID());
}

