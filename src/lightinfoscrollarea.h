#ifndef LIGHTINFOSCROLLAREA_H
#define LIGHTINFOSCROLLAREA_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "comm/arducor/arducorinfowidget.h"
#include "comm/commhue.h"
#include "comm/hue/hueinfowidget.h"
#include "comm/nanoleaf/leaflightinfowidget.h"
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
    void updateHues(std::vector<HueLight> lights);

    /*!
     * \brief updateControllers update the controllers for nanoleafs to any
     * hardware changes detected
     *
     * \param controllers nanoleaf controllers to use as the recent set.
     */
    void updateNanoLeafs(const std::vector<nano::LeafLight>& lights);

    /// updates the arducor lights in the light info list widget
    void updateAruCorLights(const std::vector<ArduCorLight>& lights);

    /*!
     * \brief lookupCurrentLight looks up the current light based on what is currently selected
     * \return returns EProtocol::MAX if nothing is selected, otherwise, returns the current light
     */
    std::pair<EProtocolType, QString> lookupCurrentLight();

    /// getter for current key
    QString key() const noexcept {
        switch (mCurrentProtocol) {
            case EProtocolType::hue:
                return mLastHueKey;
            case EProtocolType::arduCor:
                return mLastArduCorKey;
            case EProtocolType::nanoleaf:
                return mLastNanoleafKey;
            case EProtocolType::MAX:
                return {};
        }
        return {};
    }

    /// resize programmatically
    void resize();

    /// change the protocol currently displayed
    void changeProtocol(EProtocolType);
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

    /// widgets for nanoleaf displayed in scroll area
    std::vector<nano::LeafLightInfoWidget*> mNanoleafWidgets;

    /// widgets for ArduCor displayed in scroll area
    std::vector<ArduCorInfoWidget*> mArduCorWidgets;

    /// stores the last hue key clicked
    QString mLastHueKey;

    /// stores the last arducor key clicked
    QString mLastArduCorKey;

    /// stores the last nanoleaf key clicked
    QString mLastNanoleafKey;

    /// stores the current protocol shown
    EProtocolType mCurrentProtocol;
};

#endif // LIGHTINFOSCROLLAREA_H
