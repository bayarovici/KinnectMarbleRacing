#ifndef _C_EVENT_CP_INFO
  #define _C_EVENT_CP_INFO

  #include <event/IIrrOdeEvent.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

//addReplayMessageCpInfo(u16 iMessageCode, u8 iPlayer, u32 iId, u32 iTime, u8 iCp, u8 iFlags)

class CEventCpInfo : public IIrrOdeEvent {
  protected:
    u32 m_iId,m_iTime;
    u16 m_iCode;
    u8 m_iPlayer,m_iCp,m_iFlags;

  public:
    CEventCpInfo(u16 iMessageCode, u8 iPlayer, u32 iId, u32 iTime, u8 iCp, u8 iFlags);
    CEventCpInfo(CSerializer *pData);
    CEventCpInfo(irr::ode::IIrrOdeEvent *pEvent);

    u32 getId() { return m_iId; }
    u32 getTime() { return m_iTime; }
    u16 getCode() { return m_iCode; }
    u8 getPlayer() { return m_iPlayer; }
    u8 getCp() { return m_iCp; }
    u8 getFlags() { return m_iFlags; }

    s32 getID() { return 0; }
    virtual u16 getType() { return eMessageCpInfo; }
    virtual CSerializer *serialize();
    virtual const c8 *toString();

    virtual IIrrOdeEvent *clone() { return new CEventCpInfo(this); }
};

#endif


