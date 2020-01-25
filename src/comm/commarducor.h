#ifndef COMMARDUCOR_H
#define COMMARDUCOR_H

#include <QObject>
#include <memory>

#include "comm/arducor/arducordiscovery.h"
#include "comm/arducor/arducormetadata.h"
#include "comm/arducor/crccalculator.h"
#include "comm/commtype.h"
#include "cor/presetpalettes.h"

class CommUDP;
class CommHTTP;
class CommSerial;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommArduCor class wraps CommHTTP, CommUDP, and CommSerial into
 *        one object designed for communicating with ArduCor arduino samples.
 *        This object handles the discovery of both new and previously discovered
 *        arduinos, and handles sending packets and receiving packets from arduinos.
 */
class CommArduCor : public QObject {
    Q_OBJECT
public:
    /// constructor
    explicit CommArduCor(QObject* parent);

    /*!
     * \brief sendPacket sends a packet to the given controller
     * \param controller the controller to send a packet to
     * \param payload the payload to send to the controller
     */
    void sendPacket(const cor::Controller& controller, QString& payload);

    /// startup the arducor streams
    void startup();

    /// shutdown the arducor streams
    void shutdown();

    /// stop state updates
    void stopStateUpdates();

    /// restart state updates
    void resetStateUpdates();

    /// deletes light from device table and triggers deleting it from discovery data
    void deleteLight(const cor::Light& light);

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType* commByType(ECommType type);

#ifndef MOBILE_BUILD
    /// getter for serial
    CommSerial* serial() { return mSerial.get(); }
#endif // MOBILE_BUILD

    /// getter for UDP
    CommUDP* UDP() { return mUDP.get(); }

    /// getter for discovery
    ArduCorDiscovery* discovery() { return mDiscovery; }

    /// getter for all connected lights
    std::vector<cor::Light> lights();

    /// getter for arducor lights
    std::vector<ArduCorMetadata> arduCorLights() const noexcept { return mArduCorLights.items(); }

    /// converts a cor::Light into a arducor light
    ArduCorMetadata arduCorLightFromLight(const cor::Light& light);

signals:

    /*!
     * \brief packetReceived anotification that a packet was receieved by one of the commtypes.
     */
    void packetReceived(EProtocolType);

    /*!
     * \brief updateReceived an update packet was received from any controller.
     */
    void updateReceived(ECommType);

private slots:

    /// forwards the update packets from individual commtypes to commlayer
    void receivedUpdate(ECommType type) { emit updateReceived(type); }

    /*!
     * \brief parsePacket parses any packets sent from any of the commtypes. The
     *        comm type that received the packet is given as an int
     */
    void parsePacket(const QString&, const QString&, ECommType);

private:
#ifndef MOBILE_BUILD
    /*!
     * \brief mSerial Serial connection object
     */
    std::shared_ptr<CommSerial> mSerial;
#endif // MOBILE_BUILD
    /*!
     * \brief mHTTP HTTP connection object
     */
    std::shared_ptr<CommHTTP> mHTTP;
    /*!
     * \brief mUDP UDP connection object
     */
    std::shared_ptr<CommUDP> mUDP;

    /// arducor discovery ovbject
    ArduCorDiscovery* mDiscovery;

    /// dictionary of ArduCor lights
    cor::Dictionary<ArduCorMetadata> mArduCorLights;

    /*!
     * \brief preparePacketForTransmission prepare internal states and variables for sending a new
     * packet and adjust packet
     *        to fix issues with sending it, if needed
     * \param controller the controller to send the packet the to
     * \param packet the packet that is about to be sent
     * \return true to reset state timeout, false to do nothing
     */
    bool preparePacketForTransmission(const cor::Controller& controller, QString& packet);

    /*!
     * \brief verifyStateUpdatePacketValidity takes a vector and checks that all
     *        values are within the proper range. Returns true if the packet can
     *        be used.
     * \param packetIntVector The packet that is going to receive testing.
     * \param x starting index in the vector, if it contains multiple lights.
     * \return true if all values in the vector are in the proper range, false othewrise.
     */
    bool verifyStateUpdatePacketValidity(const std::vector<int>& packetIntVector, uint32_t x);

    /*!
     * \brief verifyCustomColorUpdatePacket takes a vector and checks that all vlaues
     *        are wirthing the proper range for a custom color update. Returns true if the packet
     *        can be used.
     * \param packetIntVector The packet for testing
     * \return true if its a valid packet, false othwerise.
     */
    bool verifyCustomColorUpdatePacket(const std::vector<int>& packetIntVector);

    /// used to check CRC on incoming packets.
    CRCCalculator mCRC;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;
};

#endif // COMMARDUCOR_H
