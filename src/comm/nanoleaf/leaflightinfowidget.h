#ifndef NANOLEAFLIGHTINFOWIDGET_H
#define NANOLEAFLIGHTINFOWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "comm/nanoleaf/leafmetadata.h"
#include "cor/protocols.h"
#include "editablefieldwidget.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LeafLightInfoWidget class is a widget made to be displayed in lists
 * that contains the name of a light and much of its hardware information such
 * as its model number and software version. The user can also edit the name of
 * light from this widget. If an edit is made, this widget signals out the new name
 * for it to be sent to the bridge.
 */
class LeafLightInfoWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit LeafLightInfoWidget(const nano::LeafMetadata& light, QWidget* parent);

    /*!
     * \brief updateLight update the light used internally to fill the widget
     *
     * \param light new data for the light.
     */
    void updateLight(const nano::LeafMetadata& light);

    /*!
     * \brief hideDetails true to show only the basic details of a widget, false
     *        to show all of the details.
     *
     * \param shouldHide true to show only the basic details of a widget, false
     *        to show all of the details.
     */
    void hideDetails(bool shouldHide);

    /*!
     * \brief setChecked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     *
     * \param checked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     */
    void setChecked(bool checked);

    /// true if checked, false otherwise.
    bool checked() { return mIsChecked; }

    /// getter for the key of the widget.
    const QString& key() { return mKey; }

    /// getter for LeafLight being represented.
    nano::LeafMetadata light() { return mLight; }

    /// true if details are hidden, false otherwise
    bool detailsHidden() { return mHideDetails; }

    /// resizes widget programmatically
    void resize();

signals:
    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

    /*!
     * \brief changedName emits its key and the new name given by the EditableFieldWidget
     *        when the name is changed.
     *
     * \param key key for the widget
     * \param name new name for the widget.
     */
    void changedName(EProtocolType, QString key, QString name);

private slots:

    /*!
     * \brief nameChanged Signaled by EditableFieldWidget, emits the key of the light
     *        and the new name requested for the light
     *
     * \param newName new name for the light created by the EditableFieldWidget
     */
    void nameChanged(QString newName) { emit changedName(EProtocolType::nanoleaf, mKey, newName); }

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /// handles when the widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /// Unique key based on device index of Hue Light.
    QString mKey;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// true if  all info should show, false otherwise.
    bool mHideDetails;

    /// name of nanoleaf as an editable field.
    EditableFieldWidget* mName;

    /// displays the model ID of the Hue Light.
    QLabel* mModelID;

    /// displays the IP address
    QLabel* mIPAdress;

    /// firmware
    QLabel* mFirmware;

    /// displays the serial number of the nanoleaf.
    QLabel* mSerialNumber;

    /// stored data for the light being displayed by this widget.
    nano::LeafMetadata mLight;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel* mTypeIcon;

    /// layout for top of widget
    QHBoxLayout* mTopLayout;

    /// main layout for widget
    QVBoxLayout* mMainLayout;
};

} // namespace nano

#endif // NANOLEAFLIGHTINFOWIDGET_H
