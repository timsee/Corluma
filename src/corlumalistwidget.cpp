#include "corlumalistwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"

CorlumaListWidget::CorlumaListWidget(QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setFixedWidth(this->viewport()->width());

    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    this->setWidget(mWidget);
}


void CorlumaListWidget::resizeEvent(QResizeEvent *) {
    // disable horizontal scorlling by always forcing scroll area to be width
    // of viewport.
    mWidget->setFixedWidth(this->viewport()->width());

    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        mWidgets[i]->resize();
    }
    resizeWidgets();
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
    uint32_t newHeight = std::max(yPos, (uint32_t)this->viewport()->height());
    mWidget->setFixedHeight(newHeight);
}


void CorlumaListWidget::widgetHeightChanged(int rowCount) {
    Q_UNUSED(rowCount);
    resizeWidgets();
}
