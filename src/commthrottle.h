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
     */
    void startThrottle(int interval);

    /*!
     * \brief checkThrottle true if the throttle is not set and a packet can be sent, false otherwise.
     * \return true if the throttle is not set and a packet can be sent, false otherwise.
     */
    bool checkThrottle(QString packet);

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
    void sendThrottleBuffer(QString);

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
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

};

#endif // COMMTHROTTLE_H
