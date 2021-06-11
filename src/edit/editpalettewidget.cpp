#include "editpalettewidget.h"
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>
#include "utils/qt.h"

EditPaletteWidget::EditPaletteWidget(QWidget* parent)
    : QWidget(parent),
      mColorPicker{new SingleColorPicker(this)},
      mPaletteColors{new PaletteColorPicker(this)},
      mAddButton{new QPushButton("+", this)},
      mRemoveButton{new QPushButton("-", this)},
      mNameInput{new QLineEdit(this)},
      mSaveButton{new QPushButton("save", this)},
      mRowHeight{10} {
    mColorPicker->updateBrightness(100);
    mColorPicker->enable(true, EColorPickerType::color);
    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mPaletteColors,
            SIGNAL(selectionChanged(QColor, std::uint32_t)),
            this,
            SLOT(changedSelection(QColor, std::uint32_t)));

    mRemoveButton->setEnabled(false);
    connect(mAddButton, SIGNAL(clicked(bool)), this, SLOT(addButtonPressed(bool)));
    connect(mRemoveButton, SIGNAL(clicked(bool)), this, SLOT(removeButtonPressed(bool)));
    connect(mSaveButton, SIGNAL(clicked(bool)), this, SLOT(saveButtonPresed(bool)));
}

void EditPaletteWidget::loadPalette(const cor::Palette& palette) {
    mNameInput->setText(palette.name());
    mPaletteColors->updateColors(palette.colors());
    mColorPicker->updateColorStates(palette.colors()[0], palette.colors()[0].valueF() * 100.0);
    mPalette = palette;
}

void EditPaletteWidget::lineEditChanged(QString) {}

void EditPaletteWidget::addButtonPressed(bool) {
    changeColorCount(true);
}

void EditPaletteWidget::removeButtonPressed(bool) {
    changeColorCount(false);
}

void EditPaletteWidget::colorChanged(const QColor& color) {
    auto colors = mPaletteColors->colors();
    colors[mPaletteColors->selectedIndex()] = color;
    mPaletteColors->updateColors(colors);
}

void EditPaletteWidget::saveButtonPresed(bool) {
    qDebug() << " save button pressed!";
    cor::Palette palette(mPalette.uniqueID(), mNameInput->text(), mPaletteColors->colors());
    emit savePalette(palette);
}

void EditPaletteWidget::changedSelection(QColor color, std::uint32_t) {
    mColorPicker->updateColorStates(color, color.valueF() * 100.0);
}


void EditPaletteWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void EditPaletteWidget::resize() {
    QSize size = parentWidget()->size();
    setFixedSize(int(size.width() * 0.75f), int(size.height() * 0.75f));

    int rowHeight = this->height() / 8;
    auto xSpacer = width() * 0.025;
    auto standardWidth = width() - xSpacer * 2;
    auto buttonWidth = rowHeight;
    auto yPos = 0;
    mNameInput->setGeometry(xSpacer, yPos, standardWidth - buttonWidth * 2, rowHeight);
    mSaveButton->setGeometry(xSpacer + mNameInput->width(), 0, buttonWidth * 2, rowHeight);
    yPos += mNameInput->height();

    auto xPos = xSpacer;
    mPaletteColors->setGeometry(xPos, yPos, standardWidth - buttonWidth * 2, rowHeight);
    xPos += mPaletteColors->width();
    mAddButton->setGeometry(xPos, yPos, buttonWidth, rowHeight);
    xPos += mAddButton->width();
    mRemoveButton->setGeometry(xPos, yPos, buttonWidth, rowHeight);
    yPos += mPaletteColors->height();

    mColorPicker->setGeometry(xSpacer, yPos, standardWidth, rowHeight * 6);
}

void EditPaletteWidget::pushIn() {
    isOpen(true);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())),
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)));

    raise();
    setVisible(true);
    resize();
    mPaletteColors->updateColors(mPaletteColors->colors());
}

void EditPaletteWidget::pushOut() {
    isOpen(false);

    moveWidget(
        this,
        QPoint(int(parentWidget()->width() * 0.125f), int(parentWidget()->height() * 0.125f)),
        QPoint(int(parentWidget()->width() * 0.125f), int(-1 * parentWidget()->height())));
}


void EditPaletteWidget::changeColorCount(bool addOne) {
    if (addOne) {
        mPaletteColors->changeColorCount(mPaletteColors->colorCount() + 1);
    } else {
        mPaletteColors->changeColorCount(mPaletteColors->colorCount() - 1);
    }

    if (mPaletteColors->colorCount() == 2) {
        mRemoveButton->setEnabled(false);
    } else {
        mRemoveButton->setEnabled(true);
    }

    if (mPaletteColors->colorCount() == mPaletteColors->maxColors()) {
        mAddButton->setEnabled(false);
    } else {
        mAddButton->setEnabled(true);
    }
}
