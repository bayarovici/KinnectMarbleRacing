#ifndef _C_GAME_LOGIC_CLIENT
  #define _C_GAME_LOGIC_CLIENT

  #include <irrlicht.h>
  #include <irrklang.h>
  #include <IrrOdeNet/IPacketHandler.h>

  #include <GameClasses/IGameLogic.h>

  #include <IrrOde.h>
  #include <CSerializer.h>

class CPlayer;
class COptions;
class CRespawnNode;
class CSectionTime;
class CRaceInfo;
class IIrrOdeController;
class CIrrOdeNetAdapter;
class CIrrOdeKlangManager;

using namespace ode;

class CGameLogicClient : public IGameLogic, public IIrrOdeEventListener, public IConnectionHandler {
  protected:
    CGame  *m_pGame;
    CIrrCC *m_pConfigCtrl;

    ITimer *m_pTimer;
    IrrlichtDevice *m_pDevice;
    IMetaTriangleSelector *m_pSelector;

    u32 m_iState,
        m_iTime,
        m_iLastCam,
        m_iLastTime,
        m_iStartTime,
        m_iPlayerIdx,
        m_iLastCp,
        m_iLocalStartTime;

    stringw m_aPlayerNames[4];
    CPlayer *m_pPlayer;
    list<CRespawnNode *> m_lCheckpoints;
    list<CRespawnNode *>::Iterator m_iNextCp[4];
    CRespawnNode *m_pRespawn[4];
    CIrrOdeKlangManager *m_pKlangManager;

    CRaceInfo *m_pRaceInfo;

    IIrrOdeController *m_pController;

    CIrrOdeNetAdapter *m_pNetAdapter;
    void findCheckpoint(ISceneNode *pNode);

  public:
    CGameLogicClient();
    virtual ~CGameLogicClient();

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

    virtual void setNetAdapter(CIrrOdeNetAdapter *pAdapter);

    //IConnectionHandler Callbacks
    virtual void onPeerConnect(u16 iPlayerId);
    virtual void onPeerDisconnect(u16 iPlayerId);

    virtual void quitState();

    virtual void resetPlayerObject();
};

#endif
