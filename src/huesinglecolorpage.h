#ifndef HUESINGLECOLORPAGE_H
#define HUESINGLECOLORPAGE_H

#include <QWidget>
#include "lightingpage.h"

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

signals:
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

    /*!
     * \brief rgbButtonPressed top menu button for displaying the RGB color wheel was pressed.
     */
    void rgbButtonPressed(bool);

    /*!
     * \brief temperatureButtonPressed top menu button for using the temperature slider
     *        was pressed.
     */
    void temperatureButtonPressed(bool);

private:
    Ui::HueSingleColorPage *ui;
};

#endif // HUESINGLECOLORPAGE_H
