#ifndef LISTGROUPWIDGET_H
#define LISTGROUPWIDGET_H

#include <QWidget>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "icondata.h"
#include "comm/commtype.h"
#include "cor/mood.h"
#include "cor/lightvectorwidget.h"
#include "cor/listitemwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListMoodPreviewWidget class is used to display a pre-saved group of
 *        devices and their settings in a single list widget. It displays the name
 *        of the group and the states of some of its lights.
 */
class ListMoodPreviewWidget : public cor::ListItemWidget
{
    Q_OBJECT

public:
    /*!
     * \brief ListMoodWidget constructor
     * \param group group of lights in mood
     */
    explicit ListMoodPreviewWidget(const cor::Mood& group,
                                   QWidget *parent);

    /*!
    * \brief setChecked set the widget as checked or unchecked. When it is checked it
    *        it will be a light blue color. When it isn't, it will be dark grey.
    * \param checked true to set to checked, false othwerise
    * \return true if successful, false otherwise
    */
    bool setChecked(bool checked);

    /// setter for selected state
    bool setSelected(bool selected);

    /// getter for selected state
    bool selected() const noexcept { return mIsSelected; }

    /*!
     * \brief checked getter for checked state
     * \return true if checked, false otherwise
     */
    bool checked() const noexcept { return mIsChecked; }

    /*!
     * \brief moodName getter for name
     * \return name of mood
     */
    const QString& moodName() { return mMood.name(); }

signals:

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editClicked(std::uint64_t);

    /// called when mood is selected
    void moodSelected(std::uint64_t);

protected:

    /*!
     * \brief enterEvent picks up when the mouse pointer (or finger on mobile) enters the area of the widget.
     */
    virtual void enterEvent(QEvent *);

    /*!
     * \brief leaveEvent picks up when the mouse pointer (or finger on mobile) leaves the area of the widget.
     */
    virtual void leaveEvent(QEvent *);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent *event);

    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent *);

private slots:

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editButtonClicked(bool) { emit editClicked(mMood.uniqueID());  }

private:

    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

    /// true if selected, false otherwise
    bool mIsSelected;

    /*!
     * \brief mEditButton button used to edit the collection. Editing can change
     *        the name or the lights contained in the collection.
     */
    QPushButton *mEditButton;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// icon for whether all lights are connected or not
    QLabel *mAllLightsConnectedIcon;

    /*!
     * \brief mName namme of group
     */
    QLabel *mName;

    /// palette showing the colors
    cor::LightVectorWidget *mPalette;

    /// stored local copy of the group data.
    cor::Mood mMood;

    /*!
     * \brief mLayout layout for widget
     */
    QHBoxLayout *mTopLayout;

    /*!
     * \brief mFullLayout combines the various layouts into a single layout that takes up the whole widget.
     */
    QVBoxLayout *mFullLayout;
};

#endif // LISTGROUPWIDGET_H
