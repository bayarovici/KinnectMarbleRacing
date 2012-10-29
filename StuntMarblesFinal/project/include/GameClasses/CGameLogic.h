#ifndef _C_GAME_LOGIC
  #define _C_GAME_LOGIC

  #include <irrlicht.h>
  #include <irrklang.h>

  #include <GameClasses/IGameLogic.h>

  #include <IrrOde.h>
  #include <CSerializer.h>
  #include <IrrOdeNet/IPacketHandler.h>

  #include <event/IIrrOdeEventListener.h>

class CPlayer;
class COptions;
class CRespawnNode;
class CSectionTime;
class CRaceInfo;
class IIrrOdeController;
class CGhostRecorder;
class CIrrOdeNetAdapter;

using namespace ode;

class CGameLogic : public IGameLogic, public IIrrOdeEventListener, public IConnectionHandler, public ICommandVerify {
  protected:
    CGame  *m_pGame;
    CIrrCC *m_pConfigCtrl;

    ITimer *m_pTimer;
    IrrlichtDevice *m_pDevice;
    IMetaTriangleSelector *m_pSelector;

    u32 m_iState,
        m_iTime,
        m_iStartTime,
        m_iCountdown,
        m_iPlayersFinished;

    stringw m_aPlayerNames[4];
    list<CPlayer *> m_lPlayers;
    list<CRespawnNode *> m_lCheckpoints;
    list<CRespawnNode *>::Iterator m_iNextCp[4];
    CRespawnNode *m_pRespawn[4];

    CRaceInfo *m_pRaceInfo;
    CIrrOdeManager *m_pOdeManager;

    CSerializer m_cSerializer;

    IIrrOdeController *m_pController;

    CIrrOdeNetAdapter *m_pNetAdapter;

    CGhostRecorder *m_pGhost;

    bool m_bRecordGhost;

    void findCheckpoint(ISceneNode *pNode);

    void addReplayMessageInit(u16 iMessageCode, u32 iData);
    void addReplayMessageCpInfo(u16 iMessageCode, u8 iPlayer, u32 iId, u32 iTime, u8 iCp, u8 iFlags);

  public:
    CGameLogic();
    virtual ~CGameLogic();

    virtual void setDevice(IrrlichtDevice *pDevice);
    virtual void setGame(CGame *pGame);
    virtual void setTriangleSelector(IMetaTriangleSelector *pSelector);
    virtual void init();
    virtual void initGame();
    virtual void update();
    virtual void respawn(CIrrOdeBody *pMarble, ICameraSceneNode *pCam, u32 idx);

    void removeFromPhysics(ISceneNode *pNode);

    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);
    virtual void setTimer(ITimer *pTimer);

    virtual const wchar_t *getHiScoreFile();

    virtual void setRaceInfo(CRaceInfo *pInfo);
    virtual CRaceInfo *getRaceInfo();

    virtual u32 getGameState();
    virtual void setGameState(u32 iState);

    virtual void setNetAdapter(CIrrOdeNetAdapter *pAdapter) { m_pNetAdapter=pAdapter; }

    //IPacketHandler Callbacks
    virtual void onPeerConnect(u16 iPlayerId);
    virtual void onPeerDisconnect(u16 iPlayerId);

    virtual bool validCommand(u16 iPlayerId, u16 iCommand, u16 iBodyId);

    virtual void quitState();

    virtual void resetPlayerObject() { }

    virtual void saveGhost();

    virtual void recordGhost(bool b) { m_bRecordGhost=b; }
};

#endif
