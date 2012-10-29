#ifndef _C_HI_SCORE
  #define _C_HI_SCORE

  #include <IState.h>

class CLevelList;
class CFollowPathAnimator;
class CLevelSelectGUI;

class CHiScore : public IState, public IEventReceiver {
  private:
    u32 m_iRet;
    ISceneNode *m_pMenu;

    IGUIStaticText *m_pHiScoreList;
    ISoundEngine *m_pSndEngine;
    IGUIFont *m_pBigFont;
    CLevelSelectGUI *m_pLevelSelect;

    void updateHiScore();

  public:
    CHiScore(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);
};

#endif

