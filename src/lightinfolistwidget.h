#ifndef LIGHTINFOLISTWIDGET_H
#define LIGHTINFOLISTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "appsettings.h"
#include "comm/arducor/arducorinfowidget.h"
#include "comm/commhue.h"
#include "comm/hue/hueinfowidget.h"
#include "comm/nanoleaf/leafcontrollerinfowidget.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"
#include "lightinfoscrollarea.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The HueLightInfoListWidget class contains a list of Philips Hue lights. Clicking on
 * an entry of the list shows more information about the light, such as its software version
 * and model number. This widget also allows the user to rename individual lights or delete
 * the light from the Bridge. If the light is deleted from the Bridge, then it cannot be controlled
 * again until it is rediscovered.
 */
class LightInfoListWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit LightInfoListWidget(QWidget* parent, AppSettings* appSettings);

    LightInfoScrollArea* scrollArea() { return mLightInfoScrollArea; }

    /*!
     * \brief resize size the widget programmatically
     */
    void resize();

    /// pushes in the widget
    void pushIn();

    /// pushes out the widget
    void pushOut();

signals:
    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// emits when a light name should change
    void lightNameChanged(EProtocolType, const QString&, const QString&);

    /// emits the unique ID of the light that will be deleted.
    void deleteLight(QString);

private slots:
    /*!
     * \brief lightNameChagned a LightInfoWidget is signaling that its name should change. This
     * signal is forwarded to a LightInfoWidget signal.
     *
     * \param type protocol type of light
     * \param key device index represented as a string
     * \param name new name of the light
     */
    void nameChanged(EProtocolType type, QString key, QString name) {
        emit lightNameChanged(type, key, name);
    }

    /*!
     * \brief closePressed close button pressed from top widget.
     */
    void closePressed(bool);

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

    /// hue button clicked
    void hueClicked();

    /// nanoleaf button clicked
    void nanoleafClicked();

    /// arducor button clicked
    void arducorClicked();

private:
    /// highlights the protocol type
    void highlightProtocolType();

    /// handle the protocol type thats selected
    bool handleProtocolType();

    /// scroll area that contains the information about the lights
    LightInfoScrollArea* mLightInfoScrollArea;

    /// pointer to app settings
    AppSettings* mAppSettings;

    /// title and close button at top of widget.
    cor::TopWidget* mTopWidget;

    /// button for deleting the currently selected widget
    QPushButton* mDeleteButton;

    /// buttons for changing the protocols
    std::vector<QPushButton*> mProtocolButtons;

    /// current protocol
    EProtocolType mCurrentProtocol;
};


#endif // LIGHTINFOLISTWIDGET_H