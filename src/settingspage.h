
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/objects/page.h"
#include "cor/widgets/slider.h"
#include "cor/widgets/webview.h"
#include "globalsettingswidget.h"
#include "greyoutoverlay.h"
#include "settingsbutton.h"

/// enum for state of corluma web views, tracks which is shown
enum class ECorlumaWebView { FAQ, copyright, none };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The SettingsPage provides a way to configure the application settings and view copyright
 * information.
 *
 * The first set of options relate to connected lights and allow the user to choose to add new
 * connections, or delete data on existing lights. The next set of options relate to the app's save
 * data and can be used to archive the current data or load data from a different version of the
 * app. Due to restrictions caused by discovery methods and license agreements, discovery data is
 * never saved in archivable data. However, groups, rooms, and moods are always saved and can be
 * shared among multiple instances of the app. The next set of pages are pages like a Copyright
 * page, which give necessary copyright info.
 *
 * At the bottom of the settings page is the GlobalSettingsWidget, which allows the user to set some
 * unique app settings. For instance, it currently supports turning on and off specific types of
 * hardware, and turning on and off a timeout.
 */
class SettingsPage : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit SettingsPage(QWidget* parent,
                          AppData* parser,
                          CommLayer* comm,
                          AppSettings* appSettings);

    /// change the height of a row.
    void changeRowHeight(int height) { mRowHeight = height; }

    /// called when the widget is shown
    void showWidget();

    /// called when the widget is hidden
    void hideWidget();

    /// returns pointer to global widget
    GlobalSettingsWidget* globalWidget() { return mGlobalWidget; }

    /// displays the settings page menu
    void pushIn(const QPoint& startPoint, const QPoint& endPoint);

    /// hides the settings page menu
    void pushOut(const QPoint& endPoint);

    /*!
     * \brief enables/disables buttons that require lights to be connected to work. For isntance,
     * the copyright page's button won't ever disable, but the View/Edit Lights button will.
     */
    void enableButtons(bool enable);

signals:

    /*!
     * \brief settingsPageIsStandard signaled when top menu is pressed. True if settings page should
     * show standard settings, false if showing hue-specific settings.
     */
    void settingsPageIsStandard(bool);

    /*!
     * \brief clickedDiscovery emits when discovery button is clicked
     */
    void clickedDiscovery();

    /// emits that debug mode should be used from settings page
    void enableDebugMode();

    /// emits when the add or edit group button is pressed
    void addOrEditGroupPressed();

    /*!
     * \brief clickedLoadJSON emits when the load button is clicked
     * \param path the path to the JSON to load
     */
    void clickedLoadJSON(QString path);

private slots:


    /*!
     * \brief settingsButtonPressed a settings button has been pressed
     *
     * \param title the title of the button that was pressed
     */
    void settingsButtonPressed(const QString& title);

    /// called when the greyout is clicked
    void greyOutClicked();

    /// hides whatever webview is showing. If none is showing, this does nothing.
    void hideCurrentWebView();

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent* event);

    /*!
     * \brief paintEvent paint event for rendering. used to overwrite the background
     * color of the settings page so that it hides everything behind it.
     */
    void paintEvent(QPaintEvent*);

private:
    /// resize everything in the widget
    void resize();

    /// resize the widgets in the scroll area
    void resizeScrollArea();

    /// saved app data, persistent between reloading the app
    AppData* mAppData;

    /// scroll area that contains all the information in the widget
    QScrollArea* mScrollArea;

    /// widget used for scroll area.
    QWidget* mScrollAreaWidget;

    /// widget that contains all the advanced settings that affect the app globally
    GlobalSettingsWidget* mGlobalWidget;

    /// displays the app version on the bottom right of the SettingsPage
    QLabel* mAppVersionLabel;

    /// current webview displayed
    ECorlumaWebView mCurrentWebView;

    /*!
     * \brief showWebView show one of the webviews
     *
     * \param newWebView the new webview to show
     */
    void showWebView(ECorlumaWebView newWebView);

    /*!
     * \brief loadButtonClicked loads json data from file and replaces all group
     * and collection data previously in the app.
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
     * remove all saved memory
     */
    void resetToDefaults();

    /// titles of the sections, such as About, Debug, Data
    std::vector<std::string> mSectionTitles;

    /// labels for sections.
    std::vector<QLabel*> mSectionLabels;

    /// titles of specific buttons, such as Load, Copyright, or Reset.
    std::vector<std::string> mTitles;

    /// vector of buttons. contains all buttons on the settings page
    std::vector<SettingsButton*> mButtons;

    /// widget displaying copyright information
    cor::WebView* mCopyrightWidget;

    /// pointer to comm layer
    CommLayer* mComm;

    /// widget for greying out the page when other widgets are overlaid
    GreyOutOverlay* mGreyOut;

    /// height of each row
    int mRowHeight;
};

#endif // SETTINGSPAGE_H
