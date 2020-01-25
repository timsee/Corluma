#ifndef LISTGROUPWIDGET_H
#define LISTGROUPWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "comm/commtype.h"
#include "cor/objects/mood.h"
#include "cor/widgets/lightvectorwidget.h"
#include "cor/widgets/listitemwidget.h"
#include "icondata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ListMoodPreviewWidget class is used to display a pre-saved group of
 * devices and their settings in a single list widget. It displays the name
 * of the group and the states of some of its lights.
 */
class ListMoodPreviewWidget : public cor::ListItemWidget {
    Q_OBJECT

public:
    /*!
     * \brief ListMoodWidget constructor
     *
     * \param group group of lights in mood
     */
    explicit ListMoodPreviewWidget(const cor::Mood& group, QWidget* parent);

    /*!
     * \brief setChecked set the widget as checked or unchecked. When it is checked it
     * it will be a light blue color. When it isn't, it will be dark grey.
     *
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
     *
     * \return true if checked, false otherwise
     */
    bool checked() const noexcept { return mIsChecked; }

    /*!
     * \brief moodName getter for name
     *
     * \return name of mood
     */
    const QString& moodName() { return mMood.name(); }

    /// resizes programmatically
    void resize();

signals:

    /// called when mood is selected
    void moodSelected(std::uint64_t);

protected:
    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /*!
     * \brief paintEvent paints the background of the widget
     */
    void paintEvent(QPaintEvent* event);

    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent*);

private:
    /*!
     * \brief mIsChecked true if checked, false otherwise
     */
    bool mIsChecked;

    /// true if selected, false otherwise
    bool mIsSelected;

    /*!
     * \brief mName namme of group
     */
    QLabel* mName;

    /// palette showing the colors
    cor::LightVectorWidget* mPalette;

    /// stored local copy of the group data.
    cor::Mood mMood;

    /*!
     * \brief mLayout layout for widget
     */
    QHBoxLayout* mTopLayout;

    /*!
     * \brief mFullLayout combines the various layouts into a single layout that takes up the whole
     * widget.
     */
    QVBoxLayout* mFullLayout;
};

#endif // LISTGROUPWIDGET_H
