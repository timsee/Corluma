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

enum class EListLightWidgetType {
    standard,
    fullBrightnessBar
};

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
     * \param light light for the widget
     * \param setHighlightable if true, the widget highlights itself, if false,
     * it does not highlight.
     * \param parent parent widget
     */
    explicit ListLightWidget(const cor::Light& light,
                             bool setHighlightable,
                             EListLightWidgetType type,
                             QWidget* parent);

    /*!
     * \brief updateWidget update the widget with a new state for the light.
     *
     * \param light the new state of the light
     * \param colors all the color groups in the data layer, in case the light uses
     * the color groups
     */
    void updateWidget(const cor::Light& light);

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
    bool isReachable() const noexcept { return mLight.isReachable(); }

    /// getter for name of light
    QString name() const noexcept { return mName->text(); }

    /// getter for current light
    const cor::Light& light() const noexcept { return mLight; }

    /// programmatically determine if the user can interact with the widget by clicking on it. true
    /// if they can, false if they cannot.
    void allowInteraction(bool allowInteraction) { mAllowInteraction = allowInteraction; }

    /// true if the state of the widget should display, false if it should display the metadata
    /// (name of light, type of light)
    void displayState(bool shouldDisplayState) { mDisplayState = shouldDisplayState; }

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
    /// programmatically resizes the widget
    void resize();

    /// spacer to use when creating list light widgets
    int spacer();

    /// region used for the state icon
    QRect stateIconRegion();

    /// truncates and modifies the name so that it fits, if necessary
    QString createName(QString name);

    /// pixmap used by main icon
    QPixmap mIconPixmap;

    /// pixmap shown when no connection can be established
    QPixmap mNoConnectionPixmap;

    /// type of ListLightWidget
    EListLightWidgetType mType;

    /// hardware type for the light we're displaying
    ELightHardwareType mHardwareType;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// state that is being displayed
    cor::LightState mState;

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

    /// true if interaction is allowed, false if it is disabled.
    bool mAllowInteraction;

    /// true if state should be displayed, false if just the metadata should be displayed
    bool mDisplayState;

    /// forces initial update to render
    bool mHasRendered;

    /// resizes the icons for the widget
    void resizeIcons();

    /// font point size for widget
    int mFontPtSize;

    /// light stored by widget
    cor::Light mLight;

    /*!
     * \brief mName name of the light
     */
    QLabel* mName;

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel* mTypeIcon;

    /// stores the last icon size, to check if the pixmaps need to be recomputed.
    QSize mLastIconSize;

    /// true if standard widget does not have room to display, false if theres enough room
    bool mCondenseStandardWidget;
};


#endif // LIGHTSLISTWIDGET_H
