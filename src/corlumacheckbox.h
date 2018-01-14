#ifndef CORLUMACHECKBOX_H
#define CORLUMACHECKBOX_H

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CorlumaCheckBox class is a simple widget designed to give a checkbox
 *        with a label.
 */
class CorlumaCheckBox : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit CorlumaCheckBox(QWidget *parent = 0, QString title = QString());

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

#endif // CORLUMACHECKBOX_H
