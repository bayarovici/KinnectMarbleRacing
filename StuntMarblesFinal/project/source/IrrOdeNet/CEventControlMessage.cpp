  #include <IrrOdeNet/CEventControlMessage.h>
  #include <IrrOdeNet/NetMessages.h>

CEventControlMessage::CEventControlMessage(u16 iCode) {
  m_iCode=iCode;
}

CEventControlMessage::CEventControlMessage(CSerializer *pData) {
  pData->resetBufferPos();
  if (pData->getU16()==eNetControlMessage) {
    m_iCode=pData->getU16();
    m_iBodyId=pData->getS32();

    switch (m_iCode) {
      case eNetAddTorque:
      case eNetSetPosition:
      case eNetSetLinearVelocity:
      case eNetSetAngularVelocity:
        pData->getVector3df(m_vData);
        break;

      case eNetSetAngularDamping:
        m_fData=pData->getF32();
        break;
    }
  }
}

CEventControlMessage::CEventControlMessage(irr::ode::IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==eNetControlMessage) {
    CEventControlMessage *p=(CEventControlMessage *)pEvent;
    m_iCode=p->getCode();
    m_vData=p->getVector();
    m_fData=p->getFloat();
    m_iCode=p->getCode();
    m_iClientId=p->getPlayerId();
  }
}

CSerializer *CEventControlMessage::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eNetControlMessage);
    m_pSerializer->addU16(m_iCode);
    m_pSerializer->addS32(m_iBodyId);

    switch (m_iCode) {
      case eNetAddTorque:
      case eNetSetPosition:
      case eNetSetLinearVelocity:
      case eNetSetAngularVelocity:
        m_pSerializer->addVector3df(m_vData);
        break;

      case eNetSetAngularDamping:
        m_pSerializer->addF32(m_fData);
        break;
    }
  }

  return m_pSerializer;
}

const c8 *CEventControlMessage::toString() {
  sprintf(m_sString,"CEventControlMessage");

  switch (m_iCode) {
    case eNetAddTorque         : sprintf(m_sString,"%s (eNetAddTorque)"         ,m_sString); break;
    case eNetSetPosition       : sprintf(m_sString,"%s (eNetSetPosition)"       ,m_sString); break;
    case eNetSetLinearVelocity : sprintf(m_sString,"%s (eNetSetLinearVelocity)" ,m_sString); break;
    case eNetSetAngularVelocity: sprintf(m_sString,"%s (eNetSetAngularVelocity)",m_sString); break;
    case eNetSetAngularDamping:
      sprintf(m_sString,"%s (eNetSetAngularDamping): %.2f",m_sString,m_fData);
      break;
  }

  switch (m_iCode) {
    case eNetAddTorque:
    case eNetSetPosition:
    case eNetSetLinearVelocity:
    case eNetSetAngularVelocity:
    sprintf(m_sString,"%s: (%.2f, %.2f, %.2f)",m_sString,m_vData.X,m_vData.Y,m_vData.Z);
  }

  return m_sString;
}


