#ifndef HUESETTINGSPAGE_H
#define HUESETTINGSPAGE_H

#include <QWidget>

namespace Ui {
class HueSettingsPage;
}

/*!
 * \brief The HueSettingsPage class Unimplemented page that gives Hue-specific Settings
 *        and information.
 */
class HueSettingsPage : public QWidget
{
    Q_OBJECT

public:

    /*!
     * Constructor
     */
    explicit HueSettingsPage(QWidget *parent = 0);

    /*!
     * Decosntructor
     */
    ~HueSettingsPage();

signals:
    /*!
     * \brief settingsPageIsStandard signaled when top menu is pressed. True if settings page should
     *        show standard settings, false if showing hue-specific settings.
     */
    void settingsPageIsStandard(bool);

private slots:

    /*!
     * \brief standardButtonPressed standard settings button pressed on the top menu.
     */
    void standardButtonPressed(bool);

    /*!
     * \brief hueButtonPressed hue settings bustton pressed on the top menu.
     */
    void hueButtonPressed(bool);

protected:
    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

private:
    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::HueSettingsPage *ui;
};

#endif // HUESETTINGSPAGE_H
