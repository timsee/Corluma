#ifndef ARDUCORINFOWIDGET_H
#define ARDUCORINFOWIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QWidget>

#include "comm/arducor/arducormetadata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The ArduCorInfoWidget class provides information about an Arducor controller.
 */
class ArduCorInfoWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ArduCorInfoWidget(ArduCorMetadata light, QWidget* parent);

    /*!
     * \brief updateLight update the light used internally to fill the widget
     * \param light new data for the light.
     */
    void updateLight(ArduCorMetadata light);

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

    /// getter for ArduCorMetadata being represented.
    ArduCorMetadata metadata() { return mMetadata; }

    /// true if details are hidden, false otherwise
    bool detailsHidden() { return mHideDetails; }

    /// resize widget programmatically
    void resize();

signals:
    /*!
     * \brief clicked emits the key of the widget whenever it is clicked.
     */
    void clicked(QString);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /// handles when the widget is resized
    void resizeEvent(QResizeEvent*);

private:
    //// set the title's font size
    void setTitleFontPointSize(int pt);

    /// Unique key based on device index of Hue Light.
    QString mKey;

    /// true if checked, false otherwise
    bool mIsChecked;

    /// true if  all info should show, false otherwise.
    bool mHideDetails;

    /// displays the name of arducor light
    QLabel* mName;

    /// displays the path to the light
    QLabel* mPathLabel;

    /// displays the API of arducor light
    QLabel* mAPILabel;

    /// displays the hardware type of the arducor light
    QLabel* mHardwareTypeLabel;

    /// stored data the Light being displayed by this widget.
    ArduCorMetadata mMetadata;

    /// pixmap for the type.
    QPixmap mTypePixmap;

    /// displays the type of light, such as a lightbulb or a light cube.
    QLabel* mTypeIcon;

    /// layout for top of widget
    QHBoxLayout* mTopLayout;

    /// main layout of widget
    QVBoxLayout* mMainLayout;
};

#endif // ARDUCORINFOWIDGET_H
