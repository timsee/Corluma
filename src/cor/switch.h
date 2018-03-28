#ifndef SWITCH_H
#define SWITCH_H

#include <QWidget>
#include <QPushButton>

/// state of the switch
enum class ESwitchState {
    eOn,
    eOff,
    eDisabled
};

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Switch class provides a very basic GUI for an on/off switch.
 *        The switch can have its state set programmatically or will have it swapped
 *        whenever the user clicks the button.
 */
class Switch : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit Switch(QWidget *parent = 0);

    /*!
     * \brief setSwitchState programmatically set the state of the switch. If disabled,
     *        the switch will not react to the users touches until it is put into a either the
     *        on or off state again.
     * \param state the new state of the switch
     */
    void setSwitchState(ESwitchState state);

    /// programmtically turn the switch on
    void switchOn()  { setSwitchState(ESwitchState::eOn); }

    /// programmatically turn the switch off
    void switchOff() { setSwitchState(ESwitchState::eOff); }

signals:

    /// emitted when the switch is clicked true turned to on, false if turned to off.
    void switchChanged(bool);

protected:

    /// resize the widget
    virtual void resizeEvent(QResizeEvent *);

private slots:

    /// handles when the checkbox button is clicked
    void buttonPressed(bool);

private:

    /// state of the switch
    ESwitchState mState;

    /// button that displays switched on and switched off states
    QPushButton *mSwitch;

    /// helper to resize the icon
    void resizeIcon();

    /// icon for the switch
    QPixmap mSwitchIcon;
};

}

#endif // SWITCH_H
