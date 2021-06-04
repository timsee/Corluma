#ifndef GROUPBUTTON_H
#define GROUPBUTTON_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include "cor/widgets/checkbox.h"
#include "cor/widgets/palettewidget.h"
#include "utils/painterutils.h"

namespace cor {

enum class EArrowState { disabled, closed, open };

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
    explicit GroupButton(const QString& text, QWidget* parent) : GroupButton(text, text, parent) {}

    GroupButton(const QString& key, const QString& text, QWidget* parent);

    /// key for group
    const QString& key() const noexcept { return mKey; }

    //-----------------
    // Select/Deselect All Checkbox
    //-----------------

    /// true to show the button, false to not.
    void showSelectAllCheckbox(bool shouldShowButton) {
        mShowSelectAll = shouldShowButton;
        resize();
    }

    /// handle the state of the select all button
    bool handleSelectAllCheckbox(std::uint32_t checkedDevicesCount, uint32_t reachableDevicesCount);

    /// set whether the button should show select all or deselect all
    void setSelectAll(bool shoudlSelect);

    /// update the border rectangle
    void changePaintRectOptions(EPaintRectOptions options) {
        mRectOptions = options;
        update();
    }

    //-----------------
    // Open/Closed Arrow
    //-----------------

    /// getter for arrow.
    EArrowState arrowState() const noexcept { return mArrowState; }

    /// true if arrow is open, false otherwise
    bool isArrowOpen() const noexcept { return mArrowState == EArrowState::open; }

    /// change the arrow state, this can make the arrow disabled (and thus hidden), closed, and
    /// opened.
    void changeArrowState(EArrowState arrowState);

    //-----------------
    // Text title
    //-----------------

    /*!
     * \brief changeText this changes the text of a GroupButton. Warning: if the text is used as a
     * unique key, this may break systems.
     * \param newText new text to display with the group button
     */
    void changeText(const QString& newText) { mTitle->setText(newText); }

    /// getter for text
    QString text() { return mTitle->text(); }

    //-----------------
    // Highlighting
    //-----------------

    /// if true, the highlight takes into account how many lights are selected from within the
    /// group. if all lights are selected, the highlight is fully highlighted, otherwise, the
    /// intensity of the highlight is based off of the ratio of selected lights vs unselected
    /// lights. If false, the highlight is either fully highlighted, or not higlighted at all.
    void highlightByCountOfLights(bool shouldHighlight) {
        mHighlightByCountOfLights = shouldHighlight;
    }

    /// getter for if the widget is highlighted or not.
    bool isHighlighted() const noexcept { return mCheckedCount != 0; }

    //-----------------
    // Highlighting
    //-----------------

    /// getter as to whether states are showign or not
    bool showStates() const noexcept { return mShowStates; }

    //-----------------
    // show light states
    //-----------------

    /// true to show the states, false otherwise
    void showStates(bool showStates) {
        if (mShowStates != showStates) {
            mShowStates = showStates;
            resize();
        }
    }

    /// updates the shown states, sets states to showing if they are not already showing.
    void updateStates(const std::vector<cor::LightState>& lightStates) {
        mShowStates = true;
        mPaletteWidget->show(lightStates);
        if (!mPaletteWidget->isShowingAnything()) {
            mTitle->setStyleSheet(cor::kTransparentAndBoldStylesheet);
        } else {
            mTitle->setStyleSheet(cor::kTransparentAndBoldStylesheet);
        }
    }

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
    /// resizes widget programmatically
    void resize();

    /// preferred size of icon
    QSize iconSize();

    /// getter for current pixmap
    const QPixmap& currentPixmap();

    /// key for the widget
    QString mKey;

    /// true if widget is selected, false otherwise
    bool mIsSelected;

    /// true if showing button, false otherwise
    bool mShowSelectAll;

    /// if true, the highlight reflects the number of selected lights. if false, the highlight is
    /// binary, its either highlighted, or not.
    bool mHighlightByCountOfLights;

    /// true to show the states, false otherwise.
    bool mShowStates;

    /// state for arrow
    EArrowState mArrowState;

    /// count of reachable devices
    std::uint32_t mReachableCount;

    /// count of checked devices
    std::uint32_t mCheckedCount;

    /*!
     * \brief mButtonHeight height of a button
     */
    int mButtonHeight;

    /// checkbox for selecting/deselecting a group.
    cor::CheckBox* mCheckBox;

    /*!
     * \brief mArrowIcon an arrow in the top right of the widget. If its pointing right, no
     *        buttons are shown. If its pointing down, all buttons are shown.
     */
    QLabel* mArrowIcon;

    /// widget for displaying palettes.
    cor::PaletteWidget* mPaletteWidget;

    /// label for checkbox
    QLabel* mTitle;

    /// stores the border for the widget's options.
    EPaintRectOptions mRectOptions;
};

} // namespace cor

#endif // GROUPBUTTON_H
