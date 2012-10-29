#ifndef _I_PACKET_HANDLER
  #define _I_PACKET_HANDLER

  #include <CSerializer.h>

/**
 * @class ICCommandVerify
 * create an implementation of this interface if you want to verify ODE commands sent from a client to a server
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class ICommandVerify {
  public:
    /**
     * This method will be called whenever an ODE command is received from a client. Implement it in order to verify the command
     * @return true if the command is valid and should be executed, false otherwise
     */
    virtual bool validCommand(u16 iPlayerId, u16 iCommand, u16 iBodyId)=0;
};

/**
 * Create an implementation of this interface if you want to handle the raw data received over the network
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class IPacketHandler {
  public:
    /**
     * This method is called whenever a packet is received over the network. Be aware that this should not be the normal
     * way to handle communication in IrrOdeNet. Create a descendant of IIrrOdeEvent for each message you want to handle,
     * create your message factory and the messages will automatically be decoded and send throught the IrrOde message bus.
     * @param pPacket the data
     * @param iPlayerId the client id the packet was received from
     */
    virtual void handlePacket(irr::ode::CSerializer *pPacket, u16 iPlayerId)=0;
};

/**
 * Implement this interface to handle connection changes
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class IConnectionHandler {
  public:
    /**
     * This method gets called whenever a player tries to connect
     * @param iPlayerId the internal id of the new client
     */
    virtual void onPeerConnect(u16 iPlayerId)=0;
    /**
     * This method gets called whenever a client disconnects
     * @param iPlayerId the internal id of the disconnected client
     */
    virtual void onPeerDisconnect(u16 iPlayerId)=0;
};

#endif
