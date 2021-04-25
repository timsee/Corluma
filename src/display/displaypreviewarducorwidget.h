#ifndef DISPLAYPREVIEWARDUCORWIDGET_H
#define DISPLAYPREVIEWARDUCORWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/arducor/controller.h"
#include "cor/lightlist.h"
#include "cor/stylesheets.h"
#include "cor/widgets/lightvectorwidget.h"
#include "cor/widgets/listitemwidget.h"
#include "syncwidget.h"
#include "utils/qt.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayPreviewArduCorWidget class displays a preview of an ArduCor Controller on the
 * Discovery Page.
 */
class DisplayPreviewArduCorWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    /// constructor
    explicit DisplayPreviewArduCorWidget(const cor::Controller& controller,
                                         cor::EArduCorStatus status,
                                         cor::LightList* selectedLights,
                                         QWidget* parent)
        : cor::ListItemWidget(controller.name(), parent),
          mName{new QLabel(cor::controllerToGenericName(controller, status), this)},
          mSelectedLights{selectedLights},
          mSyncWidget{new SyncWidget(this)},
          mLightVector{nullptr},
          mController{controller},
          mStatusType{status} {
        mName->setStyleSheet(cor::kTransparentStylesheet);
        mSyncWidget->setStyleSheet(cor::kTransparentStylesheet);
        createIcons(controller, status);
    }

    /// getter for controller
    const cor::Controller& controller() { return mController; }

    /// getter for the status
    cor::EArduCorStatus status() { return mStatusType; }

    /// update the controller
    void updateController(const cor::Controller& controller,
                          const std::vector<cor::Light>& lights,
                          cor::EArduCorStatus status) {
        mLights = lights;
        mController = controller;
        mStatusType = status;
        mName->setText(cor::controllerToGenericName(controller, status));
        createIcons(controller, status);
        updateLights(lights);

        highlightLights();
    }

    /// highlight lights
    void highlightLights() {
        mReachableCount = 0u;
        mSelectedCount = 0u;
        for (auto light : mLights) {
            if (light.isReachable()) {
                mReachableCount++;
            }
            if (mSelectedLights->doesLightExist(light.uniqueID())) {
                mSelectedCount++;
            }
        }
        update();
    }

    /// programmatically resize.
    void resize() {
        auto labelHeight = this->height() / 3;
        auto yPos = 0u;

        if (mStatusType == cor::EArduCorStatus::searching) {
            auto iconHeight = this->height() / 2;
            mSyncWidget->setGeometry(0, yPos, iconHeight, iconHeight);
            yPos += mSyncWidget->height();
            if (mLightVector != nullptr) {
                mLightVector->setVisible(false);
            }
        } else {
            if (mLightVector != nullptr) {
                mLightVector->setVisible(true);
                if (!mIcons.empty()) {
                    if (mIcons.size() == 1) {
                        auto iconHeight = this->height() / 2;
                        createHardwareIcon(0, iconHeight, yPos);
                        mLightVector->setGeometry(iconHeight * 1.1, yPos, iconHeight, iconHeight);
                    } else {
                        auto iconHeight = this->height() / 3;
                        for (auto i = 0u; i < mIcons.size(); ++i) {
                            createHardwareIcon(i, iconHeight, yPos);
                        }
                        mLightVector->setGeometry(0u,
                                                  yPos + iconHeight,
                                                  (iconHeight * 1.1) * mIcons.size(),
                                                  iconHeight);
                        yPos += mLightVector->height();
                    }
                    mLightVector->setVisible(true);
                }
                yPos += mIcons[0]->height();
            }
        }
        mName->setGeometry(0, yPos, this->width(), labelHeight);
        yPos += mName->height();
    }

signals:

    /// emits when the widget is clicked
    void clicked(QString);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), cor::computeHighlightColor(mSelectedCount, mReachableCount));

        // draw line at bottom of widget
        QRect area(x(), y(), width(), height());
        QPainter linePainter(this);
        linePainter.setRenderHint(QPainter::Antialiasing);
        linePainter.setBrush(QBrush(QColor(255, 255, 255)));
        QLine spacerLine(QPoint(area.x(), area.height() - 3),
                         QPoint(area.width(), area.height() - 3));
        linePainter.drawLine(spacerLine);
    }

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*) { emit clicked(mController.name()); }

private:
    /// create the icons for the the contgroller.
    void createIcons(const cor::Controller& controller, cor::EArduCorStatus status) {
        if (status == cor::EArduCorStatus::searching) {
            // show a syncing widget, we haven't found it yet
            mSyncWidget->setVisible(true);
            mSyncWidget->changeState(ESyncState::syncing);
        } else {
            mSyncWidget->setVisible(false);
            auto numOfLights = std::min(std::uint32_t(controller.names().size()), 5u);
            if (mIcons.size() != numOfLights || mIconPixmaps.size() != numOfLights) {
                clearIconMemory();
                mIconPixmaps = std::vector<QPixmap>(numOfLights);
                mIcons = std::vector<QLabel*>(numOfLights, nullptr);
                for (auto i = 0u; i < numOfLights; ++i) {
                    mIcons[i] = new QLabel(this);
                    mIcons[i]->setVisible(true);
                }
            }
        }
        resize();
    }

    /// update the lights for the controller
    void updateLights(const std::vector<cor::Light>& lights) {
        // allocate the widgets when we know how many lights there are
        if (mLightVector == nullptr) {
            auto numOfLights = std::min(std::uint32_t(mController.names().size()), 5u);
            initLightVector(numOfLights);
        } else if (mLightVector != nullptr) {
            auto numOfLights = std::min(std::uint32_t(mController.names().size()), 5u);
            if (numOfLights != mLightVector->lightCount()) {
                delete mLightVector;
                initLightVector(numOfLights);
            }
        }
        mLightVector->updateLights(lights);
    }

    void initLightVector(std::uint32_t lightCount) {
        mLightVector = new cor::LightVectorWidget(lightCount, 1, true, this);
        mLightVector->enableButtonInteraction(false);
        mLightVector->setStyleSheet(cor::kTransparentStylesheet);
        if (lightCount == 1) {
            mLightVector->setButtonIconPercent(0.8f);
        } else {
            mLightVector->setButtonIconPercent(1.0f);
        }
    }

    /// clear the memory of the pre-existing icons
    void clearIconMemory() {
        for (auto icon : mIcons) {
            delete icon;
        }
        mIcons.clear();
        mIconPixmaps.clear();
    }

    /// creates a hardwareIcon.
    void createHardwareIcon(std::uint32_t i, std::uint32_t iconHeight, std::uint32_t yPos) {
        if (mIconPixmaps[i].size() != QSize(iconHeight, iconHeight)) {
            mIcons[i]->setGeometry(i * (1.1 * iconHeight), yPos, iconHeight, iconHeight);
            mIconPixmaps[i] = lightHardwareTypeToPixmap(mLights[i].hardwareType());
            mIconPixmaps[i] = mIconPixmaps[i].scaled(iconHeight,
                                                     iconHeight,
                                                     Qt::IgnoreAspectRatio,
                                                     Qt::SmoothTransformation);
            mIcons[i]->setPixmap(mIconPixmaps[i]);
            mIcons[i]->setVisible(true);
        }
    }

    /// label to display name
    QLabel* mName;

    /// list of selected lights
    cor::LightList* mSelectedLights;

    /// sync widget to display when searching for the light
    SyncWidget* mSyncWidget;

    /// vector of icons for the lights attached to the controller
    std::vector<QLabel*> mIcons;

    /// vector of pixmaps for mIcons
    std::vector<QPixmap> mIconPixmaps;

    /// vector of lights for the preview
    std::vector<cor::Light> mLights;

    /// light vector for displaying the state of the lights.
    cor::LightVectorWidget* mLightVector;

    /// count of reachable lights
    std::uint32_t mReachableCount;

    /// count of selected lights.
    std::uint32_t mSelectedCount;

    /// controller being displayed
    cor::Controller mController;

    /// status of the controller (whether it is connected or searching)
    cor::EArduCorStatus mStatusType;
};
#endif // DISPLAYPREVIEWARDUCORWIDGET_H
