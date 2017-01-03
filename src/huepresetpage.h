#ifndef HUEPRESETPAGE_H
#define HUEPRESETPAGE_H

#include <QWidget>

namespace Ui {
class HuePresetPage;
}

/*!
 * \brief The HuePresetPage class unimplemented page of preset moods made specifically for hues.
 */
class HuePresetPage : public QWidget
{
    Q_OBJECT

public:

    /*!
     * Constructor
     */
    explicit HuePresetPage(QWidget *parent = 0);

    /*!
     * Deconstructor
     */
    ~HuePresetPage();

private:
    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::HuePresetPage *ui;
};

#endif // HUEPRESETPAGE_H
