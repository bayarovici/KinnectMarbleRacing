#ifndef _C_MENU
  #define _C_MENU

  #include <IState.h>

class CMenu : public IState, public IEventReceiver {
  private:
    u32 m_iMenuItem,m_iLastTime;
    bool m_bMoving,m_bSelect;
    ITimer *m_pTimer;
    ISceneNode *m_pMenu;
    ISoundEngine *m_pSndEngine;

  public:
    CMenu(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);
};

#endif
