#ifndef LEAFEFFECTSCROLLAREA_H
#define LEAFEFFECTSCROLLAREA_H

#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QWidget>
#include "comm/nanoleaf/leafeffectcontainer.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafEffectScrollArea class is a scroll area that contains a LeafEffectContainer.
 */
class LeafEffectScrollArea : public QWidget {
    Q_OBJECT
public:
    explicit LeafEffectScrollArea(QWidget* parent)
        : QWidget(parent),
          mScrollArea{new QScrollArea(this)},
          mEffectContainer{new LeafEffectContainer(mScrollArea)},
          mRowHeight{10} {
        mEffectContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        connect(mEffectContainer,
                SIGNAL(selectEffect(QString)),
                this,
                SLOT(effectSelected(QString)));

        mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
        mScrollArea->setWidget(mEffectContainer);
        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mScrollArea->horizontalScrollBar()->setEnabled(false);
        mScrollArea->horizontalScrollBar()->setVisible(false);
    }

    /// resizes programmatically
    void resize(const QRect& inputRect, int buttonHeight) {
        setGeometry(inputRect);
        int offsetY = 0u;
        mRowHeight = buttonHeight * 3;
        mEffectContainer->changeRowHeight(mRowHeight);

        QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
        int scrollAreaWidth = int(rect.width() * 1.2);
        mScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mEffectContainer->setFixedWidth(rect.width());
        auto heightCount = mEffectContainer->effects().size();
        if (heightCount == 0) {
            heightCount = 1;
        }
        mEffectContainer->setFixedHeight(heightCount * buttonHeight);
        mEffectContainer->resize();
    }

    /// show specific effects
    void showEffects(const QString& currentEffectName,
                     const std::vector<nano::LeafEffect>& effects) {
        mEffectContainer->showEffects(currentEffectName, effects, mRowHeight);
    }

    /// getter for all effects
    const std::vector<nano::LeafEffect>& effects() { return mEffectContainer->effects(); }

    /// clear all the widgets from the container
    void clear() { mEffectContainer->clear(); }

signals:

    /// emits when an effect is selected.
    void selectEffect(QString);

private slots:

    /// handles when an effect is selected.
    void effectSelected(QString name) { emit selectEffect(name); }

private:
    /// scroll area for showing the MenuLightContainer
    QScrollArea* mScrollArea;

    /// container that holds all the effects.
    LeafEffectContainer* mEffectContainer;

    /// height of a row in the scrolling list
    int mRowHeight;
};

} // namespace nano

#endif // LEAFEFFECTSCROLLAREA_H
