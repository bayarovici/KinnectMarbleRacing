#ifndef _C_EVENT_SERVER_MESSAGE
  #define _C_EVENT_SERVER_MESSAGE

  #include <event/IIrrOdeEvent.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CEventServerMessage : public IIrrOdeEvent {
  protected:
    u16 m_iCode,
        m_iData1,
        m_iData3;
    c8 m_sData2[0xFF];

  public:
    CEventServerMessage(u16 iCode);
    CEventServerMessage(CSerializer *pData);
    CEventServerMessage(irr::ode::IIrrOdeEvent *p);

    u16 getCode() { return m_iCode; }

    virtual s32 getID() { return 0; }
    virtual u16 getType() { return eNetServerMessage; }

    void setData1(u16 iData) { m_iData1=iData; }
    u16 getData1() { return m_iData1; }

    void setData3(u16 iData) { m_iData3=iData; }
    u16 getData3() { return m_iData3; }

    void setData2(const c8 *s) { strcpy(m_sData2,s); }
    const c8 *getData2() { return m_sData2; }

    virtual CSerializer *serialize();
    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventServerMessage(this); }
};

#endif
