#ifndef LIGHTINGPAGE_H
#define LIGHTINGPAGE_H

#include "commlayer.h"
#include "datalayer.h"
#include <memory>

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
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
     * \param commLayer the object that handles the commmunication to the LED array.
     * \param dataLayer the object that handles storing data about the application
     *                  and the LED array's state.
     */
    void setup(CommLayer *commLayer,
               DataLayer *dataLayer) {
        mComm = commLayer;
        mData = dataLayer;
    }

protected:
    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DataLayer *mData;

};


#endif // LIGHTINGPAGE_H
