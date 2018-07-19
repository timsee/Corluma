#ifndef ARDUCORINFOWIDGET_H
#define ARDUCORINFOWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>

#include "cor/light.h"

class ArduCorInfoWidget : public QWidget
{
    Q_OBJECT
public:

    /// constructor
    explicit ArduCorInfoWidget(cor::Light light, QWidget *parent);

    /*!
     * \brief updateLight update the light used internally to fill the widget
     * \param light new data for the light.
     */
    void updateLight(cor::Light light);

    /*!
     * \brief hideDetails true to show only the basic details of a widget, false
     *        to show all of the details.
     * \param shouldHide true to show only the basic details of a widget, false
     *        to show all of the details.
     */
    void hideDetails(bool shouldHide);

    /*!
     * \brief setChecked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     * \param checked true to highlight widget and treat as checked, false to keep in regular
     *        state.
     */
    void setChecked(bool checked);

    /// true if checked, false otherwise.
    bool checked() { return mIsChecked; }

    /// getter for the key of the widget.
    const QString& key() { return mKey; }

    /// getter for cor::Light being represented.
    cor::Light light() { return mLight; }

    /// true if details are hidden, false otherwise
    bool detailsHidden() { return mHideDetails; }

    /// set height of info widget
    void setHeight(int);

signals:
    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent *);

private:

    void setTitleFontPointSize(int pt);

    /// Unique key based on device index of Hue Light.
    QString mKey;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// true if  all info should show, false otherwise.
    bool mHideDetails;

    /// main layout
    QGridLayout *mLayout;

    /// displays the name of arducor light
    QLabel *mName;

    /// displays the path to the light
    QLabel *mPathLabel;

    /// displays the commtype of the arducor
    QLabel *mCommTypeLabel;

    /// displays the API of arducor light
    QLabel *mAPILabel;

    /// displays the hardware type of the arducor light
    QLabel *mHardwareTypeLabel;

    /// stored data the Light being displayed by this widget.
    cor::Light mLight;

    /// used for setting the height;
    int mHeight;
};

#endif // ARDUCORINFOWIDGET_H
