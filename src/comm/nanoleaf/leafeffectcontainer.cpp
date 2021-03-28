#include "leafeffectcontainer.h"
namespace nano {

LeafEffectContainer::LeafEffectContainer(QWidget* parent)
    : QWidget(parent),
      mListLayout(cor::EListType::linear) {}


void LeafEffectContainer::showEffects(const QString& currentEffectName,
                                      const std::vector<nano::LeafEffect>& effects,
                                      int height) {
    clear();
    mRowHeight = height;
    std::vector<bool> foundWidgets(effects.size(), false);
    // sort the effects by name
    auto sortedEffects = effects;
    auto lambda = [](const nano::LeafEffect& a, const nano::LeafEffect& b) -> bool {
        return a.name() < b.name();
    };
    std::sort(sortedEffects.begin(), sortedEffects.end(), lambda);
    for (const auto& effect : sortedEffects) {
        auto widget = new nano::LeafEffectWidget(effect, currentEffectName == effect.name(), this);
        connect(widget, SIGNAL(selectEffect(QString)), this, SLOT(effectSelected(QString)));
        mListLayout.insertWidget(widget);
    }

    resize();
}

void LeafEffectContainer::resize() {
    QPoint offset(0, 0);
    QSize size = mListLayout.widgetSize(QSize(width(), mRowHeight));

    int yPos = 0;
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
        auto widget = qobject_cast<nano::LeafEffectWidget*>(mListLayout.widgets()[i]);
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

void LeafEffectContainer::effectSelected(QString effect) {
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
        auto widget = qobject_cast<nano::LeafEffectWidget*>(mListLayout.widgets()[i]);
        widget->setSelected(effect == widget->effect().name());
    }
    emit selectEffect(effect);
}


void LeafEffectContainer::clear() {
    mListLayout.clear();
}


} // namespace nano
