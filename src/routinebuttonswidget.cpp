/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "routinebuttonswidget.h"

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>

#include "cor/presetpalettes.h"
#include "utils/exception.h"
#include "utils/qt.h"

RoutineButtonsWidget::RoutineButtonsWidget(EWidgetGroup widgetGroup,
                                           const std::vector<QColor>& colors,
                                           QWidget* parent)
    : QWidget(parent),
      mIsOpen{false} {
    mLayout = new QGridLayout(this);
    mLayout->setMargin(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);
    mLayout->setVerticalSpacing(0);
    cor::Light light("Routines", "Routines", ECommType::MAX);
    if (widgetGroup == EWidgetGroup::singleRoutines) {
        mRoutines = std::vector<std::pair<QString, QJsonObject>>(8);
        QJsonObject routineObject;
        light.speed = 100;
        light.isOn = true;
        light.color = QColor(0, 0, 0);

        mRoutines[0].first = "Solid";
        light.routine = ERoutine::singleSolid;
        mRoutines[0].second = lightToJson(light);

        mRoutines[1].first = "Blink";
        light.routine = ERoutine::singleBlink;
        mRoutines[1].second = lightToJson(light);

        mRoutines[2].first = "Wave";
        light.routine = ERoutine::singleWave;
        mRoutines[2].second = lightToJson(light);

        mRoutines[3].first = "Glimmer";
        light.routine = ERoutine::singleGlimmer;
        routineObject["param"] = 15;
        mRoutines[3].second = lightToJson(light);

        mRoutines[4].first = "Linear Fade";
        light.routine = ERoutine::singleFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 0;
        mRoutines[4].second = routineObject;

        mRoutines[5].first = "Sine Fade";
        light.routine = ERoutine::singleFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 1;
        mRoutines[5].second = routineObject;

        mRoutines[6].first = "Saw Fade In";
        light.routine = ERoutine::singleSawtoothFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 0;
        mRoutines[6].second = routineObject;

        mRoutines[7].first = "Saw Fade Out";
        light.routine = ERoutine::singleSawtoothFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 1;
        mRoutines[7].second = routineObject;

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        uint32_t maxColumn = 4;
        for (uint32_t i = 0; i < mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this, mRoutines[i].second);
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i],
                    SIGNAL(buttonClicked(QJsonObject)),
                    this,
                    SLOT(routineChanged(QJsonObject)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, int(i % maxColumn));
            mLayout->addWidget(mLabels[i], rowCount + 1, int(i % maxColumn));
        }

    } else if (widgetGroup == EWidgetGroup::multiRoutines) {
        mRoutines = std::vector<std::pair<QString, QJsonObject>>(5);
        QJsonObject routineObject;
        light.speed = 100;
        light.isOn = true;
        light.palette = Palette(paletteToString(EPalette::custom), colors, 51);

        mRoutines[0].first = "Glimmer";
        light.routine = ERoutine::multiGlimmer;
        routineObject["param"] = 15;
        mRoutines[0].second = lightToJson(light);

        mRoutines[1].first = "Fade";
        light.routine = ERoutine::multiFade;
        mRoutines[1].second = lightToJson(light);

        mRoutines[2].first = "Random Solid";
        light.routine = ERoutine::multiRandomSolid;
        mRoutines[2].second = lightToJson(light);

        mRoutines[3].first = "Random Individual";
        light.routine = ERoutine::multiRandomIndividual;
        mRoutines[3].second = lightToJson(light);

        mRoutines[4].first = "Bars";
        light.routine = ERoutine::multiBars;
        routineObject["param"] = 4;
        mRoutines[4].second = lightToJson(light);

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        uint32_t maxColumn = 3;
        for (uint32_t i = 0; i < mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this, mRoutines[i].second);

            mRoutineButtons[i]->setStyleSheet("background-color: rgb(52, 52, 52); ");
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i],
                    SIGNAL(buttonClicked(QJsonObject)),
                    this,
                    SLOT(routineChanged(QJsonObject)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, int(i % maxColumn));
            mLayout->addWidget(mLabels[i], rowCount + 1, int(i % maxColumn));
        }
    } else {
        THROW_EXCEPTION("RoutinesButtonWidget is not set up to handle that widget group");
    }

    setLayout(mLayout);
}



void RoutineButtonsWidget::highlightRoutineButton(const QString& label) {
    for (uint i = 0; i < mRoutineButtons.size(); i++) {
        if (mLabels[i]->text().compare(label) == 0) {
            mRoutineButtons[i]->setChecked(true);
        } else {
            mRoutineButtons[i]->setChecked(false);
        }
    }
}

void RoutineButtonsWidget::multiRoutineColorsChanged(const std::vector<QColor>& colors) {
    for (uint32_t i = 0; i < mRoutineButtons.size(); i++) {
        cor::Light light = cor::jsonToLight(mRoutines[i].second);
        light.palette = Palette(paletteToString(EPalette::custom), colors, 51);
        mRoutines[i].second = cor::lightToJson(light);
        mRoutineButtons[i]->updateRoutine(mRoutines[i].second);
    }
}

void RoutineButtonsWidget::routineChanged(const QJsonObject& routineObject) {
    QString routineName = jsonToButtonName(routineObject);
    highlightRoutineButton(routineName);
    emit newRoutineSelected(routineObject);
}

QString RoutineButtonsWidget::jsonToButtonName(const QJsonObject& routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    int param = INT_MIN;
    if (routineObject["param"].isDouble()) {
        param = int(routineObject["param"].toDouble());
    }
    for (uint32_t i = 0; i < mRoutineButtons.size(); i++) {
        ERoutine buttonRoutine =
            stringToRoutine(mRoutineButtons[i]->routine()["routine"].toString());
        int buttonParam = INT_MIN;

        if (mRoutineButtons[i]->routine()["param"].isDouble()) {
            buttonParam = int(mRoutineButtons[i]->routine()["param"].toDouble());
        }
        if (routine == buttonRoutine && param == buttonParam) {
            return mLabels[i]->text();
        }
    }
    return QString();
}

void RoutineButtonsWidget::singleRoutineColorChanged(const QColor& color) {
    for (uint32_t i = 0; i < mRoutineButtons.size(); ++i) {
        QJsonObject routineObject = mRoutines[i].second;
        routineObject["hue"] = color.hueF();
        routineObject["sat"] = color.saturationF();
        routineObject["bri"] = color.valueF();
        mRoutineButtons[i]->updateRoutine(routineObject);
    }
}

void RoutineButtonsWidget::resize(QSize size) {
    this->setFixedWidth(size.width());
    this->setFixedHeight(size.height() / 3);

    if (mIsOpen) {
        this->setGeometry(0,
                          this->parentWidget()->height() - this->height(),
                          this->width(),
                          this->height());
    } else {
        this->setGeometry(0, this->parentWidget()->height(), this->width(), this->height());
    }
}

void RoutineButtonsWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void RoutineButtonsWidget::showWidget(bool shouldShow) {
    if (mIsOpen && !shouldShow) {
        cor::moveWidget(this, this->pos(), QPoint(0, this->parentWidget()->height()));

        mIsOpen = false;
    } else if (!mIsOpen && shouldShow) {
        // mSingleRoutineWidget->singleRoutineColorChanged(mColor);  // update colors of single
        // color routine
        cor::moveWidget(this,
                        this->pos(),
                        QPoint(0, this->parentWidget()->height() - this->height()));
        mIsOpen = true;
    }
}
