#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The SearchWidget class is a widget with three main sections. The top contains an editable
 * field with a plus and minus button. The middle is a list of objects that are currently being
 * searched for. Objects are added to the searching list by clicking the plus button. The bottom is
 * a list of objects that have been discovered or connected. An object cannot be in both the
 * searching list and the connected list at the same time.
 */
class SearchWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit SearchWidget(const QString& defaultLineEditValue,
                          QWidget* parent,
                          int maxSearchingCount = -1,
                          const QString& errorMaxSearchString = QString());

    /*!
     * \brief lineEditText text in line edit
     * \return text in line edit
     */
    const QString lineEditText() { return mLineEdit->text(); }

    /*!
     * \brief addToSearchList add a string to the search list. This string may be modified after
     * entry, if size checks or forceUppercase are turned on. It is then added to the UI and sent to
     * the backend of the application to search for these strings
     *
     * \param key new string to add to UI and send to backend to search for
     * \return true if added successfully.
     */
    bool addToSearchList(const QString& key);

    /*!
     * \brief addToConnectedList programmatically add a name to the connected list.
     *
     * \param name name to add to the list
     * \return true if added successfully.
     */
    bool addToConnectedList(const QString& name);

    /*!
     * \brief removeKey remove the key from the searching list and connected list.
     *
     * \param key key to remove.
     */
    void removeKey(const QString& key);

    /*!
     * \brief enableSizeChecks enable size checks, which makes sure that the string
     * provided to the search widget is within the range of the min and max values provided
     *
     * \param min must be greater than or equal to this value
     * \param max must be lesser than or equal to this value
     * \param error string to give an error about a string being out of range.
     */
    void enableSizeChecks(int min, int max, QString error);

    /// disable size checks
    void disableSizeChecks();

    /*!
     * \brief forceUpperCase true to force strings for searching into upper case, false to use
     * strings as entered.
     *
     * \param shouldForceUpperCase true to force strings for searching into upper case, false to use
     * strings as entered.
     */
    void forceUpperCase(bool shouldForceUpperCase) { mForceUpperCase = shouldForceUpperCase; }

    /// list of all strings that are being searched for.
    std::list<QString> searchingFor();

signals:

    /// emitted when the pressed button is pressed.
    void plusClicked();

    /// emitted when the minus button is pressed.
    void minusClicked();

private slots:

    /*!
     * \brief plusButtonClicked called whenever the plus button is clicked
     */
    void plusButtonClicked();

    /*!
     * \brief minusButtonClicked called whenever the minus button is clicked
     */
    void minusButtonClicked();

    /*!
     * \brief connectedListClicked The connected list was clicked on a discovery page.
     * This allows the user to select one of the connections, but its internal logic
     * is handled differently between different CommTypes.
     */
    void connectedListClicked(QListWidgetItem*);

    /*!
     * \brief discoveringListClicked The discovering list was clicked on a discovery page.
     * This allows the user to select one of the connections, but its internal logic
     * is handled differently between different CommTypes.
     */
    void discoveringListClicked(QListWidgetItem*);

private:
    /// layout for the QLineEdit and QPushButtons used for input
    QHBoxLayout* mInputLayout;

    /// plus button in input layout. adds current string to discovery
    QPushButton* mPlusButton;

    /// minus button for input layout. removes current string from discovery
    QPushButton* mMinusButton;

    /// Displays current IP address and allows user to edit it. Can be added or removed from
    /// discovery with QPushButtons
    QLineEdit* mLineEdit;

    /// label for connected list
    QLabel* mConnectedLabel;

    /// label for discovering list
    QLabel* mDiscoveringLabel;

    /// widget for displaying connected IP addresses
    QListWidget* mConnectedListWidget;

    /// widget for displaying discovering IP addresses
    QListWidget* mDiscoveringListWidget;

    /// layout for widget
    QVBoxLayout* mLayout;

    /// true to force all characters to upper case, false to keep as entered.
    bool mForceUpperCase;

    /// true to check the minimum and maximum size of the string given to the SearchWidget, false to
    /// ignore it
    bool mCheckSize;

    /// minimum size of a string if mCheckSize is enabled, must be this size or greater.
    int mMinSizeCheck;

    /// maximum size of a string if mCheckSize is disabled, must be this size or less.
    int mMaxSizeCheck;

    /// maximum number of strings allowed in the searching field
    int mMaxSearchingCount;

    /// error string for when trying to add more searching strings than possible.
    QString mMaxSearchError;

    /// error string to give as popup if check size is out of range.
    QString mSizeCheckError;
};

#endif // SEARCHWIDGET_H
