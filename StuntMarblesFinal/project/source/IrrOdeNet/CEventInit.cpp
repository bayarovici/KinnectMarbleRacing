  #include <IrrOdeNet/CEventInit.h>

CEventInit::CEventInit(u16 iCode, u32 iData) : IIrrOdeEvent() {
  m_iData=iData;
  m_iCode=iCode;
}

CEventInit::CEventInit(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iCode=pData->getU16();
  if (iCode==eMessageInit) {
    m_iCode=pData->getU16();
    m_iData=pData->getU32();
  }
}

CEventInit::CEventInit(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eMessageInit) {
    CEventInit *p=(CEventInit *)pEvent;
    m_iCode=p->getCode();
    m_iData=p->getData();
  }
}

CSerializer *CEventInit::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eMessageInit);
    m_pSerializer->addU16(m_iCode);
    m_pSerializer->addU32(m_iData);
  }

  return m_pSerializer;
}

const c8 *CEventInit::toString() {
  sprintf(m_sString,"CEventInit: iCode=%i, iData=%i",m_iCode,m_iData);
  return m_sString;
}
