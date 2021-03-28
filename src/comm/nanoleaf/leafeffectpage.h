#ifndef LEAFEFFECTPAGE_H
#define LEAFEFFECTPAGE_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"
#include "comm/nanoleaf/leafeffectscrollarea.h"
#include "comm/nanoleaf/leafeffectwidget.h"
#include "comm/nanoleaf/leafmetadata.h"
#include "cor/objects/page.h"
#include "routines/speedslider.h"
#include "utils/qt.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LeafEffectPage class displays a LeafEffectContainer in a scroll area. This is used to
 * display the effects in a nanoleaf.
 */
class LeafEffectPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit LeafEffectPage(CommLayer* comm, QWidget* parent)
        : QWidget(parent),
          mComm{comm},
          mBackButton(new QPushButton("<", this)),
          mHeader(new QLabel(this)),
          mMetadata{new QLabel(this)},
          mCurrentPalette(new LeafEffectWidget({}, false, this)),
          mStoredEffects{new nano::LeafEffectScrollArea(this)} {
        mHeader->setWordWrap(true);
        mCurrentPalette->displayCheckbox(false);
        connect(mBackButton, SIGNAL(clicked()), this, SLOT(clickedClose()));

        connect(mStoredEffects, SIGNAL(selectEffect(QString)), this, SLOT(effectSelected(QString)));
    }

    /// set the current nanoleaf being used
    void setNanoleaf(const nano::LeafMetadata& metadata) { mLeaf = metadata; }

    /// returns the current effect being displayed at the top.
    const QString& currentEffect() { return mCurrentPalette->effect().name(); }

    /// update the effects displayed
    void updateEffects(const nano::LeafEffect& currentEffect,
                       const std::vector<nano::LeafEffect>& effects) {
        mCurrentPalette->update(currentEffect, true);
        mStoredEffects->showEffects(currentEffect.name(), effects);
    }

    /// pushes widget in
    void pushIn(const QString& inputPrompt) {
        mHeader->setText(inputPrompt);

        moveWidget(
            this,
            QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
            QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));

        setVisible(true);
        isOpen(true);
        raise();
    }

    /// pushes widget out
    void pushOut() {
        moveWidget(
            this,
            QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
            QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));

        setVisible(false);
        isOpen(false);
    }

    /// resizes widget programmatically
    void resize() {
        QSize size = parentWidget()->size();
        setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.75f));

        auto rowHeight = height() / 8;
        auto backButtonWidth = width() / 6;
        auto columnWidth = width() / 2;

        int yPos = 0;
        int xPos = 0;

        mBackButton->setGeometry(xPos, yPos, backButtonWidth, rowHeight);
        xPos += mBackButton->width();

        mHeader->setGeometry(xPos, yPos, width() - backButtonWidth, rowHeight);
        yPos += mHeader->height();

        mMetadata->setGeometry(0, yPos, columnWidth, rowHeight * 2);
        mCurrentPalette->setGeometry(columnWidth, yPos, columnWidth, rowHeight * 2);
        yPos += mCurrentPalette->height();

        auto ySpacer = height() / 30;
        yPos += ySpacer;

        mStoredEffects->setGeometry(0, yPos, width(), rowHeight * 5 - ySpacer);
        mStoredEffects->resize(QRect(0, yPos, width(), rowHeight * 5 - ySpacer), rowHeight);
    }

signals:
    /// emits when close is pressed
    void closePressed();

    void selectEffect(QString, QString);

private slots:

    /// handles when close is clicked
    void clickedClose() { emit closePressed(); }

    /// handles when an effect is selected, tells the nanoleaf to use this effect.
    void effectSelected(QString name) { emit selectEffect(mLeaf.serialNumber(), name); }

protected:
    /// detects when widget is resized
    virtual void resizeEvent(QResizeEvent*) { resize(); }

    /// called whenever the widget is repainted
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
    }

private:
    /// the current nanoleaf
    nano::LeafMetadata mLeaf;

    /// pointer to comm layer.
    CommLayer* mComm;

    /// back button
    QPushButton* mBackButton;

    /// header for top of widget
    QLabel* mHeader;

    /// metadata about the effects displayed
    QLabel* mMetadata;

    /// widget for the current palette
    nano::LeafEffectWidget* mCurrentPalette;

    /// scroll area to display all the effects.
    nano::LeafEffectScrollArea* mStoredEffects;
};

} // namespace nano


#endif // LEAFEFFECTPAGE_H
