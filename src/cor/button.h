#ifndef COR_BUTTON_H
#define COR_BUTTON_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonObject>
#include "icondata.h"
#include "cor/protocols.h"

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CorlumaButton class provides all of the buttons used within the application.
 *        All buttons have an icon, while some have labels or extra logic attached.
 *
 * There are currently three different ways you can set up a button. A standard button
 * emits a EPalette and a ERoutine and doesn't have a label. A labeled button
 * emits a EPalette and a ERoutine, and it also has a label at the bottom of
 * the button. A menu button emits a page number, and is used by the main menu.
 *
 */
class Button : public QPushButton
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    explicit Button(QWidget *parent, const QJsonObject& routine);

    /*!
     * \brief updateRoutine show a routine on the button
     * \param routineObject the json representatino of the routine
     */
    void updateRoutine(const QJsonObject& routineObject);

    /// getter for routine object
    const QJsonObject& routine() { return mRoutineObject; }

    /*!
     * \brief resizeIcon resize icon used for QPushButton.
     */
    void resizeIcon();

    /// set to true to resize automatically, set to false to not resize automatically
    void resizeIconAutomatically(bool shouldResize) { mShouldResizeIcon = shouldResize; }

signals:
    /*!
     * \brief buttonClicked sent only when setupAsStandardButton has been called.
     */
    void buttonClicked(QJsonObject);

private slots:
    /*!
     * \brief handleButton listens for a click on the button.
     */
    void handleButton();

protected:
    /*!
    * \brief resizeEvent used to resize the icon on the QPushButton.
    */
   virtual void resizeEvent(QResizeEvent *);

private:
    /// size for icon
    QSize mIconSize;

    /// true if should automatically resize, false otherwise.
    bool mShouldResizeIcon;

    /*!
     * \brief mIconData icon data used by the button's
     *        icon.
     */
    IconData mIconData;

    /// the json representation of the routine
    QJsonObject mRoutineObject;
};

}

#endif // COR_BUTTON_H
