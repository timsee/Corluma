
#ifndef COMMLAYER_H
#define COMMLAYER_H

#include <QColor>
#include <QWidget>
#include <memory>

#include "comm/arducor/arducordiscovery.h"
#include "comm/commtype.h"
#include "comm/hue/huemetadata.h"
#include "comm/hue/hueprotocols.h"
#include "comm/upnpdiscovery.h"
#include "cor/objects/light.h"
#include "cor/presetpalettes.h"
#include "cor/protocols.h"
#include "data/groupdata.h"

#include "colorpicker/colorpicker.h"

class CommArduCor;
class CommHue;
class CommNanoleaf;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommLayer class provides communication protocols that allow the user to connect and
 * send packets to a light. Each light protocol type has its own class for maintaining communciation
 * with lights of that type, and all of those classes are wrapped by CommLayer and given a
 * standardized API. Each protocol also has its own discovery class, which handles <i>just</i>
 * discovery of the lights, and storage of that discovery data.
 *
 * CommLayer is important in generalizing the divergent APIs of lights. Even though some lights may
 * use RGB, some may use HSV, and some may not even support colors, CommLayer gives them all the
 * same high level API, and allows the developer to access more specific fucntions, if necessary.
 *
 * CommLayer also stores the state of the lights, based off of packets it receives. Each light can
 * be queryed by either a cor::Light object or its unique ID. When a user decides to change the
 * state of the light, the CommLayer's stored state is queryed and packets keep getting sent until
 * the CommLayer's state matches the user's desired state.
 *
 * A UPnP discovery object is also wrapped by CommLayer, which is used to discover lights that give
 * their discovery information over UPnP. Since UPnP requires binding to a socket, all discovery
 * objects share the same UPnPDiscovery object, and subscribe/unsubscribe to listening to it.
 */
class CommLayer : public QObject {
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    CommLayer(QObject* parent, GroupData* groups);

    /*!
     * \brief resetStateUpdates reset the state updates timeouts for specified commtypes. If it
     * isn't on already, it gets turned on.
     */
    void resetStateUpdates(EProtocolType type);

    /*!
     * \brief stopStateUpdates turn off the state update threads for specified commtypes.
     */
    void stopStateUpdates(EProtocolType type);

    // --------------------------
    // Controller and Device Management
    // --------------------------

    /*!
     * \brief startup start stream of given type. This starts all of its maintence threads
     *        and discovery threads
     * \param type the type of communication stream to start.
     */
    void startup(EProtocolType type);

    /*!
     * \brief shutdown shuts down the stream of the given type. This stops all of its maintence
     * threads and discovery threads.
     * \param type the type of communication stream to shutdown.
     */
    void shutdown(EProtocolType type);

    /// removes light from saved data, returns true if successful
    bool removeLight(const cor::Light& light);

    /*!
     * \brief fillLight use the controller name, type, and index to fill in the rest
     *        of the devices data.
     * \param device the device with a defined name, type, and index
     * \return true if controller is found and filled, false otherwise.
     */
    bool fillLight(cor::Light& light);

    /*!
     * \brief startDiscovery put given comm type into discovery mode.
     */
    void startDiscovery(EProtocolType type);

    /*!
     * \brief stopDiscovery stop the given comm type's discovery mode.
     */
    void stopDiscovery(EProtocolType type);

    /// returns true if theres any known errors for the commtype.
    bool discoveryErrorsExist(EProtocolType type);

    /*!
     * \brief deviceTable dictionary of all connected devices of a certain connection type. The
     * light names are used as keys.
     * \param type the communication type to request.
     * \return a hash table of all connected devices of the given type.
     */
    const cor::Dictionary<cor::Light>& lightDict(ECommType type) const {
        return commByType(type)->lightDict();
    }

    /// looks up a light by its unique ID and returns its metadata and current state
    cor::Light lightByID(const QString& ID) const;

    /// converts a vector of unique IDs to a vector cor::Lights
    std::vector<cor::Light> lightsByIDs(const std::vector<QString> IDs) const {
        std::vector<cor::Light> lights;
        for (const auto& ID : IDs) {
            lights.push_back(lightByID(ID));
        }
        return lights;
    }

    /// takes the unique ID's of the lights provided, and returns their state according to comm
    /// updates
    std::vector<cor::Light> commLightsFromVector(const std::vector<cor::Light>& lights);

    /// creates a list of lights and their current state based off of a group.
    std::vector<cor::Light> lightListFromGroup(const cor::Group& group);

    /*!
     * \brief saves a for future use. For lights like hues, that can save groups to their bridge,
     * the group is saved externally. For lights without this capability, the Group is saved to the
     * @ref GroupData provided to the constructor.
     * \param group the group to save to external storage or to group data, when applicable.
     * \return true if successful, false if encounters error.
     */
    bool saveNewGroup(const cor::Group& group);

    /// makes a dictionary of lights based off of the formula provided by a mood object.
    cor::Dictionary<cor::Light> makeMood(const cor::Mood& mood);

    /// list of all devices from all comm types
    std::vector<cor::Light> allLights();

    /// list containing all arduino based cor::Controllers
    const std::vector<cor::Controller> allArduinoControllers();

    /// getter for nanoleaf
    std::shared_ptr<CommNanoleaf> nanoleaf() { return mNanoleaf; }

    /// getter for arducor
    std::shared_ptr<CommArduCor> arducor() { return mArduCor; }

    // --------------------------
    // Hardware specific functions
    // --------------------------

    /// pointer to the hue comm type
    std::shared_ptr<CommHue> hue() { return mHue; }

    /*!
     * \brief hueLightsToDevices helper to convert a list of hue lights into a list of cor::Lights
     * \param hues list of hue lights to convert
     * \return a list of cor::Lights
     */
    std::vector<cor::Light> hueLightsToDevices(std::vector<HueMetadata> hues);

    /// deletes a hue group by name
    void deleteHueGroup(const QString& name);

    /// computes the best possible color picker type that can be supported based off of the
    /// currently selected lights
    EColorPickerType bestColorPickerType(const std::vector<cor::Light>& lights);

    /*!
     * \brief lightNameChange change the name of a widget given the key and the new name.
     *
     * \param key the key for the light
     * \param name the new name for the light.
     */
    void lightNameChange(const QString& key, const QString& name);

    /// delete a light by unique ID. This deletes the light from memory.
    void deleteLight(const QString& uniqueID);

    /// gives the last time any specific CommType last received an update
    QTime lastUpdateTime();

    /// adds meta data to a light, used when creating moods.
    cor::Light addLightMetaData(cor::Light light);

    /// fills a mood with its lights metadata, as well as the metadata for its defaults. Pulls from
    /// GroupData and CommLayer when necessary.
    cor::Mood addMetadataToMood(const cor::Mood& mood);

signals:

    /*!
     * \brief packetReceived anotification that a packet was receieved by one of the commtypes.
     */
    void packetReceived(EProtocolType);

    /*!
     * \brief updateReceived a notification that a packet was received by one of the commtypes.
     * \param type the int representation of the ECommType that has been updated.
     */
    void updateReceived(ECommType);

    /// signals when a new light is found
    void newLightFound(ECommType, QString);

    /// signals when an existing light is deleted
    void lightDeleted(ECommType, QString);

private slots:

    /// forwards slots from internal connection objects to anything listening to CommLayer
    void receivedPacket(EProtocolType type) { emit packetReceived(type); }

    /*!
     * \brief receivedUpdate Each CommType signals out where it receives an update. This slot
     * combines and forwards these signals into its own updateReceived signal.
     * \param type the ECommType that has been updated.
     */
    void receivedUpdate(ECommType type) { emit updateReceived(type); }

    /*!
     * \brief hueStateChanged sent by hue whenever a packet is received that changes it state.
     */
    void hueStateChanged() { emit packetReceived(EProtocolType::hue); }

    /// handles when new light is found
    void lightFound(ECommType, QString);

    /// handles when existing light is deleted
    void deletedLight(ECommType, QString);

private:
    /*!
     * \brief mArduCor ArudCor connection object
     */
    std::shared_ptr<CommArduCor> mArduCor;

    /*!
     * \brief mHue Philips Hue connection object
     */
    std::shared_ptr<CommHue> mHue;

    /*!
     * \brief mNanoleaf Nanoleaf Aurora connection object
     */
    std::shared_ptr<CommNanoleaf> mNanoleaf;

    /// Handles discovery of devices over UPnP
    UPnPDiscovery* mUPnP;

    /// groups parser
    GroupData* mGroups;

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType* commByType(ECommType type) const;
};

#endif // COMMLAYER_H
