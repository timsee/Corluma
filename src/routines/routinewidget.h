#ifndef ROUTINEWIDGET_H
#define ROUTINEWIDGET_H

#include <QLabel>
#include "cor/objects/lightstate.h"
#include "cor/stylesheets.h"
#include "cor/widgets/checkbox.h"
#include "icondata.h"
#include "utils/painterutils.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The RoutineWidget class is the base class for QWidgets taht display a routine. This
 * provides the basic widgets needed to display and select a routine. Derived widgets may have
 * additional widgets to adjust the routine (such as sliders or buttons)
 */
class RoutineWidget : public QWidget {
public:
    RoutineWidget(QWidget* parent, const QString& name, ERoutine routine)
        : QWidget(parent),
          mLeftWidgetSize{30, 10},
          mProtocol{EProtocolType::arduCor},
          mRectOptions{cor::EPaintRectOptions::noBottom},
          mCheckBox{new cor::CheckBox(this)},
          mName{new QLabel(name, this)},
          mIcon{new QLabel(this)} {
        mState.isOn(true);
        mState.color(QColor(0, 0, 0));
        mState.routine(routine);

        mCheckBox->checkboxState(ECheckboxState::unchecked);

        mName->setStyleSheet(cor::kTransparentStylesheet);
    }

    virtual ~RoutineWidget() = default;

    /// update the state of the widget
    void updateState(const cor::LightState& state) {
        mState = state;
        updateStateIcon();
    }

    /// change the size of the checkbox, label, and state widget in the top left of the widget
    void changeLeftWidgetSize(const QSize& widgetSize) { mLeftWidgetSize = widgetSize; }

    /// getter for current routine
    ERoutine routine() { return mState.routine(); }

    /// change the outline options for the widget.
    void rectOptions(cor::EPaintRectOptions rectOptions) { mRectOptions = rectOptions; }

    /// handles whether the widget is selected or not based off of its routine.
    virtual void selectRoutine(ERoutine routine, int) {
        // mHighlight = (mState.routine() == routine);
        if ((mState.routine() == routine)) {
            mCheckBox->checkboxState(ECheckboxState::checked);
        } else {
            mCheckBox->checkboxState(ECheckboxState::unchecked);
        }
        update();
    }

    /// set the most featured protocol for a routinewidget, which may change the features displayed
    /// to the user.
    void setProtocol(EProtocolType protocol) {
        mProtocol = protocol;
        updateProtocol();
    }

protected:
    /// paints the widget
    void paintEvent(QPaintEvent*) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);

        // paint top line
        int greyValue = 185;
        QBrush brush(QColor(greyValue, greyValue, greyValue));
        QPen pen(brush, width() * 0.005);
        painter.setPen(pen);

        auto rect = QRect(0, 0, width(), height());

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect, cor::kBackgroundColor);

        cor::paintRect(painter, rect, mRectOptions);
    }

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

    /// implements the resizing of the widget
    virtual void resize() = 0;

    /// update the widget based on the protocol
    virtual void updateProtocol() {
        // in many cases, this function will do nothing, but in some cases, such as multiBars, we
        // cannot expect all lights to support these routines, so this allows us to change up the
        // routine widgets
    }

    /// updates and resizes the state icon
    void updateStateIcon() {
        mIconData.setRoutine(mState);
        QPixmap pixmap = mIconData.renderAsQPixmap();
        pixmap = pixmap.scaled(mIconSize.width(),
                               mIconSize.height(),
                               Qt::KeepAspectRatio,
                               Qt::FastTransformation);
        mIcon->setPixmap(pixmap);
        mIcon->setStyleSheet(cor::kTransparentStylesheet);
    }


    /// resizes the widgets in the top left of the widget. These are uniform across all
    /// RoutineWidgets.
    void resizeTopLeftMenu() {
        auto iconSide = std::min(mLeftWidgetSize.height() * 0.8, mLeftWidgetSize.width() / 4.0);
        mIconSize = QSize(iconSide, iconSide);
        updateStateIcon();

        auto xSpacer = mLeftWidgetSize.width() * 0.04;
        auto ySpacer = mLeftWidgetSize.height() * 0.04;

        auto widgetRowHeight = (mLeftWidgetSize.height() / 4) - ySpacer;
        auto yPos = ySpacer;
        auto xPos = xSpacer;
        mCheckBox->setGeometry(xPos, yPos, mLeftWidgetSize.width() / 2, widgetRowHeight * 3);
        xPos += mCheckBox->width() + xSpacer;
        mIcon->setGeometry(xPos, yPos, mLeftWidgetSize.width() / 2, widgetRowHeight * 3);
        yPos += mIcon->height();
        mName->setGeometry(xSpacer, yPos, mLeftWidgetSize.width(), widgetRowHeight);
    }

    /// the size allocated for the widgets on the top left
    QSize mLeftWidgetSize;

    /// the state shown in the top left of the widget
    cor::LightState mState;

    /// protocol for the widgets.
    EProtocolType mProtocol;

    /// used to generate the state icon
    IconData mIconData;

    /// size for icon
    QSize mIconSize;

    /// stores the outlien options for the widget.
    cor::EPaintRectOptions mRectOptions;

    /// checkbox for selecting/deselecting a routine.
    cor::CheckBox* mCheckBox;

    /// the name of the widget
    QLabel* mName;

    /// the icon for the widget.
    QLabel* mIcon;
};

#endif // ROUTINEWIDGET_H
