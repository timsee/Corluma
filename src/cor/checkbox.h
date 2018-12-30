#ifndef COR_CHECK_BOX_H
#define COR_CHECK_BOX_H

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CheckBox class is a simple widget designed to give a checkbox
 *        with a label.
 */
class CheckBox : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit CheckBox(QWidget *parent, QString title);

    /// checks and unchecks the checkbox
    void setChecked(bool shouldCheck);

    /// set the title for the corluma checkbox
    void setTitle(QString title);

    /*!
     * \brief downsizeTextWidthToFit downsize the font's point size until this entire widget
     *        fits into the width provided. If downsizing is not needed, the system's font
     *        size is used instead.
     *
     * NOTE: this is hacky and inefficient!
     * \param maxWidth max width for the entire widget.
     */
    void downsizeTextWidthToFit(int maxWidth);

    /// getter for whether a box is checked or not.
    bool checked() { return mIsChecked; }

signals:

    /// sent out whenever the checkbox is checked or unchecked
    void boxChecked(bool);

private slots:

    /// handles when the checkbox button is clicked
    void buttonPressed(bool);

protected:

    /// resize the widget
    virtual void resizeEvent(QResizeEvent *);

private:
    /// true if checked, false if not checked
    bool mIsChecked;
    
    /// spacer between checkbox and title
    int mSpacer;

    /// label for checkbox
    QLabel *mTitle;

    /// button that displays checked and unchecked states
    QPushButton *mCheckBox;
};

}
#endif // COR_CHECK_BOX_H
