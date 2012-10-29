#ifndef _C_REPLAY
  #define _C_REPLAY

  #include <IState.h>
  #include <irrklang.h>
  #include <CConfigFileManager.h>

  #include <IrrOdeNet/CIrrOdeRePlayer.h>

class CMarbleFollowCameraAnimator;
class CMarbles2Replayer;
class CStateMachine;
class CReplaySelect;
class CLevelList;
class CReplayCam;

class CReplay : public IState, public IEventReceiver {
  protected:
    u32 m_iRet,
        m_iFocused,
        m_iLastTime,
        m_iThisTime;
    s32 m_iMoveX,
        m_iMoveY,
        m_iMoveZ,
        m_iAngleH,
        m_iAngleV,
        m_iCamMode;
    vector3df m_vCamPos,
              m_vCamRot,
              m_vCamTgt;
    CMarbles2Replayer *m_pRePlayer;
    ITimer *m_pTimer;
    CReplaySelect *m_pSelect;
    ICameraSceneNode *m_pCam,
                     *m_pFreeCam;
    ISoundEngine *m_pSndEngine;
    IGUIStaticText *m_pLoadingProgress,
                   *m_pCamInfo,
                   *m_pPaused;
    IGUIImage *m_pLoadingBackground;
    ISceneNode *m_pActiveCam;
    IMetaTriangleSelector *m_pSelector;

    list<CReplayCam *> m_lCams;
    array<ISceneNode *> m_aPlayers;
    array<ICameraSceneNode *> m_aFollowCams,
                              m_aBlimpCams;
    array<f32> m_aBlimpDist;
    array<CMarbleFollowCameraAnimator *> m_aAnimators;

    ISceneCollisionManager *m_pCollisionMngr;
    ISceneNode *findCamera(ISceneNode *pParent);
    void findCam(ISceneNode *pParent);
    void updateCamInfo();

  public:
    CReplay(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CReplay();

    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    void setReplaySelect(CReplaySelect *pSelect);
    void createTriangleSelectors(ISceneNode *pParent);
};

#endif
