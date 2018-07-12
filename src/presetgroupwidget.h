#ifndef PRESETGROUPWIDGET_H
#define PRESETGROUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include "cor/button.h"
#include "cor/light.h"

/// mode of this widget
enum class EPresetWidgetMode {
    arduino,
    hue
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PresetGroupWidget is a widget used on the Preset
 *        Color Groups Page to display the name and all the routine
 *        options for a group group.
 */
class PresetGroupWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief PresetGroupWidget constructor for PresetGroupWidget
     * \param name name of color group
     * \param group enumerated type representation of color group
     * \param colors colors in color group
     * \param parent parent widget
     */
    explicit PresetGroupWidget(QString name,
                               EPalette palette,
                               EPresetWidgetMode mode,
                               QWidget *parent);

    /*!
     * \brief setChecked acts similarly to the setChecked of a standard QPushButton, but
     *        allows you to call the specific button of the PresetGroupWidget by its lighting
     *        routine.
     * \param routine lighting routine button to check or uncheck.
     * \param isChecked true to check button, false otherwise.
     */
    void setChecked(ERoutine routine, bool isChecked);

    /// resize this widget and all subwidgets
    void resize();

signals:

    /*!
     * \brief presetButtonClicked signal that any of the lightsbuttons were pressed and emitting
     *        their own signal.
     */
    void presetButtonClicked(QJsonObject);

private slots:

    /*!
     * \brief multiButtonClicked takes the multiButtonClicked LightsButton signals emitted from each of its buttons
     *        and forwards them as a single signal.
     * \param routine widget's lighting routine
     * \param group widget's color palette
     */
    void multiButtonClicked(QJsonObject object) { emit presetButtonClicked(object); }

private:

    /*!
     * \brief mButtons Buttons used in the grid in the scroll area, each one signals
     *        a EColorPreset and a ELightingMode.
     */
    std::vector<cor::Button *> mButtons;

    /*!
     * \brief mLabel label for name of preset group.
     */
    QLabel *mLabel;

    /*!
     * \brief mLayout layout of widget
     */
    QGridLayout *mLayout;

    /// mode for preset widget
    EPresetWidgetMode mMode;

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
