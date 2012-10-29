  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/NetMessages.h>

CEventClientMessage::CEventClientMessage(u16 iCode) {
  m_iCode=iCode;
}

CEventClientMessage::CEventClientMessage(CSerializer *pData) {
  pData->resetBufferPos();
  if (pData->getU16()==eNetClientMessage) {
    m_iCode=pData->getU16();

    switch (m_iCode) {
      case eMessageClientPing:
        m_iData1=pData->getU16();
        break;

      case eNetPlayerName:
        pData->getString(m_sData2);
        break;
    }
  }
}

CEventClientMessage::CEventClientMessage(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eNetClientMessage) {
    CEventClientMessage *p=(CEventClientMessage *)pEvent;

    m_iCode=p->getCode();
    m_iClientId=p->getClientId();
    m_iData1=p->getData1();
    strcpy(m_sData2,p->getData2());
    m_vData3=p->getData3();
    m_iData4=p->getData4();
    m_fData5=p->getData5();
  }
}

CSerializer *CEventClientMessage::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eNetClientMessage);
    m_pSerializer->addU16(m_iCode);

    switch (m_iCode) {
      case eMessageClientPing:
        m_pSerializer->addU16(m_iData1);
        break;

      case eNetPlayerName:
        m_pSerializer->addString(m_sData2);
        break;
    }
  }

  return m_pSerializer;
}

const c8 *CEventClientMessage::toString() {
  sprintf(m_sString,"CEventClientMessage");

  switch (m_iCode) {
    case eMessageClientPing:
      sprintf(m_sString,"%s (eMessageClientPing) data=%i",m_sString,m_iData1);
      break;

    case eNetPlayerName:
      sprintf(m_sString,"%s (eNetPlayerName) data2=\"%s\"",m_sString,m_sData2);
      break;

    case eNetClientRaceReady:
      sprintf(m_sString,"%s (eNetClientRaceReady)",m_sString);
      break;
  }

  return m_sString;
}
