#ifndef LIGHTINFOSCROLLAREA_H
#define LIGHTINFOSCROLLAREA_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "comm/commhue.h"
#include "comm/hue/hueinfowidget.h"
#include "cor/objects/page.h"
#include "cor/widgets/topwidget.h"

/*!
 * \brief The LightInfoScrollArea class contains the scroll area for the LightInfoListWidget.
 */
class LightInfoScrollArea : public QScrollArea {
    Q_OBJECT
public:
    /// constructor
    explicit LightInfoScrollArea(QWidget* parent);

    /*!
     * \brief updateHues update the lights displayed in the widget, normally called
     *        right before displaying the widget.
     *
     * \param lights list of lights to load into the HusLightInfoListWidget
     */
    void updateHues(std::vector<HueMetadata> lights);

    /// getter for the hue widgets
    std::vector<hue::HueInfoWidget*> hueWidgets() { return mHueWidgets; }

    /*!
     * \brief lookupCurrentLight looks up the current light based on what is currently selected
     * \return returns EProtocol::MAX if nothing is selected, otherwise, returns the current light
     */
    QString lookupCurrentLight();

    /// getter for current key
    QString key() const noexcept { return mLastHueKey; }

    /// resize programmatically
    void resize();

signals:

    /// emits the key and whether or not a light is selected when a light is clicked
    void lightClicked(const QString&, bool);

private slots:

    /// handles when a light is clicked
    void clickedLight(const QString&);

private:
    /// widget used for scroll area.
    QWidget* mScrollAreaWidget;

    /// layout for scroll area
    QVBoxLayout* mScrollLayout;

    /// widgets for hue displayed in scroll area
    std::vector<hue::HueInfoWidget*> mHueWidgets;

    /// stores the last hue key clicked
    QString mLastHueKey;
};

#endif // LIGHTINFOSCROLLAREA_H
