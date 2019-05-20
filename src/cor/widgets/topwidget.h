#ifndef COR_TOP_WIDGET_H
#define COR_TOP_WIDGET_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TopWidget class is a standardized way to put a title and a button as the header
 *        of a widget. It is used on widgets like the SettingsPage or the CorlumaWebView.
 */
class TopWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit TopWidget(const QString& title, const QString& resource, QWidget* parent)
        : QWidget(parent) {
        mLayout = new QHBoxLayout();

        mButton = new QPushButton(this);
        mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));

        mTitle = new QLabel(title, this);
        mTitle->setStyleSheet("font-size:30pt;");
        mTitle->setAlignment(Qt::AlignBottom);
        mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        mResource = resource;

        mLayout->addWidget(mTitle, 10);
        mLayout->addWidget(mButton, 1);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(6);
        this->setLayout(mLayout);
    }


    /// set the font point size.
    void setFontPoint(int pt) {
        if (pt <= 0) {
            pt = 1;
        }
        QString stylesheet = "font-size:" + QString::number(pt) + "pt;";
        mTitle->setStyleSheet(stylesheet);
    }


signals:
    /// emitted whenever its button is clicked
    void clicked(bool);

protected:
    /// handles when the widget resizes
    void resizeEvent(QResizeEvent*) {
        mButton->setFixedWidth(mButton->height());
        QPixmap pixmap(mResource);
        int min = std::min(mButton->width(), mButton->height());
        int finalSize = int(min * 0.5);
        mButton->setIconSize(QSize(finalSize, finalSize));
        mButton->setIcon(QIcon(
            pixmap.scaled(finalSize, finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }

private slots:
    /// handles when the button is pressed internally
    void buttonPressed(bool pressed) { emit clicked(pressed); }

private:
    /// resource for the button's graphic
    QString mResource;

    /// button placed at left hand side of widget
    QPushButton* mButton;

    /// title of widget
    QLabel* mTitle;

    /// layout for widget
    QHBoxLayout* mLayout;
};

} // namespace cor
#endif // COR_TOP_WIDGET_H
