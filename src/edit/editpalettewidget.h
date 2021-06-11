#ifndef EDITPALETTEWIDGET_H
#define EDITPALETTEWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include "colorpicker/singlecolorpicker.h"
#include "colorpicker/swatchvectorwidget.h"
#include "cor/objects/page.h"
#include "cor/objects/palette.h"
#include "cor/widgets/palettecolorpicker.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The EditPaletteWidget class is a widget that allows a user to define a custom palette and
 * save it to app data.
 */
class EditPaletteWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit EditPaletteWidget(QWidget* parent);

    /// load an existing palette for editing
    void loadPalette(const cor::Palette& palette);

    /// changes the row height of rows in scroll areas.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// pushes in the widget
    void pushIn();

    /// pushes out the widget
    void pushOut();

    /// resize programmatically
    void resize();

signals:

    /// emits when the save button is pressed
    void savePalette(cor::Palette);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);
    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent*) { resize(); }

private slots:
    /// catches when the line edit changes
    void lineEditChanged(QString);

    /// handles when the add button is clicked
    void addButtonPressed(bool);

    /// handles when the remove button is clicked
    void removeButtonPressed(bool);

    /// handles when color is changed
    void colorChanged(const QColor&);

    /// handles when the save button is clicked, saves the palette
    void saveButtonPresed(bool);

    /// changes the current color selection.
    void changedSelection(QColor, std::uint32_t);

private:
    /// change the count of colors currently in the palette
    void changeColorCount(bool addOne);

    /// picker for choosing a single color
    SingleColorPicker* mColorPicker;

    /// widget that allows you to choose individual colors in the palette
    PaletteColorPicker* mPaletteColors;

    /// button for adding a color to the palette
    QPushButton* mAddButton;

    /// button for removing a color from the palette
    QPushButton* mRemoveButton;

    /// line edit for the name
    QLineEdit* mNameInput;

    /// button to save a palette
    QPushButton* mSaveButton;

    /// the currently displayed palette
    cor::Palette mPalette;

    /// the height of a row in a scroll area
    int mRowHeight;
};

#endif // EDITPALETTEWIDGET_H
