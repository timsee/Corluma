#ifndef TOPMENU_H
#define TOPMENU_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>

#include "lightingpage.h"
#include "corlumaslider.h"
#include "corlumabutton.h"
#include "icondata.h"
#include "datalayer.h"

/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 *        in their QStackedWidget.
 */
enum class EPage {
    eColorPage,
    eGroupPage,
    eConnectionPage
};


/*!
 * \brief The TopMenu class is the top menu on the main window of Corluma. It contains brightness
 *        and on/off controls for all lights, as well as buttons to navigate to all the main
 *        pages of the application.
 */
class TopMenu : public QWidget
{
    Q_OBJECT
public:
    explicit TopMenu(DataLayer* data, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~TopMenu();

    /*!
     * \brief updateBrightnessSlider update the brightness slider at the top of the mnu based on the current
     *        light data.
     */
    void updateBrightnessSlider();

    /*!
     * \brief deviceCountChangedOnConnectionPage handles the case when the device count changes.
     */
    void deviceCountChangedOnConnectionPage();

    /// special case, happens only if only white LEDs are found. Opens a special color page and disables the group page.
    void hueWhiteLightsFound();

signals:

    /// sent out whenever a button is pressed. Keys are the names of the buttons, such as "settings"
    void buttonPressed(QString key);

    /// the new value of the brightness slider
    void brightnessChanged(int newValue);

public slots:
    /*!
     * \brief toggleOnOff Connected to the button in the top left of the GUI at all times.
     *        Toggles between running the current routine at current settings, and off.
     */
    void toggleOnOff();

    /*!
     * \brief brightnessSliderChanged Connected to the the slider at the top, this takeas a value between 0-100
     *        and sends that value to the lights to control how bright they are.
     */
    void brightnessSliderChanged(int);


    /*!
     * \brief updateMenuBar used to update the menu bar to app state changes.
     */
    void updateMenuBar();

    /*!
     * \brief updateSingleColor updates the icon for the single color page and the menu bar and on/off button
     *         based off of the color emitted.
     */
    void updateSingleColor(QColor);

    /*!
     * \brief updatePresetColorGroup updates the icon for the preset color page and the menu bar and on/off button
     *        based off of the two integers provided. The first integer should be cast to a ELightingRoutine and the
     *        second should be cast to a EColorGroup.
     */
    void updatePresetColorGroup(int, int);

    /*!
     * \brief deviceCountChangedOnConnectionPage handles the case when the device count reaches zero. This gets signaled
     *        from the data layer whereas deviceCOuntChangedOnConnectionPage gets signaled from the connection page.
     */
    void deviceCountReachedZero();

    /*!
     * \brief resizeMenuIcon resizes the icon for either the settings button or the connections button
     * \param button pointer to button to resize
     * \param iconPath path to icon, if it needs to be loaded again.
     */
    void resizeMenuIcon(QPushButton *button, QString iconPath);
protected:
    /// resizes assets in the widget
    void resizeEvent(QResizeEvent *event);

private slots:

    /// called when connection button is pressed.
    void connectionButtonPressed();

    /// called when the color button is pressed.
    void colorButtonPressed();

    /// called when the group button is pressed.
    void groupButtonPressed();

    /// called when the settings button is pressed.
    void settingsButtonPressed();
private:

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    CorlumaSlider *mBrightnessSlider;

    /*!
     * \brief mOnOffButton button for turning all lights on and off.
     */
    QPushButton *mOnOffButton;

    /*!
     * \brief mSettingsButton button for settings page
     */
    QPushButton *mSettingsButton;

    /*!
     * \brief mConnectionButton button for connection page
     */
    QPushButton *mConnectionButton;

    /*!
     * \brief mColorPageButton button for color page
     */
    CorlumaButton *mColorPageButton;

    /*!
     * \brief mGroupPageButton button for group page
     */
    CorlumaButton *mGroupPageButton;

    /// layout for entire widget
    QVBoxLayout *mLayout;

    /// layout for top part of widget
    QHBoxLayout *mTopLayout;

    /// layout for bottom of widget, contains buttons for pages.
    QHBoxLayout *mBottomLayout;

    /// data layer, contains intended state for all devices.
    DataLayer *mData;

    /*!
     * \brief mShouldGreyOutIcons cahced satte of whether any device is selected. If none
     *        are selected, icons that require a device are all greyed out.
     */
    bool mShouldGreyOutIcons;

    /*!
     * \brief mIconData used to generate the icons in the menu bar.
     */
    IconData mIconData;

};

#endif // TOPMENU_H
