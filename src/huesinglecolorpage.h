#ifndef HUESINGLECOLORPAGE_H
#define HUESINGLECOLORPAGE_H

#include <QWidget>
#include "lightingpage.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

enum class EHuePageType {
    eRGB,
    eAmbient
};

namespace Ui {
class HueSingleColorPage;
}

/*!
 * \brief The HueSingleColorPage a single color page designed for hues only.
 *        It does not have the lighting routines that the other communication streams
 *        have and instead it has an ambient slider.
 */
class HueSingleColorPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit HueSingleColorPage(QWidget *parent = 0);

    /*!
     * Deconstructor
     */
    ~HueSingleColorPage();

    /*!
     * \brief changePageType change the hue page type to display a different color picker.
     * \param page the new page type for the hue page.
     */
    void changePageType(EHuePageType page);

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief singleColorChanged Used to signal back to the MainWindow that the main color of the
     *        single color page has changed.
     */
    void singleColorChanged(QColor);

    /*!
     * \brief brightnessChanged signaled whenever the brightness is changed from the color temperature asset.
     */
    void brightnessChanged(int);

protected:
    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called as the page is hidden. This happens when a new page
     *        is displayed.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

    /*!
     * \brief mousePressEvent called only onnce when a mouse press is clicked down. Events not
     *        directly on top of the ambient picker are ignored.
     */
    virtual void mousePressEvent(QMouseEvent *);
    /*!
     * \brief mouseMoveEvent called when a mouse press is clicked down and moves. This event gets
     *        called very frequently, so the events are thorttled by a timer. Also, events not
     *        directly on top of the ambient picker are ignored.
     */
    virtual void mouseMoveEvent(QMouseEvent *);
    /*!
     * \brief mouseReleaseEvent called when a mouse press is released Events not
     *        directly on top of the ambient picker are ignored.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent used to repaint the widget to override standard rendering.
     */
    void paintEvent(QPaintEvent *);

private slots:

    /*!
     * \brief colorChanged signaled whenever the ColorPicker chooses a new color.
     */
    void colorChanged(QColor);

    /*!
     * \brief ambientValueChanged the color temperature slider's value changed.
     */
    void ambientValueChanged(int);

    /*!
     * \brief resetThrottleFlag called by the throttle timer to allow commands to
     *        be sent again. This gets called on a loop whenever ambient picker is being
     *        used in order to prevent clogging the communication stream.
     */
    void resetThrottleFlag();

private:

    /*!
     * \brief resizeAmbientPicker umm.. resizes the ambient color temperature picker.
     */
    void resizeAmbientPicker();

    /*!
     * \brief handleMouseEvent handles all the mouse events used in the ambient picker.
     *        The mousePressEvent and mouseReleaseEvent both map to this function.
     * \param event the mouse event that is getting processed.
     */
    void handleMouseEvent(QMouseEvent *event);

    /*!
     * \brief mThrottleTimer used to throttle the events happening from an asset so that they only
     *        send through the system every X milliseconds instead of on every possible update.
     */
    QTimer *mThrottleTimer;

    /*!
     * \brief mPressTime used by the throttle timer to ignore clicks and to focus
     *        on long presses.
     */
    QTime mPressTime;

    /*!
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

    /// normalized x value from current mouse event point in color temperature assets.
    float mNormalizedX;

    /// normalized y value from current mouse event point in color temperature assets.
    float mNormalizedY;

    /// last hue color temperature value
    int mHueCTValue;

    /// last brightness value from color temperature assets.
    int mBrightness;

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::HueSingleColorPage *ui;

    /*!
     * \brief mPageType the type of hue page being displayed. Currently it can be either
     *        an RGB picker for RGB lights or a Ambient slider for ambient lights.
     */
    EHuePageType mPageType;
};

#endif // HUESINGLECOLORPAGE_H
