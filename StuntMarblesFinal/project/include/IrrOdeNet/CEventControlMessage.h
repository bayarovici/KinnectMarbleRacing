#ifndef _C_CLIENT_CONTROL_MESSAGE
  #define _C_CLIENT_CONTROL_MESSAGE

  #include <event/IIrrOdeEvent.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CEventControlMessage : public IIrrOdeEvent {
  protected:
    u16 m_iCode,
        m_iClientId;
    s32 m_iBodyId;
    f32 m_fData;
    vector3df m_vData;

  public:
    CEventControlMessage(u16 iCode);
    CEventControlMessage(CSerializer *pData);
    CEventControlMessage(irr::ode::IIrrOdeEvent *pEvent);

    u16 getCode() { return m_iCode; }

    virtual s32 getID() { return 0; }
    virtual u16 getType() { return eNetControlMessage; }

    void setPlayerId(u16 i) { m_iClientId=i; }
    u16 getPlayerId() { return m_iClientId; }

    void setVector(vector3df v) { m_vData=v; }
    vector3df getVector() { return m_vData; }

    void setBodyId(s32 i) { m_iBodyId=i; }
    s32 getBodyId() { return m_iBodyId; }

    void setFloat(f32 f) { m_fData=f; }
    f32 getFloat() { return m_fData; }

    virtual CSerializer *serialize();
    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventControlMessage(this); }
};

#endif


