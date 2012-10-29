#ifndef _C_EVENT_INIT
  #define _C_EVENT_INIT

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

class CEventInit : public IIrrOdeEvent {
  protected:
    u32 m_iData;
    u8 m_iCode;

  public:
    CEventInit(u16 iCode, u32 iData);
    CEventInit(CSerializer *pData);
    CEventInit(irr::ode::IIrrOdeEvent *p);

    u32 getData() { return m_iData; }
    u8 getCode() { return m_iCode; }

    s32 getID() { return 0; }
    virtual u16 getType() { return eMessageInit; }
    virtual CSerializer *serialize();

    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventInit(this); }
};

#endif

