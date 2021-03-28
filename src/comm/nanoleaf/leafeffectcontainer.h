#ifndef LEAFEFFECTCONTAINER_H
#define LEAFEFFECTCONTAINER_H

#include <QWidget>
#include "comm/nanoleaf/leafeffectwidget.h"
#include "cor/listlayout.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafEffectContainer class is an expanding widget that displays multiple
 * leafeffectwidgets.
 */
class LeafEffectContainer : public QWidget {
    Q_OBJECT
public:
    explicit LeafEffectContainer(QWidget* parent);

    /// shows the effects in the vector, with the height provided used for each effect widget.
    void showEffects(const QString& currentEffectName,
                     const std::vector<nano::LeafEffect>& effects,
                     int height);

    /// getter for the effects displayed
    const std::vector<nano::LeafEffect>& effects() { return mEffects; }

    /// change the row height
    void changeRowHeight(int rowHeight) { mRowHeight = rowHeight; }

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

    /// resize programmatically
    void resize();

signals:
    /// emits when an effect is selected
    void selectEffect(QString);

private slots:

    /// handles when an effect is selected
    void effectSelected(QString);

private:
    /// layout for widget
    cor::ListLayout mListLayout;

    /// all effects in the container
    std::vector<nano::LeafEffect> mEffects;

    /// height for any efefct widget.
    int mRowHeight;
};

} // namespace nano
#endif // LEAFEFFECTCONTAINER_H
