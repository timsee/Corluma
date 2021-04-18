#ifndef PARENTGROUPWIDGET_H
#define PARENTGROUPWIDGET_H

#include <QWidget>
#include "dropdowntopwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ParentGroupWidget class is derived from the DropdownTopWidget, but has the ability to
 * change its text, as well as highlight based off of the number of reachable and checked lights.
 * This is useful for menus, since it allows the app to show how many lights are selected out of all
 * possible lights for a group.
 */
class ParentGroupWidget : public DropdownTopWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ParentGroupWidget(const QString& key,
                               const QString& name,
                               cor::EWidgetType type,
                               bool hideEdit,
                               QWidget* parent)
        : DropdownTopWidget(key, name, type, hideEdit, parent),
          mReachableCount{0u},
          mCheckedCount{0u} {}

    /// change the text of the dropdownwidget
    void changeText(const QString& text) { mName->setText(text); }

    /// getter for name of parent.
    QString text() { return mName->text(); }

    /// update the checked devices of the group that matches the key
    void updateCheckedLights(std::uint32_t checkedLightCount, std::uint32_t reachableLightCount);

protected:
    /// renders the widget
    virtual void paintEvent(QPaintEvent*);

private:
    /// count of reachable devices
    std::uint32_t mReachableCount;

    /// count of checked devices
    std::uint32_t mCheckedCount;
};

#endif // PARENTGROUPWIDGET_H
