#ifndef SPEEDWIDGET_H
#define SPEEDWIDGET_H


#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/page.h"
#include "routines/speedslider.h"
#include "utils/qt.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SpeedWidget class is a widget that displays a speed slider overlaid on the app.
 */
class SpeedWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit SpeedWidget(QWidget* parent)
        : QWidget(parent),
          mProtocol{EProtocolType::arduCor},
          mButtonOK(new QPushButton("Save", this)),
          mButtonCancel(new QPushButton("Cancel", this)),
          mInputPromptLabel(new QLabel(this)),
          mSpeedSlider(new SpeedSlider(this)) {
        mInputPromptLabel->setWordWrap(true);
        mInputPromptLabel->setText("Change the routine speed");
        connect(mButtonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
        connect(mButtonCancel, SIGNAL(clicked()), this, SLOT(clickedCancel()));
    }

    /// pushes widget in
    void pushIn(EProtocolType type, int defaultInput) {
        mProtocol = type;
        mSpeedSlider->updateSpeed(defaultInput);

        auto size = cor::applicationSize();
        float sizeRatio = size.width() / float(size.height());
        if (sizeRatio > 1.0f) {
            cor::moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.25f), int(-1 * parentWidget()->height())),
                QPoint(int(parentWidget()->width() * 0.25f),
                       int(parentWidget()->height() * 0.25f)));
        } else {
            cor::moveWidget(
                this,
                QPoint(int(-parentWidget()->width() * 0.1f), int(-1 * parentWidget()->height())),
                QPoint(int(parentWidget()->width() * 0.1f), int(parentWidget()->height() * 0.1f)));
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
            cor::moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.25f), int(parentWidget()->height() * 0.25f)),
                QPoint(int(parentWidget()->width() * 0.25f), int(-1 * parentWidget()->height())));
        } else {
            cor::moveWidget(
                this,
                QPoint(int(parentWidget()->width() * 0.1f), int(parentWidget()->height() * 0.1f)),
                QPoint(int(parentWidget()->width() * 0.1f), int(-1 * parentWidget()->height())));
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
            setFixedSize(int(size.width() * 0.5f), int(size.height() * 0.5f));
        } else {
            setFixedSize(int(size.width() * 0.8f), int(size.height() * 0.4f));
        }

        auto rowHeight = height() / 6;

        int yPos = 0;
        mInputPromptLabel->setGeometry(0, yPos, width(), rowHeight);
        yPos += mInputPromptLabel->height();

        mSpeedSlider->setGeometry(0, yPos, width(), rowHeight * 4);
        yPos += mSpeedSlider->height();


        mButtonCancel->setGeometry(0, yPos, width() / 2, rowHeight);
        mButtonOK->setGeometry(mButtonCancel->width(), yPos, width() / 2, rowHeight);
    }

signals:

    /// emitted when "OK" is clicked
    void speedChanged(int);

    /// emitted when "X" is clicked
    void cancelClicked();

private slots:

    /// handles when OK is clicked
    void clickedOK() { emit speedChanged(mSpeedSlider->value()); }

    /// handles when cancel is clicked
    void clickedCancel() { emit cancelClicked(); }

protected:
    /// detects when widget is resized
    virtual void resizeEvent(QResizeEvent*) { resize(); }

private:
    /// the protocol being displayed by the widget
    EProtocolType mProtocol;

    /// button that says "OK" and accepts the text
    QPushButton* mButtonOK;

    /// button with an "X" that closes the window
    QPushButton* mButtonCancel;

    /// text about what to input.
    QLabel* mInputPromptLabel;

    /// the slider for determining the speed
    SpeedSlider* mSpeedSlider;
};


#endif // SPEEDWIDGET_H
