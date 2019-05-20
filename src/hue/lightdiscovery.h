#ifndef HUELIGHTDISCOVERY_H
#define HUELIGHTDISCOVERY_H

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/page.h"
#include "cor/topwidget.h"
#include "searchwidget.h"

namespace hue {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LightDiscovery class is a widget that provides a UI for
 * discovering new hue lights. The Bridge gets its scanning state turned
 * on for automatic discovery. The user can also input serial numbers for the bridge
 * to use for manual discovery. All new lights found from this discovery state are added
 * to the widget in a list.
 */
class LightDiscovery : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /// constructor
    explicit LightDiscovery(QWidget* parent, CommLayer* comm);

    /*!
     * \brief resize size the widget programmatically
     *
     * \param resizeFullWidget true to resize the widget itself, false to just
     *        resize its contents.
     */
    void resize(bool resizeFullWidget = true);

    /// called when the widget is shown
    void show(const hue::Bridge& bridge);

    /// called when the widget is hidden
    void hide();

signals:

    /*!
     * \brief closePressed called when close button is pressed.
     */
    void closePressed();

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private slots:

    /// close button pressed to hide the widget.
    void closeButtonPressed(bool);

    /// search button pressed
    void searchButtonPressed(bool);

    /*!
     * \brief plusButtonClicked called whenever the plus button is clicked
     */
    void plusButtonClicked();

    /*!
     * \brief minusButtonClicked called whenever the minus button is clicked
     */
    void minusButtonClicked();

    /*!
     * \brief discoveryRoutine routine called on a timer for rendering changes
     *        and changing states.
     */
    void discoveryRoutine();

private:
    /// comm layer
    CommLayer* mComm;

    /// the currently selected bridge
    hue::Bridge mBridge;

    /// button to trigger searching on a bridge
    QPushButton* mSearchButton;

    /// timer for rendering and updating state of discovery routine
    QTimer* mDiscoveryTimer;

    /// top widget with title and close button
    cor::TopWidget* mTopWidget;

    /// widget for entering serial numbers and displaying names of discovered lights
    SearchWidget* mSearchWidget;

    /// layout
    QVBoxLayout* mLayout;
};

} // namespace hue
#endif // HUELIGHTDISCOVERY_H
