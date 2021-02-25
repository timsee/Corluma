#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QWidget>

#include "comm/commtype.h"
#include "cor/objects/groupstate.h"
#include "cor/widgets/listitemwidget.h"
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
class GroupStateWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    explicit GroupStateWidget(const cor::GroupState& groupState, QWidget* parent);

    void updateState(const cor::GroupState& groupState);

    void setChecked(bool checked);

    /*!
     * \brief checked getter for checked state
     *
     * \return true if checked, false otherwise
     */
    bool checked() { return mIsChecked; }

    /// getter for name of light
    QString name() const noexcept { return mName->text(); }

    QString uniqueID() const noexcept { return mGroupState.stringUniqueID(); }

    const cor::GroupState& groupState() { return mGroupState; }

    /// getter for current light
    const cor::LightState& state() const noexcept { return mGroupState.state(); }

    /// programmatically determine if the user can interact with the widget by clicking on it. true
    /// if they can, false if they cannot.
    void allowInteraction(bool allowInteraction) { mAllowInteraction = allowInteraction; }

    void displayState(bool shouldDisplayState) { mDisplayState = shouldDisplayState; }

signals:
    /*!
     * \brief clicked emited whenever a mouse press is released anywhere on the widget
     */
    void clicked(QString);

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

    cor::GroupState mGroupState;

    /// pixmap used by main icon
    QPixmap mIconPixmap;

    bool mIsChecked;

    /*!
     * \brief mIconData creates a QPixmap that represents the current light states
     * which gets displayed on the widget.
     */
    IconData mIconData;

    /// true if interaction is allowed, false if it is disabled.
    bool mAllowInteraction;

    /// forces initial update to render
    bool mHasRendered;

    /// true if state should be displayed, false if just the metadata should be displayed
    bool mDisplayState;

    /// font point size for widget
    int mFontPtSize;

    /*!
     * \brief mName name of the light
     */
    QLabel* mName;
};


#endif // STATEWIDGET_H
