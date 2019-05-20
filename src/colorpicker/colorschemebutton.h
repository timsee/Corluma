#ifndef COLORSCHEMEBUTTON_H
#define COLORSCHEMEBUTTON_H

#include <QLabel>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/// used to determine state of colorschemebutton
enum class EColorSchemeButtonState { deselected, selected };

/// used to determine current color scheme type.
enum class EColorSchemeType { custom, similar, complement, triad, compound, MAX };
Q_DECLARE_METATYPE(EColorSchemeType)

/// converts EColorSchemeType enum to string
QString colorSchemeTypeToString(EColorSchemeType e);

/// converts string to EColorSchemeType enum
EColorSchemeType stringToColorSchemeType(const QString&);

/*!
 * \brief The ColorSchemeButton class is a simple button for displaying a
 *        a color scheme type.
 */
class ColorSchemeButton : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit ColorSchemeButton(EColorSchemeType, const QString& imagePath, QWidget* parent);

    /// selects and deselects button
    void select(bool shouldSelect);

    /// enables and disabled button
    void enable(bool enable);

    /// key for group
    EColorSchemeType key() const { return stringToColorSchemeType(mTitle->text()); }

    /// resizes widget programmatically
    void resize();

signals:

    /// signals the type of color scheme
    void clicked(EColorSchemeType, bool);

protected:
    /// resize the widget
    virtual void resizeEvent(QResizeEvent*);

    /*!
     * \brief mouseReleaseEvent called when a mouse press is released Events not
     *        directly on top of the color wheel are ignored.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

    /// renders the widget
    virtual void paintEvent(QPaintEvent*);

private slots:

    /// picks up when the select all button is pressed
    void buttonPressed(bool);

private:
    /// state of the button
    EColorSchemeButtonState mState;

    /// true if enabled, false otherwise
    bool mEnabled;

    /// label for checkbox
    QLabel* mTitle;

    /*!
     * \brief mButton button that selects all devices when pushed and adds them to the data layer.
     */
    QLabel* mButton;

    /// path to resource file for icon
    QString mResourcePath;

    /// pixmap for icon
    QPixmap mPixmap;
};

#endif // COLORSCHEMEBUTTON_H
