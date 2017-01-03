#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include <QWidget>

namespace Ui {
class DiscoveryPage;
}

/*!
 * \brief The DiscoveryPage class An unimplemented page. Will be used to walk a user through
 *        the discovery steps for their given platform.
 */
class DiscoveryPage : public QWidget
{
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit DiscoveryPage(QWidget *parent = 0);

    /*!
     * Deconstructor
     */
    ~DiscoveryPage();

private:
    Ui::DiscoveryPage *ui;
};

#endif // DISCOVERYPAGE_H
