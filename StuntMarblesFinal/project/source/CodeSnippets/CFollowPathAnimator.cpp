  #include <CFollowPathAnimator.h>
  #include <CPreviewNode.h>

CFollowPathAnimator::CFollowPathAnimator(IrrlichtDevice *pDevice, const c8 *sPathRootName, f32 iAnimSpeed, f32 iCamTargetSpeed, ICameraSceneNode *pCam, ISceneManager *pSmgr) {
  m_fAnimSpeed=iAnimSpeed;
  m_fCamTargetSpeed=iCamTargetSpeed;

  m_iLastTime=0;

  m_bTurn=false;
  m_bAnimRunning=false;
  m_fOldDistance=0.0f;

  m_pCam=pCam;

  list<ISceneNode *> childList=pSmgr->getRootSceneNode()->getChildren();
  list<ISceneNode *>::Iterator it;
  CPreview *pPreview=NULL;

  for (it=childList.begin(); it!=childList.end() && !pPreview; it++) {
    if ((*it)->getType()==PREVIEW_ID) pPreview=(CPreview *)(*it);
  }

  if (pPreview) {
    list<ISceneNode *> children=pPreview->getChildren();
    list<ISceneNode *>::Iterator it;
    for (it=children.begin(); it!=children.end(); it++) {
      m_cPathPoints.push_back((*it)->getAbsolutePosition());
    }
    m_cIter=m_cPathPoints.begin();
    m_cCamPos=*m_cIter; m_cIter++;
    m_cCamTgt=*m_cIter; m_cIter++;
    m_cCamDir=(m_cCamTgt-m_cCamPos).normalize();
    m_pCam->setPosition(m_cCamPos);
  }
  else m_bAnimRunning=false;
  printf("anim running?%s\n",m_bAnimRunning?"YES":"NO");
}

CFollowPathAnimator::~CFollowPathAnimator() {
  m_cPathPoints.clear();
}

void CFollowPathAnimator::animateNode(ISceneNode *pNode, u32 timeMs) {
  if (!m_bAnimRunning) return;
  if (m_iLastTime==0) { m_iLastTime=timeMs; return; }
  f32 fTimeDiff=(f32)(timeMs-m_iLastTime);
  if (timeMs==m_iLastTime) return;
  m_iLastTime=timeMs;
  vector3df move;
  f32 fDistance;
  fDistance=(m_cCamTgt-m_cCamPos).getLength();

  move=fTimeDiff*m_cCamDir/m_fAnimSpeed;
  m_cCamPos+=move;
  m_pCam->setPosition(m_cCamPos);
  m_pCam->setTarget(m_pCam->getPosition()+m_cCamDir);

  if (!m_bTurn)
    if (fDistance<5.0f || (m_fOldDistance!=0.0f && fDistance-m_fOldDistance>0.0f)) {
      m_fOldDistance=0.0f;
      m_cCamTgt=*m_cIter;
      m_cIter++;
      if (m_cIter==m_cPathPoints.end()) m_cIter=m_cPathPoints.begin();
      m_bTurn=true;
      m_cCamNDr=(m_cCamTgt-m_cCamPos).normalize();
    }
    else m_fOldDistance=fDistance;
  else {
    vector3df v=(m_cCamNDr-m_cCamDir).normalize();
    m_cCamDir+=fTimeDiff*v/1000.0f;
    v=(m_cCamNDr-m_cCamDir);
    if (v.getLength()<0.001f) m_bTurn=false;
  }
}

ISceneNodeAnimator *CFollowPathAnimator::createClone(ISceneNode *node, ISceneManager *pNewMngr) {
  return NULL;
}

ESCENE_NODE_ANIMATOR_TYPE CFollowPathAnimator::getType() const {
  return (ESCENE_NODE_ANIMATOR_TYPE)2305;
}

bool CFollowPathAnimator::isEventReceiverEnabled() const {
  return false;
}

void CFollowPathAnimator::animatedEnded() {
}

void CFollowPathAnimator::setCamParameter(ICameraSceneNode *pCam) {
  pCam->setPosition(m_cCamPos);
  pCam->setTarget(m_cCamTgt);
}

void CFollowPathAnimator::start() {
  printf("size: %i\n",m_cPathPoints.getSize());
  m_bAnimRunning=m_cPathPoints.getSize()>0;
}

bool CFollowPathAnimator::animationRunning() {
  return m_bAnimRunning;
}
