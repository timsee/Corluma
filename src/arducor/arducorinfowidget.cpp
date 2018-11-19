

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>

#include "arducorinfowidget.h"

ArduCorInfoWidget::ArduCorInfoWidget(cor::Light light, QWidget *parent) : QWidget(parent), mLight(light) {
    const QString styleSheet = "background-color: rgba(0,0,0,0);";
    this->setStyleSheet(styleSheet);

    mName  = new QLabel(light.uniqueID(), this);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setTitleFontPointSize(14);

    QString pathString = commTypeToString(light.commType()) +  " " + light.controller() + " " + QString::number(light.index);
    mPathLabel = new QLabel(pathString, this);
    mPathLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mPathLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mHardwareTypeLabel = new QLabel("<b>Hardware:</b>  " + hardwareTypeToString(light.hardwareType), this);
    mHardwareTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mHardwareTypeLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mAPILabel = new QLabel("<b>API:</b>  " + QString::number(light.majorAPI) + "." + QString::number(light.minorAPI), this);
    mAPILabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mAPILabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    mLayout = new QGridLayout(this);

    mLayout->addWidget(mName, 0, 0, 1, 2);
    mLayout->addWidget(mPathLabel, 1, 0);
    mLayout->addWidget(mHardwareTypeLabel, 2, 0);
    mLayout->addWidget(mAPILabel, 3, 0);

    mKey = light.uniqueID();

    mIsChecked = false;
    hideDetails(true);
}


void ArduCorInfoWidget::updateLight(cor::Light light) {
    mLight = light;
}

void ArduCorInfoWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mKey);
}

void ArduCorInfoWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
}

void ArduCorInfoWidget::setHeight(int height) {
    if (mHideDetails) {
        height = height / 2;
    }
    int finalHeight = std::max(height, this->height());
    mHeight = finalHeight;
    this->setFixedHeight(height);
}

void ArduCorInfoWidget::hideDetails(bool shouldHide) {
    mAPILabel->setHidden(shouldHide);
    mHardwareTypeLabel->setHidden(shouldHide);
    mHideDetails = shouldHide;
}

void ArduCorInfoWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        //TODO: could I make this transparent in all cases?
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // draw line at bottom of widget
    QRect area(this->x(), this->y(), this->width(), this->height());
    QPainter linePainter(this);
    linePainter.setRenderHint(QPainter::Antialiasing);
    linePainter.setBrush(QBrush(QColor(255, 255, 255)));
    QLine spacerLine(QPoint(area.x(), area.height() - 3), QPoint(area.width(), area.height() - 3));
    linePainter.drawLine(spacerLine);
}



void ArduCorInfoWidget::setTitleFontPointSize(int pt) {
    if (pt <= 0) pt = 1;
    QString stylesheet = "font-size:" + QString::number(pt)+ "pt;";
    mName->setStyleSheet(stylesheet);

    QFont font(mName->font().toString(), pt);
    QFontMetrics fm(font);
    mName->setMinimumHeight(fm.height());
}
