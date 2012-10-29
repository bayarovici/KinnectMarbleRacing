#ifndef _C_GAME
  #define _C_GAME

  #include <IState.h>
  #include <irrCC.h>
  #include <IrrOde.h>
  #include <CShadowManager.h>

	#define C_FW 0
	#define C_BW 1
	#define C_LF 2
	#define C_RG 3
	#define C_BR 4
	#define C_CL 5
	#define C_CR 6
	#define C_RS 7

enum eReplayMessagesMarbles2 {
  eMessageReady=128,
  eMessageCntdn,
  eMessageGo,
  eMessageCheckpoint,
  eMessageLap,
  eMessageFastestLap,
  eMessagefinished,
  eMessageGameOver,
  eMessageShowLapList,
  eMessageQuitState
};

class CLapTime;
class COptions;
class CRaceTime;
class IGameLogic;
class CRespawnNode;
class CGhostPlayer;
class CReplayCam;
class CProgressBar;
class CIrrOdeKlangNode;
class CMarbles2Recorder;
class CMarbleFollowCameraAnimator;

using namespace ode;

class GUIElementToToggle {
  public:
    GUIElementToToggle(IGUIElement *pElement, u32 iTime, bool bVisible);
    IGUIElement *m_pElement;
    u32 m_iRemoveTime;
    bool m_bVisible;
};

class CGame : public IState, public IEventReceiver, public CManagedShadowProgress {
  protected:
		ITimer *m_pTimer;
    CIrrCC *m_pConfigCtrl;
    COptions *m_pOptions;

    list<GUIElementToToggle *> m_lElementsToToggle;

    dimension2di m_cScreenSize;

    array<rect<s32> > m_aViewPorts,m_aMirrors,m_aMirrorBack;
    u32 m_aCtrls[2][8];
    ICameraSceneNode *m_pCams[2],*m_pMirrorCams[2];
    stringc m_sLevelName;

    IMetaTriangleSelector *m_pSelector;
    ISoundEngine *m_pSndEngine;
    ISceneCollisionManager *m_pCollisionMngr;
    IGameLogic *m_pLogic;
    array<ITexture *> m_aCountdown;
    IGUIImage *m_pCountdown,
              *m_pGameOver,
              *m_pLoadingBackground;
    IGUIStaticText *m_pFastestLap,
                   *m_pLoadingProgress,
                   *m_pNetInfoPing,
                   *m_pNetInfoSent,
                   *m_pNetInfoRecv,
                   *m_pNetInfoTrfi,
                   *m_pNetInfoFps;
    CProgressBar *m_pPrgBar;
    IGUITab *m_pNetInfo;
    IGUIFont *m_pBigFont;

    CIrrOdeManager *m_pOdeManager;
    CMarbles2Recorder *m_pRecorder;

    u32 m_iTime,m_iState;

    bool m_bQuitGame,
         m_bInitError,
         m_bOdeInitialized,
         m_bMirrorVisible;

    wchar_t m_sInitErrorText[0xFF];

    u32 m_iNextState;
    CGhostPlayer *m_pGhostPlayer;

    void createTriangleSelectors(ISceneNode *pNode);

  public:
    CGame(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CGame();
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent(const SEvent &event);

    void addControlGui(u32 iSet, IGUIElement *pParent);
    void setOptions(COptions *pOptions);

    CIrrCC *getCC();

    bool loadLevel(const wchar_t *sLevel);
    void setNoViewports(u32 iNum, bool bSplitHorizontal);
    void setGameState(u32 iState);
    void setCam(u32 iNum, ICameraSceneNode *pCam);
    void initODE();

    IMetaTriangleSelector *getTriangleSelector();
    ISoundEngine *getSoundEngine();
    f32 getControl(u32 iSet, u32 iControl);

    void toggleCountdown(u32 iState);
    void showGameOver();
    void showFastestLap(const wchar_t *sText);
    void showRaceInfo(array<CRaceTime *> pInfo, CLapTime *pFastestLap, u32 iPlayers, const wchar_t **sNames);
    void addElementToToggle(IGUIElement *pElement, u32 iTime, bool bVisible);

    IGameLogic *getGameLogic();
    void setGameLogic(IGameLogic *pLogic);
    void setInitError(const wchar_t *sText);

    void initReplayRecord();
    virtual void progress(u32 iDone, u32 iComplete);

    CMarbles2Recorder *getRecorder();

    void setQuitGame(bool b);
    void setWaitingImageVisibility(bool b);

    void setNetInfoText(const wchar_t *sPing, const wchar_t *sSent, const wchar_t *sRecv, const wchar_t *sTraffic);

    void restartGhost();
    void stopGhost();
    void setGhostFile(const c8 *sFile);
};

#endif

