#ifndef LIGHTCHECKBOX_H
#define LIGHTCHECKBOX_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QPixmap>

/*!
 * \brief The LightCheckBox class is a push button with a customizable checkbox which allows for
 *        most customization than the standard Qt radio button.
 */
class LightCheckBox : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief LightCheckBox constructor
     */
    explicit LightCheckBox(QWidget *parent = 0);

    /*!
     * \brief setText set the text at the top of the check box asset.
     * \param text text to draw on top of check box.
     */
    void setText(QString text);

    /*!
     * \brief setChecked checks and unchecks the checkbox. Emits a signal defining its state to
     *        its parent.
     * \param checked true if checking the box, false if unchecking the box.
     */
    void setChecked(bool checked);

signals:
    /*!
     * \brief clicked emits whenever the checkbox is clicked
     * \param bool true if checked, false otherwise.
     */
    void clicked(bool);

private slots:
    /*!
     * \brief handleButton handles the click event from the mButton.
     */
    void handleButton();

protected:
    /*!
     * \brief resizeEvent called whenever the checkbox resizes so that
     *        it resizes its assets.
     */
    void resizeEvent(QResizeEvent *event);

private:
    /*!
     * \brief mChecked true if the box is checked, false otherwise.
     */
    bool mChecked;

    /*!
     * \brief mCheckedIcon icon for when the check box is in its checked state.
     */
    QIcon mCheckedIcon;
    /*!
     * \brief mUncheckedIcon icon for when the check box is in its unchcked state.
     */
    QIcon mUncheckedIcon;

    /*!
     * \brief mButton button that the user interacts with which displays the checkbox
     */
    QPushButton *mButton;

    /*!
     * \brief mLabel name for the checkbox.
     */
    QLabel *mLabel;

    /*!
     * \brief mLayout layout that combines the PushButton and Label
     */
    QVBoxLayout *mLayout;
};

#endif // LIGHTCHECKBOX_H
