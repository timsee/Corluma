#ifndef LEAFEFFECTWIDGET_H
#define LEAFEFFECTWIDGET_H

#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/nanoleaf/leafeffect.h"
#include "cor/widgets/checkbox.h"
#include "cor/widgets/listitemwidget.h"
#include "cor/widgets/palettewidget.h"
#include "utils/qt.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafEffectWidget class is a simple widget that displays all the data related to a
 * Nanoleaf's Effect.
 */
class LeafEffectWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    explicit LeafEffectWidget(const nano::LeafEffect& effect, bool isSelected, QWidget* parent)
        : cor::ListItemWidget(effect.name(), parent),
          mName{new QLabel(this)},
          mMetadata{new QLabel(this)},
          mPalette{new cor::PaletteWidget(this)},
          mCheckBox{new cor::CheckBox(this)},
          mSaveLocallyButton{new QPushButton("Save Palette", this)},
          mDisplayCheckbox{true},
          mDisplaySaveLocally{true} {
        setStyleSheet(cor::kDarkerGreyBackground);
        mMetadata->setWordWrap(true);
        mMetadata->setAlignment(Qt::AlignTop);
        update(effect, isSelected);

        connect(mSaveLocallyButton, SIGNAL(clicked(bool)), this, SLOT(saveLocallyPressed(bool)));

        connect(mCheckBox,
                SIGNAL(clicked(ECheckboxState)),
                this,
                SLOT(checkBoxClicked(ECheckboxState)));
    }

    /// getter for effect displayed.
    const nano::LeafEffect& effect() { return mEffect; }

    /// update the effect in the widget
    void update(const nano::LeafEffect& effect, bool isSelected) {
        mEffect = effect;
        mName->setText("<b>" + mEffect.name() + "</b>");
        mMetadata->setText(generateMetadataText());
        mPalette->show(effect.colors());

        setSelected(isSelected);
    }

    /// overrides the state of the checkbox, determines if it should be displayed or not.
    void displayCheckbox(bool display) {
        mDisplayCheckbox = display;
        mCheckBox->setVisible(display);
    }

    /// overrides the state of the save locally button, determines if it should be displayed or not
    void displaySaveLocally(bool display) {
        mDisplaySaveLocally = display;
        mSaveLocallyButton->setVisible(false);
    }

    /// programmatically select or deselect the checkbox
    void setSelected(bool select) {
        if (select) {
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }
    }

signals:

    /// emits when a effect is selected.
    void selectEffect(QString);

    /// emits when a palette should be saved.
    void savePalette(cor::Palette);

protected:
    /// detects when widget is resized
    virtual void resizeEvent(QResizeEvent*) { resize(); }

    /// called whenever the widget is repainted
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QBrush(QColor(33, 32, 32)));

        // paint top line
        int greyValue = 185;
        int transparency = 127;
        QBrush brush(QColor(greyValue, greyValue, greyValue, transparency));
        QPen pen(brush, 1);
        painter.setPen(pen);
        painter.drawLine(0, 0, width(), 0);
    }

    /// show the button to save a palette.
    void showPaletteButton(bool shouldShow) { mSaveLocallyButton->setVisible(shouldShow); }

private slots:

    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state) {
        if (state != ECheckboxState::checked) {
            mCheckBox->checkboxState(ECheckboxState::checked);
            emit selectEffect(mEffect.name());
        }
    }

    /// handles when the save locally button is pressed.
    void saveLocallyPressed(bool) {
        // TODO: check why the UUID for effects overlap, seems like a bug.
        auto palette = cor::Palette(cor::UUID::makeNew(), mEffect.name(), mEffect.colors());
        QMessageBox::StandardButton reply;
        QString text = "Do you want to save the palette from " + mEffect.name()
                       + " to this app? This will allow you to use the palette on other lights.";

        reply = QMessageBox::question(this, "Save?", text, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            emit savePalette(palette);
        } else {
            qDebug() << "INFO: not saving this palette: " << palette;
        }
    }

private:
    /// resize programmatically
    void resize() {
        auto ySpacer = height() / 20;
        auto rowHeight = height() / 7;
        auto buttonSide = rowHeight;

        // auto columnWidth = width() / 2;
        auto yPos = ySpacer;
        mName->setGeometry(0, yPos, width() - buttonSide, rowHeight);
        mCheckBox->setGeometry(mName->width(), yPos, buttonSide, buttonSide);
        yPos += mName->height() + ySpacer;
        mPalette->setGeometry(0, yPos, width(), rowHeight);
        yPos += mPalette->height();

        auto xPos = 0;
        mMetadata->setGeometry(xPos, yPos, width(), rowHeight * 3);
        xPos += mMetadata->width();
        yPos += mMetadata->height();

        mSaveLocallyButton->setGeometry(0, yPos, width() / 3, rowHeight);
    }

    /// generate metadata about the effect
    QString generateMetadataText() {
        std::stringstream sstream;
        if (mEffect.pluginSimpleName() != "Custom") {
            sstream << "<b>Effect:</b> " << mEffect.pluginSimpleName().toStdString() << "<br>";
        }

        if (mEffect.transitionSpeed() != nano::LeafEffect::invalidParam()) {
            sstream << "<b>Trans Time:</b> " << mEffect.transitionSpeed() * 10 << "ms<br>";
        }
        if (mEffect.routineSpeed() != nano::LeafEffect::invalidParam()) {
            sstream << "<b>Routine Time:</b> " << mEffect.routineSpeed() * 10 << "ms<br>";
        }
        if (mEffect.mainColorProb() != nano::LeafEffect::invalidParam()) {
            sstream << "<b>Color Prob:</b> " << mEffect.mainColorProb() << "%<br>";
        }
        return QString(sstream.str().c_str());
    }

    /// displays the name of the effect
    QLabel* mName;

    /// displays the metadata of the effect
    QLabel* mMetadata;

    /// displays the colors in the effect
    cor::PaletteWidget* mPalette;

    /// checkbox to select/deselect effect
    cor::CheckBox* mCheckBox;

    /// button to save a palette locally.
    QPushButton* mSaveLocallyButton;

    /// stores the effect data.
    nano::LeafEffect mEffect;

    /// true to display the checkbox, false if not. defaults to true.
    bool mDisplayCheckbox;

    /// true to display the save locally button, false if not. defaults to true.
    bool mDisplaySaveLocally;
};

} // namespace nano

#endif // LEAFEFFECTWIDGET_H
