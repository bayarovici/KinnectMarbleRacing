  #include <IrrOdeNet/CEventCpInfo.h>

CEventCpInfo::CEventCpInfo(u16 iMessageCode, u8 iPlayer, u32 iId, u32 iTime, u8 iCp, u8 iFlags) : IIrrOdeEvent() {
  m_iCode=iMessageCode;
  m_iPlayer=iPlayer;
  m_iId=iId;
  m_iTime=iTime;
  m_iCp=iCp;
  m_iFlags=iFlags;
}

CEventCpInfo::CEventCpInfo(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iCode=pData->getU16();
  if (iCode==eMessageCpInfo) {
    m_iCode  =pData->getU16();
    m_iPlayer=pData->getU8 ();
    m_iId    =pData->getU32();
    m_iTime  =pData->getU32();
    m_iCp    =pData->getU8 ();
    m_iFlags =pData->getU8 ();
  }
}

CEventCpInfo::CEventCpInfo(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eMessageCpInfo) {
    CEventCpInfo *p=(CEventCpInfo *)pEvent;
    m_iCode=p->getCode();
    m_iPlayer=p->getPlayer();
    m_iId=p->getId();
    m_iTime=p->getTime();
    m_iCp=p->getCp();
    m_iFlags=p->getFlags();
  }
}

CSerializer *CEventCpInfo::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eMessageCpInfo);
    m_pSerializer->addU16(m_iCode       );
    m_pSerializer->addU8 (m_iPlayer     );
    m_pSerializer->addU32(m_iId         );
    m_pSerializer->addU32(m_iTime       );
    m_pSerializer->addU8 (m_iCp         );
    m_pSerializer->addU8 (m_iFlags      );
  }

  return m_pSerializer;
}

const c8 *CEventCpInfo::toString() {
  sprintf(m_sString,"CEventCpInfo: iCode=%i, iPlayer=%i, iId=%i, iTime=%i, iCp=%i, iFlags=%i",
          m_iCode,m_iPlayer,m_iId,m_iTime,m_iCp,m_iFlags);
  return m_sString;
}
