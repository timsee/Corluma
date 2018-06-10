#ifndef COR_TOP_WIDGET_H
#define COR_TOP_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TopWidget class is a standardized way to put a title and a button as the header
 *        of a widget. It is used on widgets like the SettingsPage or the CorlumaWebView.
 */
class TopWidget : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit TopWidget(QString title, QString resource, QWidget *parent);

    /// set the font point size.
    void setFontPoint(int pt);

signals:
    /// emitted whenever its button is clicked
    void clicked(bool);

protected:
    /// handles when the widget resizes
    void resizeEvent(QResizeEvent *);

private slots:
    /// handles when the button is pressed internally
    void buttonPressed(bool);

private:
    /// resource for the button's graphic
    QString mResource;

    /// button placed at left hand side of widget
    QPushButton *mButton;

    /// title of widget
    QLabel *mTitle;

    /// layout for widget
    QHBoxLayout *mLayout;
};

}
#endif // COR_TOP_WIDGET_H
