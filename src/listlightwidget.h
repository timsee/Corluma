#ifndef LIGHTSLISTWIDGET_H
#define LIGHTSLISTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QWidget>

#include "comm/commtype.h"
#include "cor/widgets/listitemwidget.h"
#include "cor/widgets/statusicon.h"
#include "icondata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListLightWidget class This widget is used on the SettingsPage as a replacement
 * for a QListWidget on the connectionList. It shows more information than a standard
 * QListWidget, by giving an icon that represents the lights states, the name of of the
 * controller, and the index of the light.
 */
class ListLightWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /*!
     * \brief ListLightWidget Constructo
     *
     * \param device device for the widget
     * \param setHighlightable if true, the widget highlights itself, if false,
     * it does not highlight.
     * \param parent parent widget
     */
    explicit ListLightWidget(const cor::Light& device,
                             bool setHighlightable,
                             cor::EWidgetType type,
                             QWidget* parent);

    /*!
     * \brief updateWidget update the widget with a new state for the device.
     *
     * \param device the new state of the device
     * \param colors all the color groups in the data layer, in case the device uses
     * the color groups
     */
    void updateWidget(const cor::Light& device);

    /*!
     * \brief setHighlightChecked set the widget as checked or unchecked. When it is checked it
     *        it will be a light blue color. When it isn't, it will be dark grey.
     *
     * \param checked true to set to checked, false othwerise
     * \return true if successful, false otherwise
     */
    bool setHighlightChecked(bool checked);

    /*!
     * \brief checked getter for checked state
     *
     * \return true if checked, false otherwise
     */
    bool checked() { return mIsChecked; }

    /// true if displaying a reachable state, false otherwise
    bool isReachable() const noexcept { return mIsReachable; }

    /// getter for name of controller
    QString name() const noexcept { return mController->text(); }

    /// getter for current light
    const cor::Light& light() const noexcept { return mLight; }

signals:
    /*!
     * \brief clicked emited whenever a mouse press is released anywhere on the widget
     */
    void clicked(QString);

    /// emits the key and the state of the on/off switch when its clicked
    void switchToggled(QString, bool);

protected:
    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent* event);

    /// called when widget is resized
    void resizeEvent(QResizeEvent*);

private:
    /// spacer to use when creating list light widgets
    int spacer();

    /// size of the region used for the icon
    QSize iconRegion();

    /// truncates and modifies the name so that it fits, if necessary
    QString createName(QString name);

    /// pixmap used by main icon
    QPixmap mIconPixmap;

    /// pixmap shown when no connection can be established
    QPixmap mNoConnectionPixmap;

    /// type of ListLightWidget
    cor::EWidgetType mType;

    /// hardware type for the light we're displaying
    ELightHardwareType mHardwareType;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// state that is being displayed
    cor::LightState mState;

    /// true if displaying a reachable light, false otherwise
    bool mIsReachable;

    /// true if should highlight, false otherwise
    bool mShouldHighlight;

    /*!
     * \brief mIconData creates a QPixmap that represents the current light states
     * which gets displayed on the mStatusIcon.
     */
    IconData mIconData;

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

    /// forces initial update to render
    bool mHasRendered;

    /// resizes the icons for the widget
    void resizeIcons();

    /// stores the last rendered size to check if it needs to rerender again
    QSize mLastRenderedSize;

    /// font point size for widget
    int mFontPtSize;

    /// light stored by widget
    cor::Light mLight;

    /*!
     * \brief mController name of the controller
     */
    QLabel* mController;

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel* mTypeIcon;
};


#endif // LIGHTSLISTWIDGET_H
