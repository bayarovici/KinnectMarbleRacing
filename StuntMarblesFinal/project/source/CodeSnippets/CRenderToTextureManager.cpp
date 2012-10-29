  #include <CRenderToTextureManager.h>
  #include <CRenderToTextureNode.h>
  #include <CIrrOdeManager.h>
  #include <event/CIrrOdeEventStep.h>
  #include <event/IIrrOdeEventQueue.h>

void CRenderToTextureManager::findCam(ISceneNode *pParent) {
  list<ISceneNode *> pChildren=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  for (it=pChildren.begin(); it!=pChildren.end(); it++) {
    if ((*it)->getType()==REPLAY_CAM_NODE_ID) {
      m_lCams.push_back((CReplayCam *)(*it));
    }
    findCam(*it);
  }
}

CRenderToTextureManager::CRenderToTextureManager() {
  m_pSmgr=NULL;
  m_pDriver=NULL;

  m_iStepsToRender=0;
  m_bActive=false;
  m_bSwitch=true;
  m_iLastFrameNo=0;
}

CRenderToTextureManager::~CRenderToTextureManager() {
}

CRenderToTextureManager *CRenderToTextureManager::getSharedInstance() {
  static CRenderToTextureManager theManager;
  return &theManager;
}

void CRenderToTextureManager::setSceneManager(ISceneManager *pSmgr) {
  m_pSmgr=pSmgr;
  m_pDriver=m_pSmgr->getVideoDriver();
}

bool CRenderToTextureManager::onEvent(IIrrOdeEvent *pEvent) {
  if (!m_bActive) return true;
  m_iStepCnt++;

  if (pEvent->getType()==irr::ode::eIrrOdeEventStep) {
    if (m_iTargetSwitch>300) {
      if (m_itTarget==m_lTargets.end())
        m_itTarget=m_lTargets.begin();
      else {
        m_itTarget++;
        if (m_itTarget==m_lTargets.end()) m_itTarget=m_lTargets.begin();
      }

      m_iTargetSwitch=0;
    }
    else m_iTargetSwitch++;

    if (m_iStepCnt>=m_iStepsToRender && m_lTargets.getSize()>0 && m_iLastFrameNo!=m_iStepCnt) {
      m_iStepCnt=0;
      ISceneNode *pTarget=*m_itTarget;
      if (pTarget) {
        list<CReplayCam *>::Iterator it;
        CReplayCam *pActive=NULL;
        for (it=m_lCams.begin(); it!=m_lCams.end(); it++) {
          if (pActive==NULL || (pActive->getPosition()-pTarget->getPosition()).getLength()>((*it)->getPosition()-pTarget->getPosition()).getLength())
            pActive=*it;
        }

        if (pActive) {
          m_pDriver->setRenderTarget(m_pVideoTexture);
          m_pVideoCam->setPosition(pActive->getPosition());
          m_pVideoCam->setTarget(pTarget->getPosition());
          m_pSmgr->setActiveCamera(m_pVideoCam);
          m_pSmgr->drawAll();
          m_pDriver->setRenderTarget(0);
        }
        else printf("pActive==NULL!\n");
      }
      else printf("pTarget==NULL!\n");
      m_iLastFrameNo=m_iStepCnt;
    }
  }
  return true;
}

bool CRenderToTextureManager::handlesEvent(IIrrOdeEvent *pEvent) {
  return pEvent->getType()==irr::ode::eIrrOdeEventStep;
}

void CRenderToTextureManager::addRenderToTextureNode(CRenderToTextureNode *pNode) {
  printf("addRenderToTextureNode: ");
  if (pNode->getParent()==NULL) {
    printf("getParent==NULL\n");
    return;
  }

  printf("setting texture %i of material %i to rtt\n",pNode->getMaterial(),pNode->getTexture());
  m_lNodes.push_back(pNode);
}

void CRenderToTextureManager::setActive(bool b) {
  m_bActive=b && m_bSwitch;
  if (m_bActive) {
    findCam(m_pSmgr->getRootSceneNode());
    printf("CRenderToTextureManager::setActive ... ");
    if (m_pDriver->queryFeature(video::EVDF_RENDER_TO_TARGET)) {
      m_pVideoTexture=m_pDriver->addRenderTargetTexture(core::dimension2du(m_iTextureSize,m_iTextureSize),"video_screen");
      m_pVideoCam=m_pSmgr->addCameraSceneNode();

      list<CRenderToTextureNode *>::Iterator it;
      for (it=m_lNodes.begin(); it!=m_lNodes.end(); it++) {
        CRenderToTextureNode *pNode=*it;
        pNode->getParent()->getMaterial(pNode->getMaterial()).setTexture(pNode->getTexture(),m_pVideoTexture);
      }
      printf("OK\n");
    }
    else {
      m_pVideoTexture=NULL;
      m_pVideoCam=NULL;
      printf("RTT no availble!\n");
    }

    irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
    m_iStepCnt=0;
    m_iTargetSwitch=0;
    m_itTarget=m_lTargets.begin();
  }
  else {
    irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
    clearLists();
  }
}

void CRenderToTextureManager::clearLists() {
  printf("clear lists begin\n");
  m_lCams.clear();
  m_lNodes.clear();
  m_lTargets.clear();
  printf("clear lists end\n");
}

void CRenderToTextureManager::setStepsToRender(u32 i) {
  m_iStepsToRender=i;
}

void CRenderToTextureManager::addTarget(ISceneNode *pTarget) {
  m_lTargets.push_back(pTarget);
  m_itTarget=m_lTargets.begin();
}
