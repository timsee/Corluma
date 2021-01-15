#ifndef HUELIGHTINFOWIDGET_H
#define HUELIGHTINFOWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

#include "comm/hue/huemetadata.h"
#include "comm/hue/hueprotocols.h"

namespace hue {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
    explicit HueInfoWidget(HueMetadata light, QWidget* parent);

    /*!
     * \brief updateLight update the light used internally to fill the widget
     *
     * \param light new data for the light.
     */
    void updateLight(HueMetadata light);

    /// change the row height of the widget
    void changeRowHeight(int rowHeight) {
        mRowHeight = rowHeight;
        resize();
    }

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

    /// getter for HueMetadata being represented.
    HueMetadata metadata() { return mLight; }

    /// true if details are hidden, false otherwise
    bool detailsHidden() { return mHideDetails; }

    /// resize programmatically
    void resize();

signals:
    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

    /// emits when the delete button is pressed, emits the uniqueID of the light and its name
    void clickedDelete(QString key, QString name);

    /// emits when the change name button is presed, emits the unique ID fo the light and its
    /// current name.
    void clickedChangeName(QString key, QString name);

    /*!
     * \brief changedName emits its key and the new name given by the
     *        when the name is changed.
     *
     * \param key key for the widget
     * \param name new name for the widget.
     */
    void changedName(QString key, QString name);

private slots:

    /// handles when the change name button is pressed.
    void changeNameButtonPressed(bool);

    /// handles when the delete button is pressed.
    void deleteButtonPressed(bool);

    /*!
     * \brief emits the key of the light
     *        and the new name requested for the light
     *
     * \param newName new name for the light
     */
    void nameChanged(QString newName) { emit changedName(mKey, newName); }

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

    /// height of a row.
    int mRowHeight;

    /// amount of space on the bottom of the widget.
    int mBottomSpacer;

    /// name of Hue Light as an editable field.
    QLabel* mName;

    /// displays the model ID of the Hue Light.
    QLabel* mModelID;

    /// displays the current software version of the Hue Light.
    QLabel* mSoftwareVersion;

    /// displays the type of the Hue Light, such as "bloom"
    QLabel* mType;

    /// unique ID for Hue Light hardware.
    QLabel* mUniqueID;

    /// button to change the bridge's name
    QPushButton* mChangeNameButton;

    /// button to delete the bridge.
    QPushButton* mDeleteButton;

    /// stored data the Hue Light being displayed by this widget.
    HueMetadata mLight;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel* mTypeIcon;
};

} // namespace hue

#endif // HUELIGHTINFOWIDGET_H
