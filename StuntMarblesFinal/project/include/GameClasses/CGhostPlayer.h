#ifndef _C_GHOST_PLAYER
  #define _C_GHOST_PLAYER

  #include <irrlicht.h>
  #include <irrklang.h>
  #include <event/IIrrOdeEventListener.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;
using namespace irrklang;

class CGhostLaps;
class CGhostState;

class CGhostPlayer : public irr::ode::IIrrOdeEventListener {
  protected:
    IrrlichtDevice *m_pDevice;
    CGhostLaps *m_pReplayLap;
    list<CGhostState *>::Iterator m_it;
    ISceneNode *m_pNode;
    ISoundEngine *m_pSndEngine;
    bool m_bStopped;

  public:
    CGhostPlayer(IrrlichtDevice *pDevice, ISoundEngine *pSndEngine);
    virtual ~CGhostPlayer();

    bool validateGhostFile(const c8 *sFile, c8 *sPlayer, c8 *sLevel, u32 *iSteps);
    void loadGhostFile(const c8 *sFile);

    void startGhost(bool bPlaySound);
    void stopGhost(bool bPlaySound);

    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);
};

#endif
