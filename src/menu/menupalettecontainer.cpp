#include "menupalettecontainer.h"
#include "storedpalettewidget.h"
#include "utils/qt.h"

MenuPaletteContainer::MenuPaletteContainer(QWidget* parent)
    : QWidget(parent),
      mListLayout{cor::EListType::grid},
      mHeight{10u} {}


void MenuPaletteContainer::showPalettes(const std::vector<cor::Palette>& palettes) {
    clear();
    auto sortedPalettes = palettes;
    auto lambda = [](const cor::Palette& a, const cor::Palette& b) -> bool {
        return a.name() < b.name();
    };
    std::sort(sortedPalettes.begin(), sortedPalettes.end(), lambda);
    for (const auto& palette : sortedPalettes) {
        auto widget = new StoredPaletteWidget(palette, this);
        connect(widget,
                SIGNAL(paletteButtonClicked(cor::Palette)),
                this,
                SLOT(selectPalette(cor::Palette)));
        mListLayout.insertWidget(widget);
    }
    resize();
}

void MenuPaletteContainer::clear() {
    mListLayout.clear();
}

void MenuPaletteContainer::resize() {
    QPoint offset(0, 0);
    QSize size = mListLayout.widgetSize(QSize(width(), mHeight));

    int yPos = 0;
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
        auto widget = qobject_cast<StoredPaletteWidget*>(mListLayout.widgets()[i]);
        widget->setVisible(true);
        Q_ASSERT(widget);
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                              offset.y() + position.y() * size.height(),
                                              size.width(),
                                              size.height());
        yPos = mListLayout.widgets()[i]->geometry().y() + mListLayout.widgets()[i]->height();
    }
    setFixedHeight(yPos);
}



void MenuPaletteContainer::selectPalette(cor::Palette palette) {
    for (const auto& existingWidget : mListLayout.widgets()) {
        auto widget = qobject_cast<StoredPaletteWidget*>(existingWidget);
        Q_ASSERT(widget);
        if (widget->key() == palette.uniqueID()) {
            // widget->setSelected(true);
        } else {
            // widget->setSelected(false);
        }
    }
    emit paletteSelected(palette);
}
