#include "lightcheckbox.h"

LightCheckBox::LightCheckBox(QWidget *parent) : QWidget(parent) {
    mButton = new QPushButton(this);
    mButton->setCheckable(true);
    mButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mButton, SIGNAL(clicked(bool)), this, SLOT(handleButton()));

    mLabel = new QLabel(this);
    mLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mLabel->setFont(QFont(mLabel->font().styleName(), 16, 0));
    mLabel->setAlignment(Qt::AlignCenter);

    mLayout = new QVBoxLayout;
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(mLabel, 2);
    mLayout->addWidget(mButton, 4);
    mButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(mLayout);

    mCheckedIcon = QIcon(QPixmap(":/qss_icons/rc/checkbox_checked.png"));
    mUncheckedIcon = QIcon(QPixmap(":/qss_icons/rc/checkbox_unchecked.png"));

    mChecked = false;
}

void LightCheckBox::setText(QString text) {
    mLabel->setText(text);
}

void LightCheckBox::setChecked(bool checked) {
    if (checked) {
        mChecked = true;
        mButton->setIcon(mCheckedIcon);
    } else {
        mChecked = false;
        mButton->setIcon(mUncheckedIcon);
    }
    emit clicked(checked);
    mButton->setChecked(false);
}

void LightCheckBox::handleButton() {
   setChecked(!mChecked);
}

void LightCheckBox::resizeEvent(QResizeEvent *event) {
    int height = static_cast<int>(mButton->size().height() * 0.8f);
    mButton->setIconSize(QSize(height, height));
    mCheckedIcon = QIcon(QPixmap(":/qss_icons/rc/checkbox_checked.png").scaled(height, height));
    mUncheckedIcon = QIcon(QPixmap(":/qss_icons/rc/checkbox_unchecked.png").scaled(height, height));
    if (mChecked) {
        mButton->setIcon(mCheckedIcon);
    } else {
        mButton->setIcon(mUncheckedIcon);
    }
}
