#ifndef LIGHTINGPAGE_H
#define LIGHTINGPAGE_H

#include <QTimer>

#include <memory>

#include "datalayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief Inherited by Lighting Pages, provides a link to the backend.
 */
class LightingPage {

public:

    /*!
     * \brief ~LightingPage Deconstructor
     */
    virtual ~LightingPage(){}

    /*!
     * \brief setup called by the MainWindow after the commLayer and dataLayer
     *        of the application are set up. This connects these layers to
     *        all the other pages.
     * \param dataLayer the object that handles storing data about the application
     *                  and the LED array's state.
     */
    void setup(DataLayer *dataLayer) {
        mData = dataLayer;
    }

protected:


    /*!
     * \brief mRenderThread timer that calls a renderUI() function on each of the pages.
     *        This function is used to render more expensive assets if and only if they
     *        received a change in state since the previous renderUI call.
     */
    QTimer *mRenderThread;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DataLayer *mData;

    /*!
     * \brief mRenderInterval number of msec between each of the UI events.
     */
    int mRenderInterval = 100;
};


#endif // LIGHTINGPAGE_H
