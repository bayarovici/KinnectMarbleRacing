  #include <IrrOdeNet/CEventIdentify.h>

CEventIdentify::CEventIdentify(const c8 *sAppName) : IIrrOdeEvent() {
  strcpy(m_sIdentify,_IRR_ODE_IDENTIFY_STRING);
  strcpy(m_sAppName,sAppName);
}

CEventIdentify::CEventIdentify(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iCode=pData->getU16();
  if (iCode==eMessageIdentify) {
    pData->getString(m_sIdentify);
    pData->getString(m_sAppName);
  }
}

CEventIdentify::CEventIdentify(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eMessageIdentify) {
    CEventIdentify *p=(CEventIdentify *)pEvent;
    strcpy(m_sIdentify,p->getIdentify());
    strcpy(m_sAppName ,p->getAppName ());
  }
}

const c8 *CEventIdentify::getIdentify() {
  return m_sIdentify;
}

const c8 *CEventIdentify::getAppName() {
  return m_sAppName;
}

CSerializer *CEventIdentify::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eMessageIdentify);
    m_pSerializer->addString(m_sIdentify);
    m_pSerializer->addString(m_sAppName);
  }

  return m_pSerializer;
}

const c8 *CEventIdentify::toString() {
  sprintf(m_sString,"CEventIdentify: sIdentify=\"%s\", sAppName=\"%s\"",m_sIdentify,m_sAppName);
  return m_sString;
}
