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
    explicit CorlumaCheckBox(QString title = QString(), QWidget *parent = 0);

    /// checks and unchecks the checkbox
    void setChecked(bool shouldCheck);

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
