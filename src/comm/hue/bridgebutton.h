#ifndef BRIDGEBUTTON_H
#define BRIDGEBUTTON_H

#include <QPushButton>


namespace hue {
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The BridgeButton class is an ultra simple button that emits its text instead of just that
 * it was clicked.
 */
class BridgeButton : public QPushButton {
    Q_OBJECT

public:
    explicit BridgeButton(const QString& name, QWidget* parent) : QPushButton(name, parent) {
        connect(this, SIGNAL(clicked(bool)), this, SLOT(handleClicked(bool)));
    }

signals:

    /// emits its text when it is clicked
    void clicked(QString);

private slots:
    /// handles a click and emits its text.
    void handleClicked(bool) { emit clicked(text()); }
};

} // namespace hue

#endif // BRIDGEBUTTON_H
