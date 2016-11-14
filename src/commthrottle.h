#ifndef COMMTHROTTLE_H
#define COMMTHROTTLE_H

#include <QTimer>
#include <QTime>
#include <QWidget>


class CommThrottle : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    CommThrottle(QWidget *parent = 0);

    /*!
     * \brief startThrottle start up the throttle.
     * \param interval the minimum number of msec between each throttle reset.
     * \param throttleMax maximum number of packets that can be sent between two resets
     */
    void startThrottle(int interval, int throttleMax);

    /*!
     * \brief checkThrottle true if the throttle is not set and a packet can be sent, false otherwise.
     * \return true if the throttle is not set and a packet can be sent, false otherwise.
     */
    bool checkThrottle(QString controller, QString packet);

    /*!
     * \brief checkLastUpdate check how many msec its been since the the last update to the throttle
     * \return number of msec its been since the the last update to the throttle
     */
    int checkLastUpdate() { return mLastUpdate.elapsed(); }

    /*!
     * \brief receivedUpdate should be called each time a message is received from the communication stream.
     */
    void receivedUpdate() { mLastUpdate.restart(); }

    /*!
     * \brief checkLastSend check how long its been since a command was sent
     * \return the number of msec its been since a command was sent.
     */
    int checkLastSend() { return mLastSend.elapsed(); }

    /*!
     * \brief sentPacket call this for all packets we do not ignore. Currently, the only ignored packets
     *        are state update packets.
     */
    void sentPacket() { mLastSend.restart(); }

signals:

    /*!
     * \brief sendThrottleBuffer requests that this QString gets sent by the communication stream.
     */
    void sendThrottleBuffer(QString, QString);

private slots:

    /*!
     * \brief resetThrottleFlag resets the flag being used to throttle
     *        certain commtypes such as HTTP, which require slower updates
     *        to avoid clogging their data stream.
     */
    void resetThrottleFlag();

private:

    /*!
     * \brief mThrottleTimer Used to slow down how quickly packets are sent to the
     *        the current controller.
     */
    QTimer *mThrottleTimer;
    /*!
     * \brief mBufferedMessage the packet that is currently in the buffer.
     *        This gets overriden by new packets and by more recent packets
     *        being sent to the controller.
     */
    QString mBufferedMessage;

    /*!
     * \brief mBufferedController name of controller being buffered for next packet.
     */
    QString mBufferedController;

    /*!
     * \brief mBufferedTime amount of time the buffer has contained a packet.
     */
    QTime mBufferedTime;
    /*!
     * \brief mLastThrottleCall elapsed time since the last throttle command was called.
     */
    QTime mLastThrottleCall;

    /*!
     * \brief mLastUpdate elapsed time since the last update happened.
     */
    QTime mLastUpdate;

    /*!
     * \brief mLastSend elapse time since the last non state-update packet was sent.
     */
    QTime mLastSend;

    /*!
     * \brief mShouldSendBuffer true if the buffered packet should send, false otherwise.
     */
    bool mShouldSendBuffer;

    /*!
     * \brief mThrottleInterval set when commtypes are changed, this tracks how many milliseconds
     *        mininmum should be between each packet for this communication stream.
     */
    int mThrottleInterval;

    /*!
     * \brief mThrottleCount count of number of times something is called in the time of an interval before the
     *        thrtottle resets. If this reaches mThrottleMax, packets start to get buffered.
     */
    int mThrottleCount;

    /*!
     * \brief mThrottleMax total number of times a packet can be sent in the timespawn of one interval. To allow
     *        only one packet, set this to 1.
     */
    int mThrottleMax;
};

#endif // COMMTHROTTLE_H
