  #include <irrlicht.h>
  #include <CIrrOdeKlangManager.h>
  #include <CIrrOdeKlangNode.h>

CIrrOdeKlangManager::CIrrOdeKlangManager() {
  #ifndef _IRR_ODE_KLANG_PLUGIN
	#endif
}

CIrrOdeKlangManager *CIrrOdeKlangManager::getSharedInstance() {
  static CIrrOdeKlangManager theManager;
  return &theManager;
}

void CIrrOdeKlangManager::activate() {
	#ifndef _IRR_ODE_KLANG_PLUGIN
		CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
	#endif
}

void CIrrOdeKlangManager::deactivate() {
	#ifndef _IRR_ODE_KLANG_PLUGIN
		CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
		removeAllNodes();
	#endif
}

void CIrrOdeKlangManager::addOdeKlangNode(CIrrOdeKlangNode *pNode) {
	#ifndef _IRR_ODE_KLANG_PLUGIN
    pNode->setSoundEngine(m_pSndEngine);
		if (pNode->isCollisionSound ())
      m_lCollisionNodes.push_back(pNode);
    else
      if (pNode->isLinearVelSound ())
        m_lLinVelNodes.push_back(pNode);
      else
        if (pNode->isAngularVelSound())
          m_lAngVelNodes.push_back(pNode);
        else
          m_lOtherNodes.push_back(pNode);
	#endif
}

void CIrrOdeKlangManager::removeAllNodes() {
  list<CIrrOdeKlangNode *>::Iterator it;

  for (it=m_lCollisionNodes.begin(); it!=m_lCollisionNodes.end(); it++) (*it)->stop();
  for (it=m_lLinVelNodes   .begin(); it!=m_lLinVelNodes   .end(); it++) (*it)->stop();
  for (it=m_lAngVelNodes   .begin(); it!=m_lAngVelNodes   .end(); it++) (*it)->stop();
  for (it=m_lOtherNodes    .begin(); it!=m_lOtherNodes    .end(); it++) (*it)->stop();

  m_lCollisionNodes.clear();
  m_lLinVelNodes.clear();
  m_lAngVelNodes.clear();
  m_lOtherNodes.clear();
  m_pListener[0]=NULL;
  m_pListener[1]=NULL;
}

void CIrrOdeKlangManager::setListener(ICameraSceneNode *pListener, u32 iIdx) {
  if (iIdx<2) m_pListener[iIdx]=pListener;
}

void CIrrOdeKlangManager::setListenerVelocity(vector3df vVel, u32 iIdx) {
  if (iIdx<2) m_vVel[iIdx]=vVel;
}

bool CIrrOdeKlangManager::onEvent(IIrrOdeEvent *pEvent) {
	#ifndef _IRR_ODE_KLANG_PLUGIN
	  if (!m_pSndEngine) return false;
		if (pEvent->getType()==irr::ode::eIrrOdeEventStep) {
		  if (m_pListener[1]==NULL) {
        ICameraSceneNode *pCam=m_pListener[0];

        if (pCam) {
          vector3df pos=pCam->getPosition(),tgt=pCam->getTarget(),upv=pCam->getUpVector();

          m_pSndEngine->setListenerPosition(vec3df(pos.X,pos.Y,pos.Z),
                                            vec3df(tgt.X,tgt.Y,tgt.Z),
                                            vec3df(0.0f,0.0f,0.0f),
                                            //vec3df(m_vVel[0].X,m_vVel[0].Y,m_vVel[0].Z),
                                            vec3df(upv.X,upv.Y,upv.Z));
        }
      }

      list<CIrrOdeKlangNode *>::Iterator it;
      for (it=m_lCollisionNodes.begin(); it!=m_lCollisionNodes.end(); it++) (*it)->update(m_pListener[0],m_pListener[1]);
      for (it=m_lLinVelNodes   .begin(); it!=m_lLinVelNodes   .end(); it++) (*it)->update(m_pListener[0],m_pListener[1]);
      for (it=m_lAngVelNodes   .begin(); it!=m_lAngVelNodes   .end(); it++) (*it)->update(m_pListener[0],m_pListener[1]);
      for (it=m_lOtherNodes    .begin(); it!=m_lOtherNodes    .end(); it++) (*it)->update(m_pListener[0],m_pListener[1]);
		}
  #endif

  return true;
}

bool CIrrOdeKlangManager::handlesEvent(IIrrOdeEvent *pEvent) {
  return pEvent->getType()==irr::ode::eIrrOdeEventStep;
}

void CIrrOdeKlangManager::setSoundEngine(ISoundEngine *pSndEngine) {
  m_pSndEngine=pSndEngine;

  list<CIrrOdeKlangNode *>::Iterator it;

  for (it=m_lCollisionNodes.begin(); it!=m_lCollisionNodes.end(); it++) (*it)->setSoundEngine(m_pSndEngine);
  for (it=m_lLinVelNodes   .begin(); it!=m_lLinVelNodes   .end(); it++) (*it)->setSoundEngine(m_pSndEngine);
  for (it=m_lAngVelNodes   .begin(); it!=m_lAngVelNodes   .end(); it++) (*it)->setSoundEngine(m_pSndEngine);
  for (it=m_lOtherNodes    .begin(); it!=m_lOtherNodes    .end(); it++) (*it)->setSoundEngine(m_pSndEngine);
}
