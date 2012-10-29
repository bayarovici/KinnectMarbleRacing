#ifndef _C_IRR_ODE_NET_ADAPTER
  #define _C_IRR_ODE_NET_ADAPTER

  #include <irrlicht.h>
  #include <INetManager.h>
  #include <IrrOdeNet/CMarbles2WorldObserver.h>
  #include <event/IIrrOdeEvent.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace net;

  #include <CPacketDistributor.h>

/**
 * This class is internal. It is used to store client data
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class CIrrOdePeer {
  protected:
    void *m_pData;    /**< data that can be stored for a peer */
    u16 m_iPlayerId,  /**< the internal id of the client this object belongs to */
        m_iPing;      /**< ping of the client */

  public:
    /**
     * constructor
     * @param iPlayerId the internal id of the client
     */
    CIrrOdePeer(u16 iPlayerId) { m_iPlayerId=iPlayerId; m_iPing=0; m_pData=NULL; }

    /**
     * set/get an user defined object
     */
    void setData(void *pData) { m_pData=pData; }
    void *getData() { return m_pData; }

    /**
     * set/get the ping of this peer
     */
    u16 getPing() { return m_iPing; }
    void setPing(u16 iPing) { m_iPing=iPing; }

    /**
     *get the internal id of the client this object belongs to
     */
    u16 getPlayerId() { return m_iPlayerId; }
};

/**
 * This is the central class that manages the network connection stuff
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class CIrrOdeNetAdapter : public INetCallback, public CPacketDistributor, public IIrrOdeWorldListener {
  protected:
    INetManager *m_pNetManager; /**< the irrodenet lite net manager */
    ISceneManager *m_pSmgr;     /**< the irrlicht scene manager */

    list<CIrrOdePeer *> m_lPeers; /**< the connected peers */

    bool m_bIsServer,         /**< is this object a server? */
         m_bInitialized;      /**< is the network code initialized? */
    u32 m_iTime,              /**< the local time */
        m_iPing,              /**< the ping ("0" for servers) */
        m_iLastPing,          /**< the timestamp of the last ping (the server gets pinged once per second by all clients) */
        m_iLastStep,          /**< the last step time */
        m_iMaxClients,        /**< the maximum number of clients that can connect (to a server) */
        m_iSentCnt,           /**< the number of sent packets */
        m_iRecvCnt,           /**< the number of received packets */
        m_iSentSize,          /**< the number of bytes sent */
        m_iRecvSize,          /**< the number of bytes received */
        m_iTrafficStartTime;  /**< the start time of the traffic measurement */

    ITimer *m_pTimer;   /**< the irrlicht timer */

    ICommandVerify *m_pCommandVerify;   /**< the command verify object */

    void setPing(u16 iPlayerId, u16 iPing);   /**< update the "ping" member of a CIrrOdePeer object */

    bool isValid(u16 iPlayerId, u16 iCommand, u16 iBodyId);   /**< validatation check for an ODE command */
    /**
     * protected constructor for singleton
     */
    CIrrOdeNetAdapter();

  public:
    /**
     * get the shared singleton instance
     * @return the shared singleton instance
     */
    static CIrrOdeNetAdapter *getSharedInstance();

    /**
     * set the timer
     * @param pTimer the timer
     */
    void setTimer(ITimer *pTimer);
    /**
     * set the used scene manager
     * @param pSmgr the used scene manager
     */
    void setSceneManager(ISceneManager *pSmgr);

    /**
     * Irrnet lite callback to handle packets
     * @param packet the packet to handle
     */
    virtual void handlePacket(SInPacket& packet);
    /**
     * irrnet lite callback to handle connection requests
     * @param iPlayerId the internal id of the newly connected peer
     */
    virtual void onConnect(const u16 playerId);
    /**
     * irrnet lite callback to handle disconnections
     * @param playerId the internal id of the disconnected client
     */
    virtual void onDisconnect(const u16 playerId);

    /**
     * call this method once per frame
     */
    void update();

    /**
     * is the object initialized?
     * @return true if the network code is initialized, false otherwise
     */
    bool isInitialized();

    /**
     * send a packet to a specific client (server only)
     * @param pEvent the IIrrOdeEvent that should be serialized and sent
     * @param iPlayerId the receiver of the event
     */
    void sendPacket(IIrrOdeEvent *pEvent, u16 iPlayerId);
    /**
     * send a packet to all connected clients (for servers) or to the server (for clients)
     * @param pEvent the IrrOde event to be serialized and sent
     */
    void sendPacket(IIrrOdeEvent *pEvent);

    /**
     * send a raw packet to a specific client (servers only)
     * @param pBuffer the data to be sent
     * @param iPlayerId internal id of the receiver
     */
    void sendPacketUnreliable(CSerializer *pBuffer, u16 iPlayerId);
    /**
     * send a raw packet to all connected clients (for server) or to the server (for clients)
     * @param pBuffer the data to be sent
     */
    void sendPacketUnreliable(CSerializer *pBuffer);

    /**
     * create a server
     * @param iPortNo the port number to listen to
     * @param iUpBandwidth the upload bandwidth
     * @param iDownBandwidth the download bandwidth
     * @param iMaxClients the maximum number of clients
     * @return "true" if the server was started, "false" otherwise
     */
    bool createServer(u32 iPortNo, u32 iUpBandwidth, u32 iDownBandwidth, u32 iMaxClients);
    /**
     * create a client
     * @param sServer ip address of the server to connect to
     * @param iPortNo the port number to connect to
     * @param iUpBandwidth the upload bandwidth
     * @param iDownBandwidth the download bandwidth
     * @param iTimeout the timeout value
     */
    bool createClient(const char *sServer, u32 iPortNo, u32 iUpBandwidth, u32 iDownBandwidth, u32 iTimeout);

    /**
     * close a started server or client
     */
    void close();

    /**
     * assign some app specific data to a peer
     * @param iPlayerId the internal id of the client
     * @param pData the data to assign
     */
    void setPeerData(u16 iPlayerId, void *pData);
    /**
     * get the app specific data assigned to a peer
     * @param iPlayerId the internal id of the client
     * @return the data assigned to the peer, "NULL" if no data was assigned or found
     */
    void *getPeerData(u16 iPlayerId);
    /**
     * get the app specific data by the internal index number
     * @param iIdx the internal index
     * @return the data assigned to the peer, "NULL" if no data was assigned or found
     */
    void *getPeerDataByIndex(u16 iIdx);
    /**
     * get the internal id of the peer at a specific array index
     * @param iIdx
     * @return the internal id of the peer, "0" if no internal id was found
     * @see getPlayerIdByIndex
     */
    u16 getIdOfPeerAtIndex(u16 iIdx);
    /**
     * get the internal array index of an internal player id
     * @param iPlayerId the internal player id
     * @return the internal array index of the peer
     * @see getNumOfPeer
     */
    u16 getIndexOfPeer(u16 iPlayerId);
    /**
     * get the ping of a peer
     * @param iPlayerId the internal client id
     * @return the ping of the client, "0" if an invalid client id was passed
     */
    u16 getPing(u16 iPlayerId);

    /**
     * is this a server?
     * @return "true" if this is a server, "false" otherwise
     */
    bool isServer() { return m_bIsServer; }

    /**
     * get the number of peers connected to the server
     * @return number of peers connected to the server
     */
    u32 getPeerCount();
    /**
     * get the internal array index of an internal player id
     * @param iPlayerId the internal player id
     * @return the internal array index of the peer
     * @see getIndexOfPeer
     */
    s32 getNumOfPeer(u16 iPlayerId);
    /**
     * get the ping of this client
     * @return the ping of this client or "0" if this is a server
     */
    u32 getPing();

    /**
     * This callback is called by the CMarbles2WorldObserver whenever somthing in the world changed
     * @param pMsg the event of the world's change
     */
    virtual void worldChange(IIrrOdeEvent *pMsg);

    /**
     * Call this method to start distributing all world changed over the network. This method registers the object
     * at the CMarbles2WorldObserver singleton
     */
    void startWorldUpdate();
    /**
     * call this method to stop distributing all world changes over the network. This method deregisteres the
     * object from the CMarbles2WorldObserver singleton
     */
    void stopWorldUpdate();

    /**
     * remove a scene node from physics
     * @param pNode the node to be removed
     */
    void removeFromPhysics(ISceneNode *pNode);

    /**
     * get the internal id of the peer at a specific array index
     * @param iIdx the index in the internal array
     * @return the internal id of the peer, "0" if no internal id was found
     * @see getIdOfPeerAtIndex
     */
    u16 getPlayerIdByIndex(u32 iIdx);

    /**
     * set a command verify object. Set this to "NULL" (initial value) to bypass command validation
     * @param pVerify the new verification object
     */
    void setCommandVerify(ICommandVerify *pVerify);

    /**
     * set the net iterations
     * @param iIterations the net iterations
     */
    void setNetIterations(u32 iIterations);
    /**
     * set global packet relay
     * @param bRelay enable or disable global packet relay
     */
    void setGlobalPacketRelay(bool bRelay);

    /**
     * this doesn't (yet) work, though it would be nice
     */
    virtual u32 getIpAddress();

    /**
     * for client side: add data for a peer
     * @param playerId the internal id of the peer
     * @param pData the data object for the player
     */
    void clientAddPeerData(u16 iPlayerId, void *pData);

    u32 getSentCount() { return m_iSentCnt; } /**< get the number of sent packets */
    u32 getRecvCount() { return m_iRecvCnt; } /**< get the number of received packets */
    u32 getSentSize() { return m_iSentSize; } /**< get the number of bytes sent */
    u32 getRecvSize() { return m_iRecvSize; } /**< get the number of bytes received */

    /**
     * get the traffic value in 1000 bytes/second
     */
    f32 getTraffic() { return ((f32)(m_iSentSize+m_iRecvSize))/(((f32)(m_iTime-m_iTrafficStartTime))); }

    /**
     * reset all counters
     */
    void resetCounters() { m_iSentCnt=0; m_iSentSize=0; m_iRecvCnt=0; m_iRecvSize=0; m_iTrafficStartTime=m_iTime; }
};

#endif
