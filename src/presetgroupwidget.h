#ifndef PRESETGROUPWIDGET_H
#define PRESETGROUPWIDGET_H

#include <QLabel>
#include <QObject>
#include <QWidget>

#include "cor/objects/light.h"
#include "cor/widgets/button.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PresetGroupWidget is a widget used on the Preset
 * Color Groups Page to display the name and all the routine
 * options for a group group.
 */
class PresetGroupWidget : public QWidget {
    Q_OBJECT
public:
    /*!
     * \brief PresetGroupWidget constructor for PresetGroupWidget
     *
     * \param name name of color group
     * \param group enumerated type representation of color group
     * \param colors colors in color group
     * \param parent parent widget
     */
    explicit PresetGroupWidget(const QString& name, EPalette palette, QWidget* parent);

    /*!
     * \brief setChecked acts similarly to the setChecked of a standard QPushButton, but
     * allows you to call the specific button of the PresetGroupWidget by its lighting
     * palette.
     *
     * \param palette palette to use
     */
    void setChecked(EPalette palette);

    /// resize this widget and all subwidgets
    void resize();

signals:

    /*!
     * \brief presetButtonClicked signal that any of the lightsbuttons were pressed and emitting
     *        their own signal.
     */
    void presetButtonClicked(cor::LightState);

private slots:

    /*!
     * \brief multiButtonClicked takes the multiButtonClicked LightsButton signals emitted from each
     * of its buttons and forwards them as a single signal.
     *
     * \param routine widget's lighting routine
     * \param group widget's color palette
     */
    void multiButtonClicked(cor::LightState object) { emit presetButtonClicked(object); }

private:
    cor::Button* mButton;

    /*!
     * \brief mLabel label for name of preset group.
     */
    QLabel* mLabel;

    /*!
     * \brief mLayout layout of widget
     */
    QGridLayout* mLayout;

    /*!
     * \brief kCheckedStyleSheet stylesheet for when the icon is checked
     */
    static const QString kCheckedStyleSheet;

    /*!
     * \brief kUncheckedStyleSheet stylesheet for when the icon is unchecked
     */
    static const QString kUncheckedStyleSheet;
};


#endif // PRESETGROUPWIDGET_H
