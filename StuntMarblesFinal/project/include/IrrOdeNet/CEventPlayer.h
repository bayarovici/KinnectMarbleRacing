#ifndef _C_EVENT_PLAYER
  #define _C_EVENT_PLAYER

  #include <event/IIrrOdeEvent.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

  #define _IRR_ODE_IDENTIFY_STRING "IrrOdeReplayFile"

class CEventPlayer : public IIrrOdeEvent {
  protected:
    c8 m_sName[0xFF];
    u32 m_iNum;

  public:
    CEventPlayer(u32 iNum, const c8 *sName);
    CEventPlayer(CSerializer *pData);
    CEventPlayer(irr::ode::IIrrOdeEvent *p);

    const c8 *getName();
    u32 getNum();

    s32 getID() { return 0; }
    virtual u16 getType() { return eMessagePlayer; }
    virtual CSerializer *serialize();

    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventPlayer(this); }
};

#endif
