#ifndef DATASYNC_H
#define DATASYNC_H

#include <QObject>
#include <QTimer>

#include "datalayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */


class CommLayer;

/*!
 * \brief The DataSync class compares the data layer's representation of devices with the commlayer's
 *        understanding of devices and tries to sync them up. The DataLayer's representation is used
 *        as the "desired" state of lights. The CommLayer's understanding is used as the current state.
 *        If the desired state and current state do not match, the commlayer is requested to send packets
 *        to try to update the devices.
 */
class DataSync : public QObject
{
    Q_OBJECT
public:

    /*!
     * \brief DataSync Constructor for DataSync.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSync(DataLayer *data, CommLayer *comm);

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync();

public slots:
    /*!
     * \brief resetSync Tells the DataSync object that the commlayer and the datalayer are potentially
     *        no longer in sync and the syncData() function needs to get called on the timer again.
     */
    void resetSync();

private slots:
    /*!
     * \brief syncData called by the SyncTimer. Runs the sync routine, which checks
     *        the data layer's desired representation of devices against the comm layers
     *        understanding of the current state of devices. Sends packets through the commlayer
     *        to try to bring them mmore in sync.
     */
    void syncData();

private:

    /*!
     * \brief mData pointer to data layer. Used for checking what state the data layer
     *        desires the devices to be.
     */
    DataLayer *mData;

    /*!
     * \brief mComm pointer to comm layer. Used for checking what state the comm layer
     *        thinks the devices are in and for sending packets to the devices.
     */
    CommLayer *mComm;

    /*!
     * \brief standardSync checks if the light device of a comm layer and a data layer are in sync.
     * \param dataDevice device from the data layer
     * \param commDevice device from the comm layer
     * \return true if they match, false otherwise
     */
    bool standardSync(const SLightDevice& dataDevice, const SLightDevice& commDevice);

    /*!
     * \brief hueSync checks if the light device of a comm layer and a data layer are in sync. Handles
     *        hue devices as they have some unique issues, such as syncing color temperature.
     * \param dataDevice device from the data layer
     * \param commDevice device from the comm layer
     * \return true if they match, false otherwise
     */
    bool hueSync(const SLightDevice& dataDevice, const SLightDevice& commDevice);

    /*!
     * \brief colorDifference gives a percent difference between two colors
     * \param first first color to check
     * \param second second color to check
     * \return a value between 0 and 1 which is how different two colors are.
     */
    float colorDifference(QColor first, QColor second);

    /*!
     * \brief brightnessDifference gives a percent difference between two brightness
     *        values
     * \param first first brightness to check
     * \param second second brightness to check
     * \return a value between 0 and 1 which represents how different the brightnesses are.
     */
    float brightnessDifference(float first, float second);

    /*!
     * \brief ctDifference gives a percent difference between two color temperatures. This
     *        function currently assumes that only Hue lightss are using it and will need
     *        adjustment for other lights that use a different range of color temperatures.
     * \param first first color temperature to check.
     * \param second second color temperature to check.
     * \return a value between 0 and 1 which represents how different the temperatures are.
     */
    float ctDifference(float first, float second);

    /*!
     * \brief mSyncTimer Whenever data is not in sync,
     */
    QTimer *mSyncTimer;

    /*!
     * \brief mStartTime time that sync thread was started so in case something groes wrong it
     *        can still timeout after attempting to sync for a reasonable amount of time.
     */
    QTime mStartTime;

    /*!
     * \brief mDataIsInSync true if all the datalayer and commlayer representations of devices are identical,
     *        false otherwise.
     */
    bool mDataIsInSync;
};

#endif // DATASYNC_H
