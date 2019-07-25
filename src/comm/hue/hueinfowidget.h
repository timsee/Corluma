#ifndef HUELIGHTINFOWIDGET_H
#define HUELIGHTINFOWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QWidget>

#include "comm/hue/huelight.h"
#include "comm/hue/hueprotocols.h"
#include "editablefieldwidget.h"

namespace hue {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LightInfoWidget class is a widget made to be displayed in lists
 *        that contains the name of a light and much of its hardware information such
 *        as its model number and software version. The user can also edit the name of
 *        light from this widget. If an edit is made, this widget signals out the new name
 *        for it to be sent to the bridge.
 */
class HueInfoWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit HueInfoWidget(HueLight light, QWidget* parent);

    /*!
     * \brief updateLight update the light used internally to fill the widget
     *
     * \param light new data for the light.
     */
    void updateLight(HueLight light);

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

    /// getter for SHueLight being represented.
    HueLight light() { return mLight; }

    /// true if details are hidden, false otherwise
    bool detailsHidden() { return mHideDetails; }

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
    void nameChanged(QString newName) { emit changedName(EProtocolType::hue, mKey, newName); }

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

private:
    /// Unique key based on device index of Hue Light.
    QString mKey;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// true if  all info should show, false otherwise.
    bool mHideDetails;

    /// main layout
    QGridLayout* mLayout;

    /// name of Hue Light as an editable field.
    EditableFieldWidget* mName;

    /// displays the model ID of the Hue Light.
    QLabel* mModelID;

    /// displays the type of the Hue Light, such as "extended color lamp"
    QLabel* mType;

    /// unique ID for Hue Light hardware.
    QLabel* mUniqueID;

    /// displays the current software version of the Hue Light.
    QLabel* mSoftwareVersion;

    /// stored data the Hue Light being displayed by this widget.
    HueLight mLight;
};

} // namespace hue

#endif // HUELIGHTINFOWIDGET_H
