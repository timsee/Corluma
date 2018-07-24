
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>

#include "cor/page.h"
#include "cor/slider.h"
#include "settingsbutton.h"
#include "cor/webview.h"
#include "globalsettingswidget.h"


/// enum for state of corluma web views, tracks which is shown
enum class ECorlumaWebView {
    FAQ,
    copyright,
    none
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SettingsPage provides a way to configure the
 * application settings.
 *
 * It provides the ability to change the speed the LEDs update
 * and the amount of minutes it takes for the LEDs to timeout.
 *
 * It also provides a way to switch between different communication types.
 * It currently support Serial, HTTP, UDP, and Hue lights. This interface
 * automatically populates serial and Hue connections, but allows users to
 * add and remove connections for UDP and HTTP.
 *
 */
class SettingsPage : public QWidget, public cor::Page
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit SettingsPage(QWidget *parent, GroupsParser *parser, AppSettings *appSettings);

    /*!
     * \brief Destructor
     */
    ~SettingsPage();

    /// called when the widget is shown
    void show();

    /// returns pointer to global widget
    GlobalSettingsWidget *globalWidget() { return mGlobalWidget; }

    /// remove debug options from settings page
    void removeDebug();

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief settingsPageIsStandard signaled when top menu is pressed. True if settings page should
     *        show standard settings, false if showing hue-specific settings.
     */
    void settingsPageIsStandard(bool);

    /*!
     * \brief debugPressed emited when debug button is pressed. Feeds fake communication data into the app
     *        to act as if some devices are connected.
     */
    void debugPressed();

    /*!
     * \brief closePressed emitted when close button in top left is pressed.
     */
    void closePressed();

    /*!
     * \brief clickedInfoWidget emits when hue info widget is clicked
     */
    void clickedInfoWidget();

    /*!
     * \brief clickedDiscovery emits when discovery button is clicked
     */
    void clickedDiscovery();

private slots:
    /*!
     * \brief closeButtonPressed signaled from close button, emits a close signal.
     */
    void closeButtonPressed(bool) { emit closePressed(); }

    /*!
     * \brief settingsButtonPressed a settings button has been pressed
     * \param title the title of the button that was pressed
     */
    void settingsButtonPressed(QString title);

    /// hides whatever webview is showing. If none is showing, this does nothing.
    void hideCurrentWebView();

protected:

    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent *event);

    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     *        color of the settings page so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent *);

private:

    /// groups parser
    GroupsParser *mGroups;

    /// top widget with settings title and close button
    cor::TopWidget *mTopWidget;

    /// scroll area that contains all the information in the widget
    QScrollArea *mScrollArea;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// layout for scoll area
    QVBoxLayout *mScrollLayout;

    /// layout for the widget
    QVBoxLayout *mMainLayout;

    /// widget that contains all the advanced settings that affect the app globally
    GlobalSettingsWidget *mGlobalWidget;

    /// current webview displayed
    ECorlumaWebView mCurrentWebView;

    /*!
     * \brief showWebView show one of the webviews
     * \param newWebView the new webview to show
     */
    void showWebView(ECorlumaWebView newWebView);

    /*!
     * \brief loadButtonClicked loads json data from file and replaces all group
     *         and collection data previously in the app.
     */
    void loadButtonClicked();

    /*!
     * \brief saveButtonClicked saves all json data to an external file.
     */
    void saveButtonClicked();

    /// handles when the settings button pressed is the reset button
    void resetButtonClicked();

    /*!
     * \brief resetToDefaults reset the app to all default settings and
     *        remove all saved memory
     */
    void resetToDefaults();

    /// titles of the sections, such as About, Debug, Data
    std::vector<std::string> mSectionTitles;

    /// titles of specific buttons, such as Load, Copyright, or Reset.
    std::vector<std::string> mTitles;

    /// descriptions of buttons, gives more info on what the button does
    std::vector<std::string> mDescriptions;

    /// labels for sections.
    std::vector<QLabel*> mSectionLabels;

    /// vector of buttons. contains all buttons on the settings page
    std::vector<SettingsButton*> mButtons;

    /// widget displaying copyright information
    cor::WebView *mCopyrightWidget;

    /// widget displaying an FAQ
    cor::WebView *mFAQWidget;

    /// true if showing debug options, false otherwise.
    bool mShowingDebug;
};

#endif // SETTINGSPAGE_H
