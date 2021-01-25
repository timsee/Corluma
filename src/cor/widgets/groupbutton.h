#ifndef GROUPBUTTON_H
#define GROUPBUTTON_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include "cor/widgets/checkbox.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupButton class is a simple widget that either emits its key if its pressed, or
 * emits whether to select/deselect all if its additional button is pressed
 */
class GroupButton : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit GroupButton(QWidget* parent, const QString& text);

    /// handle the state of the select all button
    bool handleSelectAllButton(std::uint32_t checkedDevicesCount, uint32_t reachableDevicesCount);

    /*!
     * \brief changeText this changes the text of a GroupButton. Warning: if the text is used as a
     * unique key, this may break systems.
     * \param newText new text to display with the group button
     */
    void changeText(const QString& newText) { mTitle->setText(newText); }

    /// key for group
    QString key() const { return mTitle->text(); }

    /// if true, the highlight takes into account how many lights are selected from within the
    /// group. if all lights are selected, the highlight is fully highlighted, otherwise, the
    /// intensity of the highlight is based off of the ratio of selected lights vs unselected
    /// lights. If false, the highlight is either fully highlighted, or not higlighted at all.
    void highlightByCountOfLights(bool shouldHighlight) {
        mHighlightByCountOfLights = shouldHighlight;
    }

    /// true to programmatically highlight, false otherwise
    void highlight(bool shouldHighlight) {
        if (shouldHighlight != mShouldHighlight) {
            mShouldHighlight = shouldHighlight;
            update();
        }
    }

    /// getter for if the widget is highlighted or not.
    bool isHighlighted() const noexcept { return mShouldHighlight; }

    /// true to show the button, false to not.
    void showButton(bool shouldShowButton) { mShowButton = shouldShowButton; }

    /// set whether the button should show select all or deselect all
    void setSelectAll(bool shoudlSelect);

    /// resizes widget programmatically
    void resize();
signals:

    /// emits when the group button is pressed
    void groupButtonPressed(QString key);

    /// emits when the select all butotn is toggled to either select all or deselect all
    void groupSelectAllToggled(QString, bool);

protected:
    /// resize the widget
    virtual void resizeEvent(QResizeEvent*);

    /// emits the group name if its not above the checkbox
    virtual void mouseReleaseEvent(QMouseEvent*);

    /// renders the widget
    virtual void paintEvent(QPaintEvent*);

private slots:

    /// handle when the checkbox is clicked
    void checkBoxClicked(ECheckboxState state);

private:
    /// preferred size of icon
    QSize iconSize();

    /// getter for current pixmap
    const QPixmap& currentPixmap();

    /// true if widget is selected, false otherwise
    bool mIsSelected;

    /// true if showing button, false otherwise
    bool mShowButton;

    /// if true, the highlight reflects the number of selected lights. if false, the highlight is
    /// binary, its either highlighted, or not.
    bool mHighlightByCountOfLights;

    /// true if the widget should be able to be highlighted, false otherwise.
    bool mShouldHighlight;

    /// count of reachable devices
    std::uint32_t mReachableCount;

    /// count of checked devices
    std::uint32_t mCheckedCount;

    /// label for checkbox
    QLabel* mTitle;

    cor::CheckBox* mCheckBox;
};

} // namespace cor

#endif // GROUPBUTTON_H
