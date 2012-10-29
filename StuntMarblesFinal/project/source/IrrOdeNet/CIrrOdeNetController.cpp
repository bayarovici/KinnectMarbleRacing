  #include <IrrOdeNet/CIrrOdeNetController.h>
  #include <IrrOdeNet/CEventControlMessage.h>
  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/NetMessages.h>

void CIrrOdeNetController::addTorque(CIrrOdeBody *pBody, vector3df vTorque) {
  CEventControlMessage cMsg(eNetAddTorque);
  cMsg.setBodyId(pBody->getID());
  cMsg.setVector(vTorque);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CIrrOdeNetController::setAngularDamping(CIrrOdeBody *pBody, f32 fDamping) {
  CEventControlMessage cMsg(eNetSetAngularDamping);
  cMsg.setBodyId(pBody->getID());
  cMsg.setFloat(fDamping);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CIrrOdeNetController::setPosition(CIrrOdeBody *pBody, vector3df vPos) {
  CEventControlMessage cMsg(eNetSetPosition);
  cMsg.setBodyId(pBody->getID());
  cMsg.setVector(vPos);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CIrrOdeNetController::setLinearVelocity(CIrrOdeBody *pBody, vector3df vVel) {
  CEventControlMessage cMsg(eNetSetLinearVelocity);
  cMsg.setBodyId(pBody->getID());
  cMsg.setVector(vVel);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CIrrOdeNetController::setAngularVeclocity(CIrrOdeBody *pBody, vector3df vVel) {
  CEventControlMessage cMsg(eNetSetAngularVelocity);
  cMsg.setBodyId(pBody->getID());
  cMsg.setVector(vVel);
  m_pNetAdapter->sendPacket(&cMsg);
}

