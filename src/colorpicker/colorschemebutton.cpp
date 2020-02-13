/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "colorschemebutton.h"

#include <QDebug>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"
#include "utils/exception.h"


QString colorSchemeTypeToString(EColorSchemeType type) {
    switch (type) {
        case EColorSchemeType::similar:
            return "Similar";
        case EColorSchemeType::complement:
            return "Complement";
        case EColorSchemeType::triad:
            return "Triad";
        case EColorSchemeType::compound:
            return "Compound";
        case EColorSchemeType::custom:
            return "Custom";
        case EColorSchemeType::MAX:
            return "MAX";
    }
    THROW_EXCEPTION("Did not recognize EColorSchemeType");
}

EColorSchemeType stringToColorSchemeType(const QString& string) {
    if (string == "Similar") {
        return EColorSchemeType::similar;
    } else if (string == "Complement") {
        return EColorSchemeType::complement;
    } else if (string == "Triad") {
        return EColorSchemeType::triad;
    } else if (string == "Compound") {
        return EColorSchemeType::compound;
    } else if (string == "Custom") {
        return EColorSchemeType::custom;
    }
    THROW_EXCEPTION("String not recognized as a EColorSchemeType");
}

ColorSchemeButton::ColorSchemeButton(EColorSchemeType type,
                                     const QString& imagePath,
                                     QWidget* parent)
    : QWidget(parent),
      mResourcePath{imagePath} {
    mEnabled = true;
    const QString transparentStyleSheet = "font-size:12pt; background-color: rgba(0,0,0,0);";

    mTitle = new QLabel(colorSchemeTypeToString(type), this);
    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mTitle->setStyleSheet(transparentStyleSheet);
    mTitle->setAlignment(Qt::AlignCenter);

    mState = EColorSchemeButtonState::deselected;

    mButton = new QLabel(this);
    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mButton->setStyleSheet(transparentStyleSheet);
    mButton->setAlignment(Qt::AlignCenter);

    setMinimumHeight(mTitle->height());
}


void ColorSchemeButton::resize() {
    auto parentSize = parentWidget()->size();
    QSize pixmapSize = QSize(parentSize.height() * 0.5, parentSize.height() * 0.5);
    mPixmap = QPixmap(mResourcePath);
    mPixmap = mPixmap.scaled(pixmapSize.width(),
                             pixmapSize.height(),
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);

    mButton->setGeometry(0, 0, parentSize.width() / 5, parentSize.height() * 2 / 3);
    mTitle->setGeometry(0,
                        parentSize.height() * 2 / 3,
                        parentSize.width() / 5,
                        parentSize.height() / 3);
    mButton->setPixmap(mPixmap);
}

void ColorSchemeButton::select(bool shouldSelect) {
    if (shouldSelect) {
        mState = EColorSchemeButtonState::selected;
    } else {
        mState = EColorSchemeButtonState::deselected;
    }
    update();
}

void ColorSchemeButton::buttonPressed(bool) {
    if (mState == EColorSchemeButtonState::deselected) {
        mState = EColorSchemeButtonState::selected;
    } else {
        mState = EColorSchemeButtonState::deselected;
    }
    emit clicked(key(), mState == EColorSchemeButtonState::selected);
    update();
}

void ColorSchemeButton::resizeEvent(QResizeEvent*) {
    resize();
    mButton->setPixmap(mPixmap);
}

void ColorSchemeButton::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);

    QPainterPath path;
    path.addRect(rect());

    if (!mEnabled) {
        painter.fillPath(path, QBrush(QColor(23, 22, 22, 255)));
    } else if (mState == EColorSchemeButtonState::deselected) {
        painter.fillPath(path, QBrush(QColor(43, 42, 42, 255)));
    } else {
        painter.fillPath(path, QBrush(QColor(126, 125, 125, 255)));
        painter.drawPath(path);
    }
}


void ColorSchemeButton::enable(bool enable) {
    mEnabled = enable;
    setEnabled(enable);
    update();
}

void ColorSchemeButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        buttonPressed(true);
    }
    event->ignore();
}
