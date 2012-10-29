#ifndef _C_NET_PLAYER
  #define _C_NET_PLAYER

enum eNetPlayerState {
  ePlayerStateUnknown,
  ePlayerConnected,
  ePlayerLevelReady,
  ePlayerRaceReady
};

class CNetPlayer {
  protected:
    c8 m_sName[0xFF];
    u16 m_iPlayerId,
        m_iNodeId;

    eNetPlayerState m_eState;

  public:
    CNetPlayer(u16 iPlayerId) { m_iPlayerId=iPlayerId; m_eState=ePlayerStateUnknown; }
    ~CNetPlayer() { }

    u16 getPlayerId() { return m_iPlayerId; }

    void setName(const c8 *sName) {
      printf("\t\tset name of %i to %s\n",m_iPlayerId,sName);
      strcpy(m_sName,sName);
    }

    const c8 *getName() { return m_sName; }

    void setState(eNetPlayerState e) { m_eState=e; }
    eNetPlayerState getState() { return m_eState; }

    void setNodeId(u16 iNodeId) { m_iNodeId=iNodeId; }
    u16 getNodeId() { return m_iNodeId; }
};

#endif
