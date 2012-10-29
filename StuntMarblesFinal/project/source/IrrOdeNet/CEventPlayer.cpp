  #include <IrrOdeNet/CEventPlayer.h>

CEventPlayer::CEventPlayer(u32 iNum, const c8 *sName) : IIrrOdeEvent() {
  m_iNum=iNum;
  strcpy(m_sName,sName);
}

CEventPlayer::CEventPlayer(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iCode=pData->getU16();
  if (iCode==eMessagePlayer) {
    m_iNum=pData->getU16();
    pData->getString(m_sName);
  }
}

CEventPlayer::CEventPlayer(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eMessagePlayer) {
    CEventPlayer *p=(CEventPlayer *)pEvent;
    m_iNum=p->getNum();
    strcpy(m_sName,p->getName());
  }
}

const c8 *CEventPlayer::getName() {
  return m_sName;
}

u32 CEventPlayer::getNum() {
  return m_iNum;
}

CSerializer *CEventPlayer::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eMessagePlayer);
    m_pSerializer->addU16(m_iNum);
    m_pSerializer->addString(m_sName);
  }

  return m_pSerializer;
}

const c8 *CEventPlayer::toString() {
  sprintf(m_sString,"CEventPlayer: iNum=%i, sName=\"%s\"",m_iNum,m_sName);
  return m_sString;
}
