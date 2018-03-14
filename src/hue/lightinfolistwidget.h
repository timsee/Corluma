#ifndef HUELIGHTINFOLISTWIDGET_H
#define HUELIGHTINFOLISTWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

#include "hue/lightinfowidget.h"
#include "comm/commhue.h"
#include "cor/topwidget.h"

namespace hue
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The HueLightInfoListWidget class contains a list of Philips Hue lights. Clicking on
 *        an entry of the list shows more information about the light, such as its software version
 *        and model number. This widget also allows the user to rename individual lights or delete the light
 *        from the Bridge. If the light is deleted from the Bridge, then it cannot be controlled again until
 *        it is rediscovered.
 */
class LightInfoListWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit LightInfoListWidget(QWidget *parent = 0);

    /*!
     * \brief updateLights update the lights displayed in the widget, normally called
     *        right before displaying the widget.
     * \param lights list of lights to load into the HusLightInfoListWidget
     */
    void updateLights(std::list<HueLight> lights);

    /*!
     * \brief resize size the widget programmatically
     * \param resizeFullWidget true to resize the widget itself, false to just
     *        resize its contents.
     */
    void resize(bool resizeFullWidget = true);

signals:
    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// emits the device index of Hue Light as a key and a new name for the light.
    void hueChangedName(QString, QString);

    /// emits the device index of a Hue Light as a key.
    void hueDeleted(QString);

private slots:
    /*!
     * \brief nameChanged HueLightInfoWidget is signaling that its name should change. This signal
     *        is forwarded to a HueLightInfoWidget signal.
     * \param key device index of Hue Light represented as a string
     * \param name new name of the Hue Light.
     */
    void nameChanged(QString key, QString name) { emit hueChangedName(key, name); }

    /*!
     * \brief closePressed close button pressed from top widget.
     */
    void closePressed(bool);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

    /*!
     * \brief lightInfoWidgetClicked an individual widget has been clicked and has sent out
     *        its key.
     */
    void lightInfoWidgetClicked(QString);

    /*!
     * \brief deleteButtonPressed delete button pressed, which triggers deleting a hue
     *        light from a bridge.
     */
    void deleteButtonPressed(bool);

private:

    /// last key clicked
    QString mLastKey;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// title and close button at top of widget.
    cor::TopWidget *mTopWidget;

    /// button for deleting the currently selected widget
    QPushButton *mDeleteButton;

    /// layout for top area of widget.
    QHBoxLayout *mTopLayout;

    /// layout for widget
    QVBoxLayout *mMainLayout;

    /// layout for scroll area
    QVBoxLayout *mScrollLayout;

    /// scroll area for displaying list.
    QScrollArea *mScrollArea;

    /// widgets displayed in scroll area
    std::vector<LightInfoWidget *> mWidgets;

};

}

#endif // HUELIGHTINFOLISTWIDGET_H
