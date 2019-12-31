#ifndef HARDWARECONNECTIONWIDGET_H
#define HARDWARECONNECTIONWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/// states for the HardwareConnectionWidget
enum class EHardwareConnectionStates {
    noOutgoingFound,
    attemptingOutgoing,
    attemptingIncoming,
    connected
};

/*!
 * \brief The HardwareConnectionWidget class is a simple widget that shows the current state of
 * connecting communication between the Corluma application and a hardware device such as a Philips
 * Bridge. It displays this through the use of two icons and arrows connecting the two icons.
 */
class HardwareConnectionWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit HardwareConnectionWidget(const QString& hardwareIconPath, QWidget* parent);

    /*!
     * \brief changeState change the state of the widget to display a new state. If its given the
     * state that its already in, it will do nothing. \param newState new state for the
     * HardwareConnectionWidget
     */
    void changeState(EHardwareConnectionStates newState);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /// current state for HardwareConnectionWidget
    EHardwareConnectionStates mState;

    /// scaling value for size of pngs
    float mScale;

    /// label for displaying the hardware icon
    QLabel* mHardwareLabel;

    /// pixmap for the hardware icon
    QPixmap mHardwarePixmap;

    /// label for displaying the Corluma icon
    QLabel* mCorlumaLabel;

    /// pixmap for the Corluma icon
    QPixmap mCorlumaPixmap;

    /// label for displaying the arrow that represents the incoming stream
    QLabel* mIncomingArrow;

    /// pixmap for incoming arrow
    QPixmap mIncomingArrowPixmap;

    /// label for displaying the arrow that represents the outgoing stream
    QLabel* mOutgoingArrow;

    /// pixmap for outgoing arrow
    QPixmap mOutgoingArrowPixmap;

    /// layout
    QGridLayout* mLayout;
};

#endif // HARDWARECONNECTIONWIDGET_H