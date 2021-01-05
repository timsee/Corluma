#ifndef LIGHTINFOLISTWIDGET_H
#define LIGHTINFOLISTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "appsettings.h"
#include "comm/hue/hueinfowidget.h"
#include "comm/hue/huemetadata.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"
#include "lightinfoscrollarea.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LightInfoListWidget class contains a list of Philips Hue lights. Clicking on
 * an entry of the list shows more information about the light, such as its software version
 * and model number. This widget also allows the user to rename individual lights or delete
 * the light from the Bridge. If the light is deleted from the Bridge, then it cannot be controlled
 * again until it is rediscovered.
 */
class LightInfoListWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit LightInfoListWidget(QWidget* parent);

    /// update a light name
    void updateLightName(QString, QString);

    /// add a light to the info widget.
    void addLight(const HueMetadata& light) { mLightInfoScrollArea->addLight(light); }

    /// delete a light from the display.
    void deleteLightFromDisplay(QString);

    /// getter for scroll area
    LightInfoScrollArea* scrollArea() { return mLightInfoScrollArea; }

    /// change the row height.
    void changeRowHeight(int rowHeight);

    /*!
     * \brief resize size the widget programmatically
     */
    void resize();

signals:
    /// emits the unique ID of the light that will be deleted.
    void deleteLight(QString);

    /// signals when any widget in the list sends a changeName signal. Sends the uniqueID of the
    /// light and the name of the light.
    void changeLightName(QString key, QString name);

    /// handles when the find new light button is clicked.
    void findNewLightClicked();

private slots:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief lightClicked an individual widget has been clicked and has sent out
     * its key.
     */
    void lightInfoClicked(const QString&, bool);

    /*!
     * \brief deleteButtonPressed delete button pressed, which triggers deleting a hue
     * light from a bridge.
     */
    void findNewLightButtonPressed(bool);

    /// delete button is pressed from any widget in the list
    void deleteButtonPressed(QString uniqueID, QString name);

    /// change name button is pressed from any widget in the list
    void changeNamePressed(QString uniqueID, QString name);

private:
    /// scroll area that contains the information about the lights
    LightInfoScrollArea* mLightInfoScrollArea;

    /// button for finding new lights.
    QPushButton* mNewLightButton;
};


#endif // LIGHTINFOLISTWIDGET_H
