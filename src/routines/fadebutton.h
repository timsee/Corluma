#ifndef COR_ROUTINES_FADEBUTTON_H
#define COR_ROUTINES_FADEBUTTON_H

#include <QLabel>
#include <QPushButton>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The FadeButton is a simple combination of a QPushButton and a QLabel. It adds a label to
 * the bottom portion of the button and adds an image to the main button.
 */
class FadeButton : public QWidget {
    Q_OBJECT
public:
    explicit FadeButton(QWidget* parent, QString label, QString iconPath)
        : QWidget(parent),
          mIconPath{iconPath},
          mLabel{new QLabel(label, this)},
          mButton{new QPushButton(this)} {
        mLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        mLabel->setStyleSheet("background-color:rgb(33,32,32);");


        mButton->setCheckable(true);
        connect(mButton, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
    }

    /// true to check the button, false otherwsie
    void setChecked(bool checked) { mButton->setChecked(checked); }

signals:

    /// emits the name of the button when pressed.
    void pressed(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

private slots:

    void buttonPressed(bool) { emit pressed(mLabel->text()); }

private:
    /// handle when it resizes
    void resize() {
        auto rowHeight = height() / 4;
        mButton->setGeometry(0, 0, width(), rowHeight * 3);
        mLabel->setGeometry(0, mButton->height(), width(), rowHeight);

        auto buttonSide = std::min(mButton->width(), mButton->height());
        addIconToButton(mIconPath, buttonSide);
    }

    /// adds icon to label
    void addIconToButton(const QString& path, int buttonSide) {
        auto pixmap = QPixmap(path);
        mButton->setIconSize(QSize(buttonSide, buttonSide));
        mButton->setIcon(QIcon(
            pixmap.scaled(buttonSide, buttonSide, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }

    /// path to the icon resource
    QString mIconPath;

    /// label for the sldier
    QLabel* mLabel;

    /// image to the left of the slider
    QPushButton* mButton;
};


} // namespace cor
#endif // COR_ROUTINES_FADEBUTTON_H
