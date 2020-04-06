#ifndef GROUPBUTTON_H
#define GROUPBUTTON_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

/// group button state
enum class EGroupButtonState { selectAll, clearAll, disabled };

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

    /// set whether the button should show select all or deselect all
    void setSelectAll(bool shoudlSelect);

signals:

    /// emits when the group button is pressed
    void groupButtonPressed(QString key);

    /// emits when the select all butotn is toggled to either select all or deselect all
    void groupSelectAllToggled(QString, bool);

protected:
    /// resize the widget
    virtual void resizeEvent(QResizeEvent*);

    /*!
     * \brief mouseReleaseEvent called when a mouse press is released Events not
     *        directly on top of the color wheel are ignored.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /// renders the widget
    virtual void paintEvent(QPaintEvent*);

private slots:

    /// picks up when the select all button is pressed
    void buttonPressed(bool);

private:
    /// resizes widget programmatically
    void resize();

    /*!
     * \brief computeHighlightColor compute the color of the highlight of the widget. The highlight
     * is based on the number of devices and the number that are currently selected. If all
     * devices are currently selected, it highlights completely.
     *
     * \param checkedDeviceCount the number of devices that are checked
     * \param reachableDeviceCount the total number of reachable devices
     * \return the color to use for the highlight.
     */
    QColor computeHighlightColor(std::uint32_t checkedDeviceCount, uint32_t reachableDeviceCount);

    /*!
     * \brief resizeRightHandIcon resize the right hand icon to match the widgets size
     * \param pixmap the pixmap to resize
     * \param button the button to apply the resized pixmap to
     */
    void resizeRightHandIcon(QPixmap pixmap, QPushButton* button);

    /// preferred size of icon
    QSize iconSize();

    /// getter for current pixmap
    const QPixmap& currentPixmap();

    /// state of the button
    EGroupButtonState mButtonState;

    /// true if widget is selected, false otherwise
    bool mIsSelected;

    /// count of reachable devices
    std::uint32_t mReachableCount;

    /// count of checked devices
    std::uint32_t mCheckedCount;

    /// label for checkbox
    QLabel* mTitle;

    /*!
     * \brief mButton button that selects all devices when pushed and adds them to the data layer.
     */
    QLabel* mButton;

    /// pixmap for the select all button
    QPixmap mSelectAllPixmap;

    /// pixmap for the clear all button
    QPixmap mClearAllPixmap;

    /// pixmap for disabled button
    QPixmap mDisabledPixmap;
};

} // namespace cor

#endif // GROUPBUTTON_H
