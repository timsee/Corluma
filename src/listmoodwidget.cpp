/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>
#include "listmoodwidget.h"
#include "cor/utils.h"

ListMoodWidget::ListMoodWidget(const SLightGroup& group,
                                 const std::vector<std::vector<QColor> >& colors,
                                 QWidget *parent) {
    Q_UNUSED(parent);
    mIsChecked = false;

    mGroup = group ;
    mKey = mGroup.name;

    // setup icon
    QString reachableStlyeSheet = "background:rgba(0, 0, 0, 0%); color: #333;";
    QString backgroundStyleSheet = "background:rgba(0, 0, 0, 0%);";

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // setup main label
    mName = new QLabel(this);
    QString modifiedName;
    if (mGroup.name.size() > 15) {
        modifiedName = mGroup.name.mid(0, 12) + "...";
    } else {
        modifiedName = mGroup.name;
    }
    mName->setText(modifiedName);
    mName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mEditButton = new QPushButton(this);
    mEditButton->setStyleSheet("border: none;");
    mEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mEditButton, SIGNAL(clicked(bool)), this, SLOT(editButtonClicked(bool)));
    mEditIcon = QPixmap(":/images/editIcon.png");
    float editSize = mName->height() * 0.5f;
    mEditButton->setMaximumHeight(editSize);
    mEditButton->setIcon(QIcon(mEditIcon.scaled(editSize,
                                                editSize)));
    mEditButton->setIconSize(QSize(editSize,
                                   editSize));
    mEditButton->setHidden(true);

    cor::Light device = *mGroup.devices.begin();
    if(!device.isReachable) {
        mName->setStyleSheet(reachableStlyeSheet);
    } else {
        mName->setStyleSheet(backgroundStyleSheet);
    }

    int previewNumber = 5;
    mIconData = std::vector<IconData>(previewNumber, IconData(32, 32));
    mPreviews = std::vector<QLabel*>(previewNumber, nullptr);

    for (int i = 0; i < previewNumber; ++i) {
        mPreviews[i] = new QLabel;
        mPreviews[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        mPreviews[i]->setStyleSheet(backgroundStyleSheet);
//#ifndef MOBILE_BUILD
//        mPreviews[i]->setMaximumWidth(this->width() / 5);
//#endif

    }

    int index = 0;
    for (auto&& device : mGroup.devices) {
        if (index < previewNumber) {
            //TODO: fix this ratio...
#ifdef MOBILE_BUILD
            int size = std::min(this->width() / 5, this->height() / 2);
#else
            int size = std::min(this->width() / 20, this->height() / 2);
#endif
            if (device.lightingRoutine <= cor::ELightingRoutineSingleColorEnd ) {
                mIconData[index].setSingleLightingRoutine(device.lightingRoutine, device.color);
                QPixmap iconRendered = mIconData[index].renderAsQPixmap();
                mPreviews[index]->setPixmap(iconRendered.scaled(size * 0.9f,
                                                            size *  0.9f,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::FastTransformation));
                mPreviews[index]->setFixedSize(size, size);
            } else {
                mIconData[index].setMultiLightingRoutine(device.lightingRoutine, device.colorGroup, colors[(int)device.colorGroup]);
                QPixmap iconRendered = mIconData[index].renderAsQPixmap();
                mPreviews[index]->setPixmap(iconRendered.scaled(size * 0.9f,
                                                            size * 0.9f,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::FastTransformation));
                mPreviews[index]->setFixedSize(size, size);
            }
            index++;
        }
    }



    // setup layout
    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mName, 5);
    mTopLayout->addWidget(mEditButton, 1);
    mTopLayout->setContentsMargins(0,0,0,0);

    mBottomLayout = new QHBoxLayout;
    int temp = 1;
    for (auto moodWidget : mPreviews) {
        mBottomLayout->addWidget(moodWidget, 2);
        temp++;
    }
    mBottomLayout->setContentsMargins(0,0,0,0);

    mFullLayout = new QVBoxLayout(this);
    mFullLayout->addLayout(mTopLayout, 1);
    mFullLayout->addLayout(mBottomLayout, 1);
    setLayout(mFullLayout);

    this->setStyleSheet(backgroundStyleSheet);
}



void ListMoodWidget::enterEvent(QEvent *) {
    mEditButton->setHidden(false);

    float editSize = mName->height() * 0.95f;
    mEditIcon = mEditIcon.scaled(editSize, editSize,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
    mEditButton->setIcon(QIcon(mEditIcon));
    mEditButton->setIconSize(QSize(editSize,
                                   editSize));
    mEditButton->setFixedSize(editSize, editSize);
}

void ListMoodWidget::leaveEvent(QEvent *) {
    mEditButton->setHidden(true);
}

bool ListMoodWidget::setChecked(bool checked) {
    mIsChecked = checked;
    repaint();
    return mIsChecked;
}


void ListMoodWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (mIsChecked) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31)));
    }
}


void ListMoodWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked(mKey);
}

