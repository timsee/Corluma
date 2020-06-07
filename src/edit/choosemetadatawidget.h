#ifndef EDITNAMEANDDESCRIPTIONWIDGET_H
#define EDITNAMEANDDESCRIPTIONWIDGET_H

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>
#include "edit/editpagechildwidget.h"

namespace {

constexpr int kNameMaxSize = 25;

constexpr int kDescriptionMaxSize = 200;

} // namespace

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EditNameAndDescriptionWidget class allows the user to set the name and the description
 * of a group, room, or mood. It provides two fields: a name field, which is required, and a
 * description field, which is optional. The widget checks that the name and description are both
 * shorter than a maximum length, and that the name isn't used by any reserved strings for the name
 * such as "Miscellaneous".
 */
class ChooseMetadataWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ChooseMetadataWidget(QWidget* parent)
        : EditPageChildWidget(parent),
          mNameLabel{new QLabel("Name:", this)},
          mNameInput{new QLineEdit(this)},
          mDescriptionLabel{new QLabel("Description (Optional):", this)},
          mDescriptionInput{new QTextEdit(this)} {
        mBottomButtons->hideBack(true);
        mBottomButtons->enableForward(false);

        mNameInput->setMaxLength(kNameMaxSize);
        connect(mNameInput, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

        connect(mDescriptionInput, SIGNAL(textChanged()), this, SLOT(descriptionChanged()));
    }

    /// getter for the name for set by the page
    QString name() { return mNameInput->text(); }

    /// getter for the description set by the page.
    QString description() { return mDescriptionInput->toPlainText(); }

    /// prefill the metadata with existing data to edit
    void prefill(const QString& name, const QString& description) {
        mNameInput->setText(name);
        mDescriptionInput->setText(description);
        conditionsMet();
    }

    /// clears all data currently on the page.
    void clear() {
        mNameInput->setText("");
        mDescriptionInput->setText("");
        mBottomButtons->enableForward(false);
    }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) {
        int yPos = 0;
        int heightSpacer = this->height() / 10;

        mNameLabel->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mNameLabel->height();

        mNameInput->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mNameInput->height();

        mDescriptionLabel->setGeometry(0, yPos, this->width(), heightSpacer);
        yPos += mDescriptionLabel->height();

        mDescriptionInput->setGeometry(0, yPos, this->width(), heightSpacer * 4);
        yPos += mDescriptionInput->height();

        // add a spacer
        yPos += heightSpacer * 2;

        mBottomButtons->setGeometry(0, yPos, this->width(), heightSpacer);
    }

private slots:

    /// QLineEdit had its text changed, check if its a valid name or not.
    void lineEditChanged(QString) {
        if (conditionsMet()) {
            mBottomButtons->enableForward(true);
        } else {
            mBottomButtons->enableForward(false);
        }
    }


    /// the description text changed, verify that its less than the maximum length
    void descriptionChanged() {
        QString text = mDescriptionInput->toPlainText();
        if (text.length() > kDescriptionMaxSize) {
            // get count of number of characters over
            auto diff = text.length() - kDescriptionMaxSize;
            text.chop(diff);
            mDescriptionInput->setText(text);
            QTextCursor cursor(mDescriptionInput->textCursor());
            cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            mDescriptionInput->setTextCursor(cursor);
        }
    }

private:
    /// true if all fields have met the conditions needed to proceed, false if any widget has an
    /// invalid input.
    bool conditionsMet() {
        if (mNameInput->text().size() > 2) {
            auto name = mNameInput->text();
            if ((name != "zzzzMiscellaneous") && (name != "Miscellaneous")) {
                emit stateChanged(mIndex, EEditProgressState::completed);
                return true;
            }
        }
        emit stateChanged(mIndex, EEditProgressState::incomplete);
        return false;
    }

    /// label for the name input
    QLabel* mNameLabel;

    /// line edit for the name
    QLineEdit* mNameInput;

    /// label for the description
    QLabel* mDescriptionLabel;

    /// text field for the description
    QTextEdit* mDescriptionInput;
};

#endif // EDITNAMEANDDESCRIPTIONWIDGET_H
