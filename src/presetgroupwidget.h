#ifndef PRESETGROUPWIDGET_H
#define PRESETGROUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include "corlumabutton.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
                               EColorGroup group,
                               const std::vector<QColor>& colors,
                               QWidget *parent = 0);

    /*!
     * \brief setChecked acts similarly to the setChecked of a standard QPushButton, but
     *        allows you to call the specific button of the PresetGroupWidget by its lighting
     *        routine.
     * \param routine lighting routine button to check or uncheck.
     * \param isChecked true to check button, false otherwise.
     */
    void setChecked(ELightingRoutine routine, bool isChecked);

signals:

    /*!
     * \brief presetButtonClicked signal that any of the lightsbuttons were pressed and emitting
     *        their own signal.
     */
    void presetButtonClicked(int, int);

private slots:

    /*!
     * \brief multiButtonClicked takes the multiButtonClicked LightsButton signals emitted from each of its buttons
     *        and forwards them as a single signal.
     * \param routine int representation of ELightingRoutine
     * \param group int representation of EColorGroup
     */
    void multiButtonClicked(int routine, int group) { emit presetButtonClicked(routine, group); }

private:
    /*!
     * \brief mButtons Buttons used in the grid in the scroll area, each one signals
     *        a EColorPreset and a ELightingMode.
     */
    std::vector<CorlumaButton *> mButtons;

    /*!
     * \brief mLabel label for name of preset group.
     */
    QLabel *mLabel;

    /*!
     * \brief mLayout layout of widget
     */
    QHBoxLayout *mLayout;

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
