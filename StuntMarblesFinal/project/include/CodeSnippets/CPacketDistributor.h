#ifndef _C_PACKET_DISTRIBUTOR
  #define _C_PACKET_DISTRIBUTOR

  #include <IrrOdeNet/IPacketHandler.h>
  #include <CSerializer.h>

/**
 * This is the packet distributor. The CIrrOdeNetAdapter derived from this class. It handles all connection changes and received packets.
 * @author Christian Keimel / dustbin::games / bulletbyte
 */
class CPacketDistributor {
  protected:
    list<IPacketHandler *> m_lPacketHandlers;           /**< a list of packet handlers that handle the raw packets recieved from a client or server */
    list<IConnectionHandler *> m_lConnectionHandlers;   /**< a list of connection handlers that handle all connection changes (disconnect/connect) */

  public:
    /**
     * Add a packet handler. The handler will only be registered if it is not yet registered
     * @param pHandler the packet handler to be registered
     */
    void addPacketHandler(IPacketHandler *pHandler) {
      printf("adding packet handler\n");
      list<IPacketHandler *>::Iterator it;
      for (it=m_lPacketHandlers.begin(); it!=m_lPacketHandlers.end(); it++)
        if (*it==pHandler) {
          printf("\thandler already registered.\n");
          return;
        }
      m_lPacketHandlers.push_back(pHandler);
    }

    /**
     * remove a packet handler from the list
     * @param pHandler the packet handler to be removed
     */
    void removePacketHandler(IPacketHandler *pHandler) {
      list<IPacketHandler *>::Iterator it;
      printf("removing packet handler\n");
      for (it=m_lPacketHandlers.begin(); it!=m_lPacketHandlers.end(); it++)
        if (*it==pHandler) {
          m_lPacketHandlers.erase(it);
          return;
        }
    }

    /**
     * distribute a packet to all registered packet handlers
     * @param pBuffer the received data
     * @param iPlayerId the sender of the data
     */
    void distribute(CSerializer *pBuffer, u16 iPlayerId) {
      list<IPacketHandler *>::Iterator it;
      for (it=m_lPacketHandlers.begin(); it!=m_lPacketHandlers.end(); it++) {
        pBuffer->resetBufferPos();
        (*it)->handlePacket(pBuffer,iPlayerId);
      }
    }

    /**
     * add a connection handler. Handlers will only be registered once
     * @param pHandler the connection handler to register
     */
    void addConnectionHandler(IConnectionHandler *pHandler) {
      list<IConnectionHandler *>::Iterator it;
      for (it=m_lConnectionHandlers.begin(); it!=m_lConnectionHandlers.end(); it++) if (*it==pHandler) return;
      m_lConnectionHandlers.push_back(pHandler);
    }

    /**
     * remove connection handler.
     * @param pHandler the connection handler to be removed
     */
    void removeConnectionHandler(IConnectionHandler *pHandler) {
      list<IConnectionHandler *>::Iterator it;
      for (it=m_lConnectionHandlers.begin(); it!=m_lConnectionHandlers.end(); it++)
        if (*it==pHandler) {
          m_lConnectionHandlers.erase(it);
          return;
        }
    }

    /**
     * This method distributes a connection change
     * @param iPlayerId the internal id of the player that connects/disconnects
     * @param bConnected true if the change is a connection, false if is's a disconnection
     */
    void distributeConnectionChange(u16 iPlayerId, bool bConnected) {
      list<IConnectionHandler *>::Iterator it;
      for (it=m_lConnectionHandlers.begin(); it!=m_lConnectionHandlers.end(); it++)
        if (bConnected)
          (*it)->onPeerConnect(iPlayerId);
        else
          (*it)->onPeerDisconnect(iPlayerId);
    }
};

#endif
