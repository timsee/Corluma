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
    explicit LeafEffectPage(QWidget* parent)
        : QWidget(parent),
          mBackButton(new QPushButton("<", this)),
          mHeader(new QLabel(this)),
          mMetadata{new QLabel(this)},
          mIsSoundControlled{false},
          mStandardButton{new QPushButton("Standard", this)},
          mSoundButton{new QPushButton("Sound", this)},
          mStandardEffects{new nano::LeafEffectScrollArea(this)},
          mSoundEffects{new nano::LeafEffectScrollArea(this)} {
        mHeader->setWordWrap(true);
        connect(mBackButton, SIGNAL(clicked()), this, SLOT(clickedClose()));

        mStandardButton->setCheckable(true);
        mSoundButton->setCheckable(true);

        connect(mStandardButton, SIGNAL(clicked(bool)), this, SLOT(standardButtonPressed(bool)));
        connect(mSoundButton, SIGNAL(clicked(bool)), this, SLOT(soundButtonPressed(bool)));

        mMetadata->setVisible(false);
        connect(mStandardEffects,
                SIGNAL(selectEffect(QString)),
                this,
                SLOT(effectSelected(QString)));

        connect(mSoundEffects, SIGNAL(selectEffect(QString)), this, SLOT(effectSelected(QString)));

        mSoundEffects->setVisible(false);
        highlightButtons();
    }

    /// set the current nanoleaf being used
    void setNanoleaf(const nano::LeafMetadata& metadata) { mLeaf = metadata; }

    /// returns the current effect being displayed at the top.
    const QString& currentEffect() { return mCurrentEffect.name(); }

    /// update the effects displayed
    void updateEffects(const nano::LeafEffect& currentEffect,
                       const std::vector<nano::LeafEffect>& effects) {
        mCurrentEffect = currentEffect;

        std::vector<nano::LeafEffect> standardEffects;
        std::vector<nano::LeafEffect> soundEffects;
        for (const auto& effect : effects) {
            if (effect.pluginType() == "rhythm") {
                soundEffects.push_back(effect);
            } else {
                standardEffects.push_back(effect);
            }
        }
        mSoundEffects->showEffects(currentEffect.name(), soundEffects);
        mStandardEffects->showEffects(currentEffect.name(), standardEffects);
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

        auto rowHeight = height() / 10;
        auto backButtonWidth = width() / 6;
        auto columnWidth = width() / 2;

        int yPos = 0;
        int xPos = 0;

        mBackButton->setGeometry(xPos, yPos, backButtonWidth, rowHeight);
        xPos += mBackButton->width();

        mHeader->setGeometry(xPos, yPos, width() - backButtonWidth, rowHeight);
        yPos += mHeader->height();

        mStandardButton->setGeometry(0, yPos, columnWidth, rowHeight);
        mSoundButton->setGeometry(columnWidth, yPos, columnWidth, rowHeight);
        yPos += mStandardButton->height();

        //        mMetadata->setGeometry(0, yPos, width(), rowHeight);
        //        yPos += mMetadata->height();

        auto ySpacer = height() / 30;
        yPos += ySpacer;

        mStandardEffects->setGeometry(0, yPos, width(), rowHeight * 9 - ySpacer);
        mStandardEffects->resize(QRect(0, yPos, width(), rowHeight * 9 - ySpacer), rowHeight);

        mSoundEffects->setGeometry(0, yPos, width(), rowHeight * 9 - ySpacer);
        mSoundEffects->resize(QRect(0, yPos, width(), rowHeight * 9 - ySpacer), rowHeight);
    }

signals:
    /// emits when close is pressed
    void closePressed();

    /// signals when an effect is selected, signals the light's serial number and the effect's name.
    void selectEffect(QString, QString);

private slots:

    /// handles when close is clicked
    void clickedClose() { emit closePressed(); }

    /// handles when an effect is selected, tells the nanoleaf to use this effect.
    void effectSelected(QString name) { emit selectEffect(mLeaf.serialNumber(), name); }

    /// handles when a standard button is pressed.
    void standardButtonPressed(bool) {
        mIsSoundControlled = false;
        highlightButtons();

        mSoundEffects->setVisible(false);
        mStandardEffects->setVisible(true);
        resize();
    }

    /// handles when a sound button is pressed.
    void soundButtonPressed(bool) {
        mIsSoundControlled = true;
        highlightButtons();

        mSoundEffects->setVisible(true);
        mStandardEffects->setVisible(false);
        resize();
    }

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
    /// handles highlighting buttons.
    void highlightButtons() {
        if (mIsSoundControlled) {
            mSoundButton->setChecked(true);
            mStandardButton->setChecked(false);
        } else {
            mSoundButton->setChecked(false);
            mStandardButton->setChecked(true);
        }
    }


    /// the current nanoleaf
    nano::LeafMetadata mLeaf;

    /// back button
    QPushButton* mBackButton;

    /// header for top of widget
    QLabel* mHeader;

    /// metadata about the effects displayed
    QLabel* mMetadata;

    /// true if sound controlled, false otherwsie
    bool mIsSoundControlled;

    /// button to show standard routiens
    QPushButton* mStandardButton;

    /// button to show sound based routines
    QPushButton* mSoundButton;

    /// current effect shown
    nano::LeafEffect mCurrentEffect;

    /// scroll area to display all the standard effects.
    nano::LeafEffectScrollArea* mStandardEffects;

    /// scroll area for effects based on sound.
    nano::LeafEffectScrollArea* mSoundEffects;
};

} // namespace nano


#endif // LEAFEFFECTPAGE_H
