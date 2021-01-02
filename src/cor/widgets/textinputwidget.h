#ifndef TEXTINPUTWIDGET_H
#define TEXTINPUTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/page.h"
#include "utils/qt.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License
 *
 *
 * \brief The TextInputWidget class is a replacement for QInputDialog that is more consistent across
 * multiple environments. When the "OK" button is clicked, the text in the line edit is emitted.
 * This text can then be handled by another class.
 *
 */
class TextInputWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    explicit TextInputWidget(QWidget* parent)
        : QWidget(parent),
          mButtonOK(new QPushButton("OK", this)),
          mButtonCancel(new QPushButton("X", this)),
          mInputPromptLabel(new QLabel(this)),
          mLineEdit(new QLineEdit(this)) {
        mInputPromptLabel->setWordWrap(true);
        connect(mButtonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
        connect(mButtonCancel, SIGNAL(clicked()), this, SLOT(clickedCancel()));
    }

    /// pushes widget in
    void pushIn(const QString& inputPrompt, const QString& defaultInput) {
        mInputPromptLabel->setText(inputPrompt);
        mLineEdit->setText(defaultInput);

        auto size = cor::applicationSize();
        float sizeRatio = size.width() / float(size.height());
        if (sizeRatio > 1.0f) {
            moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.333f), int(-1 * parentWidget()->height())),
                QPoint(int(parentWidget()->width() * 0.333f),
                       int(parentWidget()->height() * 0.333f)));
        } else {
            moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
                QPoint(int(parentWidget()->width() * 0.125f),
                       int(parentWidget()->height() * 0.25f)));
        }

        setVisible(true);
        isOpen(true);
        raise();
    }

    /// pushes widget out
    void pushOut() {
        auto size = cor::applicationSize();
        float sizeRatio = size.width() / float(size.height());
        if (sizeRatio > 1.0f) {
            moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.333f),
                       int(parentWidget()->height() * 0.333f)),
                QPoint(int(parentWidget()->width() * 0.333f), int(-1 * parentWidget()->height())));
        } else {
            moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.125f),
                       int(parentWidget()->height() * 0.25f)),
                QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
        }
        setVisible(false);
        isOpen(false);
    }

    /// resizes widget programmatically
    void resize() {
        QSize size = parentWidget()->size();
        auto appSize = cor::applicationSize();
        float sizeRatio = appSize.width() / float(appSize.height());
        if (sizeRatio > 1.0f) {
            setFixedSize(int(size.width() * 0.333f), int(size.height() * 0.333f));
        } else {
            setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.2f));
        }

        int yPos = 0;
        mInputPromptLabel->setGeometry(0, yPos, width(), height() / 2);
        yPos += mInputPromptLabel->height();

        int xPos = 0;
        mLineEdit->setGeometry(xPos, yPos, width() * 2 / 3, height() / 2);
        xPos += mLineEdit->width();
        mButtonOK->setGeometry(xPos, yPos, width() / 6, height() / 2);
        xPos += mButtonOK->width();
        mButtonCancel->setGeometry(xPos, yPos, width() / 6, height() / 2);
    }

signals:

    /// emitted when "OK" is clicked
    void textAdded(QString);

    /// emitted when "X" is clicked
    void cancelClicked();

private slots:

    /// handles when OK is clicked
    void clickedOK() { emit textAdded(mLineEdit->text()); }

    /// handles when cancel is clicked
    void clickedCancel() { emit cancelClicked(); }

protected:
    /// detects when widget is resized
    virtual void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// button that says "OK" and accepts the text
    QPushButton* mButtonOK;

    /// button with an "X" that closes the window
    QPushButton* mButtonCancel;

    /// text about what to input.
    QLabel* mInputPromptLabel;

    /// line edit for entering text.
    QLineEdit* mLineEdit;
};

} // namespace cor

#endif // TEXTINPUTWIDGET_H
