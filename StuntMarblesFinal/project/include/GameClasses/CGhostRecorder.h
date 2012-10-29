#ifndef _C_GHOST_RECORDER
  #define _C_GHOST_RECORDER

  #include <IrrOde.h>
  #include <irrlicht.h>
  #include <IrrOdeNet/CMarbles2WorldObserver.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CGhostHeader;
class CGhostLaps;

class CGhostRecorder : public IIrrOdeWorldListener {
  protected:
    ISceneManager *m_pSmgr;
    IrrlichtDevice *m_pDevice;
    list<CGhostLaps *> m_lLaps;
    CGhostHeader *m_pHeader;
    CGhostLaps *m_pCurLap;
    ISceneNode *m_pNode;
    u32 m_iLapNo;

  public:
    CGhostRecorder(IrrlichtDevice *pDevice);
    virtual ~CGhostRecorder();

    void setData(u32 iNodeId, const c8 *sPlayer, const c8 *sLevel);
    void activate();
    void deactivate();

    int getLapCount();

    virtual void worldChange(IIrrOdeEvent *pMsg);

    void save();
};

#endif
