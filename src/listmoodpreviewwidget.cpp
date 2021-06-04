/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "listmoodpreviewwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "utils/qt.h"

ListMoodPreviewWidget::ListMoodPreviewWidget(const cor::Mood& mood, QWidget* parent)
    : cor::ListItemWidget(mood.name(), parent),
      mPalette{new cor::PaletteWidget(this)},
      mMood{mood} {
    mIsChecked = false;
    mIsSelected = false;

    mKey = QString::number(mMood.uniqueID());

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
    mName->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mName->setAlignment(Qt::AlignBottom);
    mName->setStyleSheet(backgroundStyleSheet);

    mPalette->skipOffLightStates(true);
    mPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mPalette->show(cor::lightStatesFromLights(mMood.lights(), false));

    // setup layout
    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mName, 6);
    mTopLayout->setContentsMargins(0, 0, 0, 0);
    mFullLayout = new QVBoxLayout(this);
    mFullLayout->addLayout(mTopLayout, 1);
    mFullLayout->addWidget(mPalette, 2);

    setLayout(mFullLayout);

    setStyleSheet(backgroundStyleSheet);
}

void ListMoodPreviewWidget::resize() {
    mName->setFixedSize(width(), height() / 4);
    mPalette->setFixedSize(int(width() * 0.75f), 2 * height() / 4);
}

void ListMoodPreviewWidget::resizeEvent(QResizeEvent*) {
    resize();
}

bool ListMoodPreviewWidget::setChecked(bool checked) {
    mIsChecked = checked;
    update();
    return mIsChecked;
}

bool ListMoodPreviewWidget::setSelected(bool selected) {
    mIsSelected = selected;
    update();
    return mIsSelected;
}

void ListMoodPreviewWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(cor::kHighlightColor));
    } else {
        painter.fillRect(rect(), QBrush(cor::kBackgroundColor));
    }

    if (mIsSelected) {
        painter.fillRect(rect(), QBrush(cor::kHighlightColor));
    }
}


void ListMoodPreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        emit moodSelected(mMood.uniqueID());
        event->accept();
    } else {
        event->ignore();
    }
}
