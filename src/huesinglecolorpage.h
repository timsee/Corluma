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

private slots:

    /*!
     * \brief colorChanged signaled whenever the ColorPicker chooses a new color.
     */
    void colorChanged(QColor);

    /*!
     * \brief ambientValueChanged the color temperature slider's value changed.
     */
    void ambientValueChanged(int);

private:
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
