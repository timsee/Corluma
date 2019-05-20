#ifndef SWITCH_H
#define SWITCH_H

#include <QPushButton>
#include <QWidget>

/// state of the switch
enum class ESwitchState { on, off, disabled };

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Switch class provides a very basic GUI for an on/off switch.
 *        The switch can have its state set programmatically or will have it swapped
 *        whenever the user clicks the button.
 */
class Switch : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit Switch(QWidget* parent) : QWidget(parent) {
        mSwitch = new QPushButton(this);
        mSwitch->setCheckable(false);
        connect(mSwitch, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
        mSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // initialize switch state
        mState = ESwitchState::on;
        setSwitchState(ESwitchState::off);
    }


    /*!
     * \brief setSwitchState programmatically set the state of the switch. If disabled,
     *        the switch will not react to the users touches until it is put into a either the
     *        on or off state again.
     *
     * \param state the new state of the switch
     */
    void setSwitchState(ESwitchState state) {
        if (state != mState) {
            mState = state;
            resizeIcon();
        }
    }

    /// getter for switch state
    const ESwitchState& switchState() const noexcept { return mState; }

    /// programmtically turn the switch on
    void switchOn() { setSwitchState(ESwitchState::on); }

    /// programmatically turn the switch off
    void switchOff() { setSwitchState(ESwitchState::off); }

signals:

    /// emitted when the switch is clicked true turned to on, false if turned to off.
    void switchChanged(bool);

protected:
    /// resize the widget
    virtual void resizeEvent(QResizeEvent*) { resizeIcon(); }

private slots:

    /// handles when the checkbox button is clicked
    void buttonPressed(bool) {
        switch (mState) {
            case ESwitchState::on:
                setSwitchState(ESwitchState::off);
                emit switchChanged(false);
                break;
            case ESwitchState::off:
                setSwitchState(ESwitchState::on);
                emit switchChanged(true);
                break;
            case ESwitchState::disabled:
                setSwitchState(ESwitchState::disabled);
                break;
        }
    }

private:
    /// state of the switch
    ESwitchState mState;

    /// button that displays switched on and switched off states
    QPushButton* mSwitch;

    /// helper to resize the icon
    void resizeIcon() {
        switch (mState) {
            case ESwitchState::on:
                mSwitchIcon = QPixmap(":/images/onSwitch.png");
                mSwitch->setChecked(true);
                break;
            case ESwitchState::off:
                mSwitchIcon = QPixmap(":/images/offSwitch.png");
                mSwitch->setChecked(false);
                break;
            case ESwitchState::disabled:
                mSwitchIcon = QPixmap(":/images/closeX.png");
                mSwitch->setChecked(false);
                break;
        }
        mSwitch->setFixedSize(this->width(), this->height());
        QSize newSize = QSize(int(this->width() * 0.8f), int(this->height() * 0.8f));
        mSwitchIcon = mSwitchIcon.scaled(
            newSize.width(), newSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        mSwitch->setIcon(QIcon(mSwitchIcon));
        mSwitch->setIconSize(newSize);
    }

    /// icon for the switch
    QPixmap mSwitchIcon;
};

} // namespace cor

#endif // SWITCH_H
