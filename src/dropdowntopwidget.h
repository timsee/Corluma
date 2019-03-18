#ifndef DROPDOWNTOPWIDGET_H
#define DROPDOWNTOPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "cor/protocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DropdownTopWidget class provides a widget that can be used at the top of a dropdown list. It is frequently
 *        used in the LightPage and the MoodPage to hide the details of overally lights/moods but still display some meta information.
 *        It also provides the user with buttons to edit the contents of the widget, or to select all/none of the contents in some cases.
 */
class DropdownTopWidget : public QWidget
{
    Q_OBJECT
public:

    /// constuctor
    explicit DropdownTopWidget(const QString& key, cor::EWidgetType type, bool hideEdit, QWidget *parent);

    /*!
     * \brief showButtons getter that checks if buttons are showing
     * \return  true if buttons are showing, false otherwise.
     */
    bool showButtons() { return mShowButtons; }

    /// set to true to show all widgets, false to just show the dropdown widget.
    void showButtons(bool showButtons);

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

private slots:

    /*!
    * \brief editButtonClicked called when edit button is pressed, sends out an edit signal
    */
   void editButtonClicked(bool) { emit editClicked(mKey);  }

protected:

   /*!
    * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
    */
   virtual void mouseReleaseEvent(QMouseEvent *);

private:

    /// the type of dropdowntopwidget
    cor::EWidgetType mType;

    /*!
     * \brief mLayout layout for the widget
     */
    QHBoxLayout *mLayout;

    /*!
     * \brief mName label for name of collection
     */
    QLabel *mName;

    /*!
     * \brief mEditButton button used to edit the collection. Editing can change
     *        the name or the lights contained in the collection.
     */
    QPushButton *mEditButton;

    /*!
     * \brief mHiddenStateIcon an arrow in the top right of the widget. If its pointing right, no
     *        buttons are shown. If its pointing down, all buttons are shown.
     */
    QLabel *mHiddenStateIcon;

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

    /// helper for computing icon size
    QSize iconSize() { return QSize(int(mMinimumHeight * mIconRatio), int(mMinimumHeight * mIconRatio)); }

    /*!
     * \brief resizeRightHandIcon resize the right hand icon to match the widgets size
     * \param pixmap the pixmap to resize
     * \param button the button to apply the resized pixmap to
     */
    void resizeRightHandIcon(QPixmap pixmap, QPushButton *button);

    /*!
     * \brief mMinimumHeight minimum size allowed for collection.
     */
    int mMinimumHeight;

    /*!
     * \brief mIconRatio
     */
    float mIconRatio;
};

#endif // DROPDOWNTOPWIDGET_H
