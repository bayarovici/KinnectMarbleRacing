  #include <CIrrOdeController.h>

void CIrrOdeController::addTorque(CIrrOdeBody *pBody, vector3df vTorque) {
  pBody->addTorque(vTorque);
}

void CIrrOdeController::setAngularDamping(CIrrOdeBody *pBody, f32 fDamping) {
  pBody->setAngularDamping(fDamping);
}

void CIrrOdeController::setPosition(CIrrOdeBody *pBody, vector3df vPos) {
  pBody->setPosition(vPos);
}

void CIrrOdeController::setLinearVelocity(CIrrOdeBody *pBody, vector3df vVel) {
  pBody->setLinearVelocity(vVel);
}

void CIrrOdeController::setAngularVeclocity(CIrrOdeBody *pBody, vector3df vVel) {
  pBody->setAngularVelocity(vVel);
}
