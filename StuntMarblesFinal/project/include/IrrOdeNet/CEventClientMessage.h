#ifndef _C_CLIENT_CLIENT_MESSAGE
  #define _C_CLIENT_CLIENT_MESSAGE

  #include <event/IIrrOdeEvent.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CEventClientMessage : public IIrrOdeEvent {
  protected:
    u16 m_iCode,
        m_iClientId,
        m_iData1;
    c8 m_sData2[0xFF];
    vector3df m_vData3;
    s32 m_iData4;
    f32 m_fData5;

  public:
    CEventClientMessage(u16 iCode);
    CEventClientMessage(CSerializer *pData);
    CEventClientMessage(irr::ode::IIrrOdeEvent *pEvt);

    u16 getCode() { return m_iCode; }

    u16 getClientId() { return m_iClientId; }

    virtual s32 getID() { return 0; }
    virtual u16 getType() { return eNetClientMessage; }

    void setPlayerId(u16 i) { m_iClientId=i; }
    u16 getPlayerId() { return m_iClientId; }

    void setData1(u16 iData) { m_iData1=iData; }
    u16 getData1() { return m_iData1; }

    void setData2(const c8 *sData) { strcpy(m_sData2,sData); }
    const c8 *getData2() { return m_sData2; }

    void setData3(vector3df v) { m_vData3=v; }
    vector3df getData3() { return m_vData3; }

    void setData4(s32 i) { m_iData4=i; }
    s32 getData4() { return m_iData4; }

    void setData5(f32 f) { m_fData5=f; }
    f32 getData5() { return m_fData5; }

    virtual CSerializer *serialize();
    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventClientMessage(this); }
};

#endif

