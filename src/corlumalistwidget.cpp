#include "corlumalistwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"

CorlumaListWidget::CorlumaListWidget(QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());
    mScaleWidth = 0.9f;

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mWidget->setMaximumWidth(parent->width() * mScaleWidth);
    mWidget->setMinimumWidth(parent->width() * mScaleWidth);

    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    this->setWidget(mWidget);
}


void CorlumaListWidget::resizeEvent(QResizeEvent *) {
    mWidget->setMaximumWidth(this->parentWidget()->width() * mScaleWidth);
    mWidget->setMinimumWidth(this->parentWidget()->width()  * mScaleWidth);
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        mWidgets[i]->resize();
    }
    resizeWidgets();

//    mWidget->setMinimumWidth(this->parentWidget()->width() * mScaleWidth);
//    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
//        mWidgets[i]->setMaximumSize(this->size());
//    }
}

void CorlumaListWidget::addWidget(ListCollectionWidget *widget) {
    int widgetIndex = searchForWidget(widget->key());
    if (widgetIndex == -1) {
        widget->setParent(mWidget);
        connect(widget, SIGNAL(rowCountChanged(int)), this, SLOT(widgetHeightChanged(int)));
        mWidgets.push_back(widget);
        resizeWidgets();
    }
}

void CorlumaListWidget::removeWidget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {

        //mWidgets[widgetIndex];

        // move all widgets below it up.
    }
}

ListCollectionWidget *CorlumaListWidget::widget(uint32_t index) {
    if (index >= mWidgets.size()) {
        throw "ERROR: requested a widget that is out of bounds";
    }
    return mWidgets[index];
}

ListCollectionWidget *CorlumaListWidget::widget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {
        // if it exists, return it
        return mWidgets[widgetIndex];
    } else {
        // if it doesn't exist, return nullptr
        return nullptr;
    }
}

int CorlumaListWidget::searchForWidget(QString key) {
    int widgetIndex = -1;
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        if (mWidgets[i]->key().compare(key) == 0) {
            widgetIndex = i;
        }
    }
    return widgetIndex;
}

void CorlumaListWidget::resizeWidgets() {
    uint32_t yPos = 0;
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        mWidgets[i]->setListHeight(this->height());
        QSize preferredSize = mWidgets[i]->preferredSize();
        mWidgets[i]->setGeometry(0, yPos, preferredSize.width(), preferredSize.height());
        yPos += preferredSize.height();

        if (!mWidgets[i]->isMoodWidget()) {
            ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(mWidgets[i]);
            devicesWidget->updateRightHandButtons();
        }
    }
    mWidget->setMinimumHeight(yPos);
}


void CorlumaListWidget::widgetHeightChanged(int rowCount) {
    Q_UNUSED(rowCount);
    resizeWidgets();
}
