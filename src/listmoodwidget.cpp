/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>
#include "listmoodwidget.h"
#include "cor/utils.h"

ListMoodWidget::ListMoodWidget(const cor::LightGroup& group,
                                 const std::vector<std::vector<QColor> >& colors,
                                 QWidget *parent) {
    this->setParent(parent);
    mIsChecked = false;

    mGroup = group;
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
    mName->setText("<b>" + modifiedName + "</b>");
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

    mPalette = new  cor::PaletteWidget(3, 2, colors, cor::EPaletteWidgetType::eInfo, this);
    mPalette->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mPalette->enableButtonInteraction(false);
    mPalette->updateDevices(mGroup.devices);

    // setup layout
    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mName, 5);
    mTopLayout->addWidget(mEditButton, 1);
    mTopLayout->setContentsMargins(0,0,0,0);
    mFullLayout = new QVBoxLayout(this);
    mFullLayout->addLayout(mTopLayout, 1);
    mFullLayout->addWidget(mPalette, 2);

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

