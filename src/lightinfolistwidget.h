#ifndef LIGHTINFOLISTWIDGET_H
#define LIGHTINFOLISTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "appsettings.h"
#include "comm/commhue.h"
#include "comm/hue/hueinfowidget.h"
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

    /// getter for scroll area
    LightInfoScrollArea* scrollArea() { return mLightInfoScrollArea; }

    /*!
     * \brief resize size the widget programmatically
     */
    void resize();

signals:
    /// emits when a light name should change
    void lightNameChanged(const QString&, const QString&);

    /// emits the unique ID of the light that will be deleted.
    void deleteLight(QString);

private slots:
    /*!
     * \brief lightNameChagned a LightInfoWidget is signaling that its name should change. This
     * signal is forwarded to a LightInfoWidget signal.
     *
     * \param key device index represented as a string
     * \param name new name of the light
     */
    void nameChanged(QString key, QString name) { emit lightNameChanged(key, name); }

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
    void deleteButtonPressed(bool);

private:
    /// scroll area that contains the information about the lights
    LightInfoScrollArea* mLightInfoScrollArea;

    /// button for deleting the currently selected widget
    QPushButton* mDeleteButton;
};


#endif // LIGHTINFOLISTWIDGET_H
