#ifndef DISPLAYPREVIEWNANOLEAFWIDGET_H
#define DISPLAYPREVIEWNANOLEAFWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "comm/nanoleaf/leafmetadata.h"
#include "comm/nanoleaf/leafpanelimage.h"
#include "cor/widgets/listitemwidget.h"
#include "syncwidget.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DisplayPreviewNanoleafWidget class displays a Nanoleaf preview on the dsicovery page.
 */
class DisplayPreviewNanoleafWidget : public cor::ListItemWidget {
    Q_OBJECT
public:
    explicit DisplayPreviewNanoleafWidget(const nano::LeafMetadata& leafMetadata,
                                          cor::LightState state,
                                          nano::ELeafDiscoveryState status,
                                          QWidget* parent)
        : cor::ListItemWidget(leafMetadata.serialNumber(), parent),
          mIsHighlighted{false},
          mName{new QLabel(leafMetadata.name(), this)},
          mSyncWidget{new SyncWidget(this)},
          mStatusPrompt{new QLabel(this)},
          mPanelImage{new nano::LeafPanelImage(this)},
          mPanelLabel{new QLabel(this)},
          mRotation{420},
          mLeafMetadata{leafMetadata},
          mState{state},
          mStatus{status} {
        updateNanoleaf(leafMetadata, state, status);

        mPanelLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mStatusPrompt->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mName->setWordWrap(true);
        mStatusPrompt->setWordWrap(true);
    }

    /// getter for the nanoleaf
    const nano::LeafMetadata& nanoleaf() { return mLeafMetadata; }

    /// getter for the status
    nano::ELeafDiscoveryState status() { return mStatus; }

    /// set whether the light is highlighted or not.
    void setShouldHighlight(bool shouldHighlight) {
        mIsHighlighted = shouldHighlight;
        update();
    }

    /// update the nanoleaf
    void updateNanoleaf(const nano::LeafMetadata& metadata,
                        const cor::LightState& state,
                        nano::ELeafDiscoveryState status) {
        // pick up when status changes for edge cases.
        bool statusChanged = (status != mStatus);
        bool stateChanged = (state != mState);
        mLeafMetadata = metadata;
        mStatus = status;
        mState = state;
        handleStatusPrompt();
        if (statusChanged) {
            // resize when status changes
            resize();
        }
        if (mStatus == nano::ELeafDiscoveryState::searchingIP
            || mStatus == nano::ELeafDiscoveryState::searchingAuth
            || mStatus == nano::ELeafDiscoveryState::reverifying) {
            // show a syncing widget, we haven't found it yet
            mSyncWidget->setVisible(true);
            mStatusPrompt->setVisible(true);
            mSyncWidget->changeState(ESyncState::syncing);
        } else {
            mSyncWidget->setVisible(false);
            mStatusPrompt->setVisible(true);
            if (!mLeafMetadata.name().isEmpty()) {
                mName->setText(mLeafMetadata.name());
            } else {
                // handle edge case where name is not filled.
                if (!mLeafMetadata.hardwareName().isEmpty()) {
                    mName->setText(mLeafMetadata.hardwareName());
                } else {
                    mName->setText(mLeafMetadata.IP());
                }
            }
            if (mLeafMetadata.panels().orientationValue() != mRotation || stateChanged) {
                drawPanels();
            }
        }
    }

    /// resize the widget programmatically.
    void resize() {
        auto yPos = 0u;
        auto rowHeight = this->height() / 4;

        if (mStatus == nano::ELeafDiscoveryState::searchingIP
            || mStatus == nano::ELeafDiscoveryState::searchingAuth
            || mStatus == nano::ELeafDiscoveryState::reverifying) {
            mSyncWidget->setGeometry(0, yPos, this->width(), rowHeight);
            yPos += mSyncWidget->height();
            mName->setGeometry(0, yPos, this->width(), rowHeight);
            yPos += mName->height();
            mStatusPrompt->setGeometry(0, yPos, this->width(), rowHeight * 2);
            yPos += mStatusPrompt->height();
        } else if (mStatus == nano::ELeafDiscoveryState::connected) {
            mPanelLabel->setGeometry(0, yPos, this->width(), rowHeight * 3);

            drawPanels();
            yPos += mPanelLabel->height();

            mName->setGeometry(0, yPos, this->width(), rowHeight);
            yPos += mName->height();
        }
    }

signals:

    /// emits when a nanoleaf is clicked
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
        if (mIsHighlighted) {
            painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
        } else {
            painter.fillRect(rect(), QBrush(QColor(32, 31, 31, 255)));
        }

        // draw line at bottom of widget
        QRect area(0, y(), width(), height());
        auto lineOffset = 3;
        QPainter linePainter(this);
        linePainter.setRenderHint(QPainter::Antialiasing);
        linePainter.setBrush(QBrush(QColor(255, 255, 255)));
        QLine spacerLine(QPoint(area.x(), area.height() - lineOffset),
                         QPoint(area.width(), area.height() - lineOffset));
        linePainter.drawLine(spacerLine);
    }

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*) { emit clicked(mLeafMetadata.name()); }

private:
    /// handle the status prompt.
    void handleStatusPrompt() {
        QString text;
        switch (mStatus) {
            case nano::ELeafDiscoveryState::searchingAuth:
                text = "Light found! Hold the power button for around 5 seconds.";
                break;
            case nano::ELeafDiscoveryState::reverifying:
                text = "Testing: " + nanoleaf().IP();
                break;

            case nano::ELeafDiscoveryState::searchingIP:
                text = "Searching: " + nanoleaf().IP();
                break;

            case nano::ELeafDiscoveryState::connected:
            default:
                break;
        }
        mStatusPrompt->setText(text);
    }

    void drawPanels() {
        mRotation = mLeafMetadata.panels().orientationValue();
        // render the image for the panel
        mPanelImage->drawPanels(mLeafMetadata.panels(), mRotation, mState.palette(), mState.isOn());
        // draw the image to the panel label
        mPanelPixmap.convertFromImage(mPanelImage->image());
        if (!mPanelPixmap.isNull()) {
            mPanelPixmap = mPanelPixmap.scaled(mPanelLabel->width(),
                                               mPanelLabel->height(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
            mPanelLabel->setPixmap(mPanelPixmap);
        }
    }

    /// true if highlighted, false if not.
    bool mIsHighlighted;

    /// name of the nanoleaf.
    QLabel* mName;

    /// sync widget to display when searching for the light
    SyncWidget* mSyncWidget;

    /// status prompt for when searching but need user to hold down button.
    QLabel* mStatusPrompt;

    /// generates the panel image
    nano::LeafPanelImage* mPanelImage;

    /// label that shows the panel image
    QLabel* mPanelLabel;

    /// pixmap that stores the panel image.
    QPixmap mPanelPixmap;

    /// stored rotation of the lights
    int mRotation;

    /// metadata widget for the nanoleaf
    nano::LeafMetadata mLeafMetadata;

    /// state of the light
    cor::LightState mState;

    /// status for the discovery state.
    nano::ELeafDiscoveryState mStatus;
};

#endif // DISPLAYPREVIEWNANOLEAFWIDGET_H
