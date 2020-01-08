#ifndef COR_BUTTON_H
#define COR_BUTTON_H

#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/lightstate.h"
#include "icondata.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CorlumaButton class provides all of the buttons used within the application.
 * All buttons have an icon, while some have labels or extra logic attached.
 *
 * There are currently three different ways you can set up a button. A standard button
 * emits a EPalette and a ERoutine and doesn't have a label. A labeled button
 * emits a EPalette and a ERoutine, and it also has a label at the bottom of
 * the button. A menu button emits a page number, and is used by the main menu.
 *
 */
class Button : public QPushButton {
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    explicit Button(QWidget* parent, const cor::LightState& state)
        : QPushButton(parent),
          mShouldResizeIcon{true},
          mLabelMode{false},
          mIconData{4, 4},
          mState(state) {
        setCheckable(!mLabelMode);
        connect(this, SIGNAL(clicked(bool)), this, SLOT(handleButton()));
        mIconData.setRoutine(state);
        resizeIcon();
    }

    /*!
     * \brief updateRoutine show a routine on the button
     * \param state the state to display
     */
    void updateRoutine(const cor::LightState& state) {
        mState = state;
        mIconData.setRoutine(state);
        QPixmap pixmap = mIconData.renderAsQPixmap();
        pixmap = pixmap.scaled(mIconSize.width(),
                               mIconSize.height(),
                               Qt::KeepAspectRatio,
                               Qt::FastTransformation);
        setIcon(QIcon(pixmap));
    }

    /// getter for routine object
    const cor::LightState& state() { return mState; }

    /*!
     * \brief setLabelMode true to hide the border and make it non-interactable. False to turn the
     * button back into a button.
     * \param isLabel true to set into label mode, false to set into button mode
     */
    void setLabelMode(bool isLabel) {
        if (isLabel != mLabelMode) {
            mLabelMode = isLabel;
            setCheckable(!mLabelMode);
            if (mLabelMode) {
                setStyleSheet("border:none;");
            } else {
                setStyleSheet("");
            }
        }
    }

    /*!
     * \brief resizeIcon resize icon used for QPushButton.
     */
    void resizeIcon() {
        int size = std::min(this->size().height(), this->size().width());
        mIconSize = QSize(int(size * 0.6f), int(size * 0.6f));
        QPixmap pixmap = mIconData.renderAsQPixmap();
        pixmap = pixmap.scaled(mIconSize.width(),
                               mIconSize.height(),
                               Qt::KeepAspectRatio,
                               Qt::FastTransformation);
        setIcon(QIcon(pixmap));
        setIconSize(mIconSize);
    }

    /// set to true to resize automatically, set to false to not resize automatically
    void resizeIconAutomatically(bool shouldResize) { mShouldResizeIcon = shouldResize; }

signals:
    /*!
     * \brief buttonClicked sent only when setupAsStandardButton has been called.
     */
    void buttonClicked(cor::LightState);

private slots:
    /*!
     * \brief handleButton listens for a click on the button.
     */
    void handleButton() { emit buttonClicked(mState); }

protected:
    /*!
     * \brief resizeEvent used to resize the icon on the QPushButton.
     */
    virtual void resizeEvent(QResizeEvent*) {
        if (mShouldResizeIcon) {
            resizeIcon();
        }
    }

private:
    /// size for icon
    QSize mIconSize;

    /// true if should automatically resize, false otherwise.
    bool mShouldResizeIcon;

    /// true if in label mode, false otherwise
    bool mLabelMode;

    /*!
     * \brief mIconData icon data used by the button's
     *        icon.
     */
    IconData mIconData;

    /// the state of the light to display
    cor::LightState mState;
};

} // namespace cor

#endif // COR_BUTTON_H
