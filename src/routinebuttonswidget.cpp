/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "routinebuttonswidget.h"

#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>

RoutineButtonsWidget::RoutineButtonsWidget(EWidgetGroup widgetGroup, std::vector<QColor> colors, QWidget *parent) : QWidget(parent) {

    mLayout = new QGridLayout(this);
    mLayout->setMargin(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);
    mLayout->setVerticalSpacing(0);

    if (widgetGroup == EWidgetGroup::eSingleRoutines) {
        mRoutines = std::vector<std::pair<QString, QJsonObject> >(8);
        cor::Light light;
        QJsonObject routineObject;
        light.speed = 100;
        light.color = QColor(0, 0, 0);

        mRoutines[0].first = "Solid";
        light.routine = ERoutine::eSingleSolid;
        mRoutines[0].second = lightToJson(light);

        mRoutines[1].first = "Blink";
        light.routine = ERoutine::eSingleBlink;
        mRoutines[1].second = lightToJson(light);

        mRoutines[2].first = "Wave";
        light.routine = ERoutine::eSingleWave;
        mRoutines[2].second = lightToJson(light);

        mRoutines[3].first = "Glimmer";
        light.routine = ERoutine::eSingleGlimmer;
        routineObject["param"] = 15;
        mRoutines[3].second = lightToJson(light);

        mRoutines[4].first = "Linear Fade";
        light.routine = ERoutine::eSingleFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 0;
        mRoutines[4].second = routineObject;

        mRoutines[5].first = "Sine Fade";
        light.routine = ERoutine::eSingleFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 1;
        mRoutines[5].second = routineObject;

        mRoutines[6].first = "Saw Fade In";
        light.routine = ERoutine::eSingleSawtoothFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 0;
        mRoutines[6].second = routineObject;

        mRoutines[7].first = "Saw Fade Out";
        light.routine = ERoutine::eSingleSawtoothFade;
        routineObject = lightToJson(light);
        routineObject["param"] = 1;
        mRoutines[7].second = routineObject;

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        int maxColumn = 4;
        for (int i = 0; i < (int)mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(mRoutines[i].second, std::vector<QColor>(), this);
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(routineChanged(QJsonObject)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, i % maxColumn);
            mLayout->addWidget(mLabels[i], rowCount + 1, i % maxColumn);
       }

    } else if (widgetGroup == EWidgetGroup::eMultiRoutines) {
        mRoutines = std::vector<std::pair<QString, QJsonObject> >(5);
        cor::Light light;
        QJsonObject routineObject;
        light.speed = 100;
        light.palette = EPalette::eCustom;

        mRoutines[0].first = "Glimmer";
        light.routine = ERoutine::eMultiGlimmer;
        routineObject["param"] = 15;
        mRoutines[0].second = lightToJson(light);

        mRoutines[1].first = "Fade";
        light.routine = ERoutine::eMultiFade;
        mRoutines[1].second = lightToJson(light);

        mRoutines[2].first = "Random Solid";
        light.routine = ERoutine::eMultiRandomSolid;
        mRoutines[2].second = lightToJson(light);

        mRoutines[3].first = "Random Individual";
        light.routine = ERoutine::eMultiRandomIndividual;
        mRoutines[3].second = lightToJson(light);

        mRoutines[4].first = "Bars";
        light.routine = ERoutine::eMultiBars;
        routineObject["param"] = 4;
        mRoutines[4].second = lightToJson(light);

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        int maxColumn = 3;
        for (int i = 0; i < (int)mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(mRoutines[i].second, colors, this);
            mRoutineButtons[i]->setStyleSheet("background-color: rgb(52, 52, 52); ");
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(routineChanged(QJsonObject)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, i % maxColumn);
            mLayout->addWidget(mLabels[i], rowCount + 1, i % maxColumn);
       }
    } else {
        throw "RoutinesButtonWidget is not set up to handle that widget group";
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
        mRoutineButtons[i]->updateRoutine(mRoutines[i].second, colors);
    }
}

void RoutineButtonsWidget::routineChanged(QJsonObject routineObject) {
    QString routineName = jsonToButtonName(routineObject);
    highlightRoutineButton(routineName);
    emit newRoutineSelected(routineObject);
}

QString RoutineButtonsWidget::jsonToButtonName(const QJsonObject& routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    int param = INT_MIN;
    if (routineObject["param"].isDouble()) {
        param = routineObject["param"].toDouble();
    }
    for (uint32_t i = 0; i < mRoutineButtons.size(); i++) {
        ERoutine buttonRoutine = stringToRoutine(mRoutineButtons[i]->routine()["routine"].toString());
        int buttonParam = INT_MIN;

        if (mRoutineButtons[i]->routine()["param"].isDouble()) {
            buttonParam = mRoutineButtons[i]->routine()["param"].toDouble();
        }
        if (routine == buttonRoutine && param == buttonParam) {
            return mLabels[i]->text();
        }
    }
    return QString();
}

void RoutineButtonsWidget::singleRoutineColorChanged(QColor color) {
    for (int i = 0; i < (int)mRoutineButtons.size(); ++i) {
        QJsonObject routineObject = mRoutines[i].second;
        routineObject["red"]   = color.red();
        routineObject["green"] = color.green();
        routineObject["blue"]  = color.blue();
        mRoutineButtons[i]->updateRoutine(routineObject, std::vector<QColor>());
    }
}

void RoutineButtonsWidget::resize(QSize size) {
    this->setMinimumWidth(size.width());
    this->setMinimumHeight(size.height() / 3);
    this->setMaximumWidth(size.width());
    this->setMaximumHeight(size.height() / 3);
}

void RoutineButtonsWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}
