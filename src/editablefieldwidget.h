#ifndef EDITABLEFIELDWIDGET_H
#define EDITABLEFIELDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The EditableFieldWidget class provides a QLabel with an edit button. If the edit button is clicked,
 *        the label turns into a QLineEdit, with a an accept and cancel button. This allows the user to edit a label
 *        without leaving to busy of a UI available when they aren't editing it.
 */
class EditableFieldWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit EditableFieldWidget(const QString& text, QWidget *parent = 0);

    /// programmatically set the text
    void setText(const QString& text);

    /*!
     * \brief setFontPointSize set the font point size, updating the minimum size
     *        of the widget, if needed.
     * \param pt point size of font
     */
    void setFontPointSize(int pt);

signals:

    /// emitted when Accept is pressed and the text of the widget changes.
    void updatedField(QString);

private slots:

    /// button on the right used for Edit or Accept was clicked.
    void rightButtonClicked(bool);

    /// button the left used for Cancel was clicked.
    void leftButtonClicked(bool);

    /// QLineEdit had its text changed
    void lineEditChanged(QString newText);

private:

    /// true to turn on edit mode, false to turn off edit mode.
    void setInEditMode(bool);

    /// true if editing, false if just displaying text.
    bool mIsEditing;

    /// text stored when line editing starts, in case it is canceled.
    QString mStoredText;

    /// layout
    QHBoxLayout *mLayout;

    /// Label for displaying the editable text.
    QLabel *mText;

    /// Editable version of displaying the text.
    QLineEdit *mEditableField;

    /// displays either an Edit button or an Accept button.
    QPushButton *mRightButton;

    /// hidden sometimes, displays a Cancel button at other times.
    QPushButton *mLeftButton;

    /// cached version of edit icon.
    QIcon mEditIcon;

    /// cached version of check icon.
    QIcon mCheckIcon;
};


#endif // EDITABLEFIELDWIDGET_H
