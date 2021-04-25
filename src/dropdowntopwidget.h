#ifndef DROPDOWNTOPWIDGET_H
#define DROPDOWNTOPWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/group.h"
#include "cor/protocols.h"
#include "cor/stylesheets.h"
#include "cor/widgets/palettewidget.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DropdownTopWidget class provides a widget that can be used at the top of a dropdown
 * list. It is frequently used in the LightPage and the MoodPage to hide the details of overally
 * lights/moods but still display some meta information. It also provides the user with buttons to
 * edit the contents of the widget, or to select all/none of the contents in some cases.
 */
class DropdownTopWidget : public QWidget {
    Q_OBJECT
public:
    /// constuctor
    explicit DropdownTopWidget(const QString& key,
                               const QString& name,
                               cor::EWidgetType type,
                               bool hideEdit,
                               QWidget* parent);

    /*!
     * \brief showButtons getter that checks if buttons are showing
     * \return  true if buttons are showing, false otherwise.
     */
    bool showButtons() const noexcept { return mShowButtons; }

    /// set to true to show all widgets, false to just show the dropdown widget.
    void showButtons(bool showButtons);

    /// getter as to whether states are showign or not
    bool showStates() const noexcept { return mShowStates; }

    /// true to show the states, false otherwise
    void showStates(bool showStates);

    /// updates the shown states, sets states to showing if they are not already showing.
    void updateStates(const std::vector<cor::LightState>& lightStates) {
        mShowStates = true;
        mPaletteWidget->show(lightStates);
        if (!mPaletteWidget->isShowingAnything()) {
            mName->setStyleSheet(cor::kTransparentAndBoldStylesheet);
        } else {
            mName->setStyleSheet(cor::kGradientStylesheet);
        }
    }

    /// getter for key
    const QString& key() const noexcept { return mKey; }

signals:

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editClicked(QString);

    /*!
     * \brief buttonsShown emitted when the buttons are shown or hidden. emits the key and a boolean
     *        representing whether the buttons are shown.
     */
    void buttonsShown(QString, bool);

    /// emits when the widget is pressed
    void pressed();

    /// signals when the widget is pressed. sends its current text
    void dropdownPressed(QString);

private slots:

    /*!
     * \brief editButtonClicked called when edit button is pressed, sends out an edit signal
     */
    void editButtonClicked(bool) { emit editClicked(mKey); }

    /// sends the widgets current text when the widget is pressed
    void widgetPressed() { emit dropdownPressed(mName->text()); }

protected:
    /// handles when widget resizes
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /*!
     * \brief mName label for name of collection
     */
    QLabel* mName;

private:
    /// the type of dropdowntopwidget
    cor::EWidgetType mType;

    /*!
     * \brief mEditButton button used to edit the collection. Editing can change
     *        the name or the lights contained in the collection.
     */
    QPushButton* mEditButton;

    /*!
     * \brief mArrowIcon an arrow in the top right of the widget. If its pointing right, no
     *        buttons are shown. If its pointing down, all buttons are shown.
     */
    QLabel* mArrowIcon;

    /// widget for displaying palettes.
    cor::PaletteWidget* mPaletteWidget;

    /// pixmap for icon that conveys no buttons being shown
    QPixmap mClosedPixmap;

    /// pixmap for icon that conveys all buttons being shown
    QPixmap mOpenedPixmap;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// key for dropdown widget
    QString mKey;

    /*!
     * \brief mShowButtons true if buttons are showing, false otherwise.
     */
    bool mShowButtons;

    /*!
     * \brief mHideEdit true if the edit button should be hidden, false otherwise.
     */
    bool mHideEdit;

    /// programmatically resize
    void resize();

    /*!
     * \brief mButtonHeight height of a button
     */
    int mButtonHeight;

    /// true to show the states, false otherwise.
    bool mShowStates;
};

#endif // DROPDOWNTOPWIDGET_H
