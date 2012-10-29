  #include <CCredits.h>
  #include <CStateMachine.h>

CCredits::CCreditsAnimator::CCreditsAnimator(IVideoDriver *pDriver, ISceneManager *pSmgr) {
  m_iLastAnim=0;
  m_fRotY=0.0f;

  ITexture *pTex=NULL;
  u32 i=1;

  do {
    c8 sName[0xFF];
    sprintf(sName,"data/credits/%i.jpg",i);
    pTex=pDriver->getTexture(sName);
    if (pTex) m_lTextures.push_back(pTex);
    i++;
  }
  while (pTex!=NULL);
  m_lIt=m_lTextures.begin();
  m_bLessZero=true;
  m_iPauseStart=0;
  pLight=pSmgr->getSceneNodeFromName("light");
}

CCredits::CCreditsAnimator::~CCreditsAnimator() {
}

void CCredits::CCreditsAnimator::animateNode(ISceneNode *pNode, u32 iTime) {
  if (m_iLastAnim && (m_iPauseStart==0 || iTime-m_iPauseStart>1500)) {
    m_fRotY+=(f32)(iTime-m_iLastAnim)/15.0f;
    if (m_fRotY>90.0f) {
      m_fRotY-=180.0f;
      m_lIt++;
      if (m_lIt==m_lTextures.end()) m_lIt=m_lTextures.begin();
      pNode->getMaterial(0).setTexture(0,*m_lIt);
      m_bLessZero=true;
    }

    if (m_bLessZero && m_fRotY>0.0f) {
      m_bLessZero=false;
      m_iPauseStart=iTime;
      pLight->setVisible(true);
    }

    pNode->setRotation(vector3df(0.0f,m_fRotY,0.0f));
  }
  else pNode->setRotation(vector3df(0.0f,0.0f,0.0f));

  m_iLastAnim=iTime;
}

ISceneNodeAnimator *CCredits::CCreditsAnimator::createClone(ISceneNode *node, ISceneManager *newManager) {
  return NULL;
}

ESCENE_NODE_ANIMATOR_TYPE CCredits::CCreditsAnimator::getType() const {
  return (ESCENE_NODE_ANIMATOR_TYPE)0;
}

bool CCredits::CCreditsAnimator::isEventReceiverEnabled() const {
  return false;
}

bool CCredits::CCreditsAnimator::OnEvent(const SEvent &event) {
  return false;
}

CCredits::CCredits(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
}

void CCredits::activate(IState *pPrevious) {
  m_pDevice->setEventReceiver(this);
  m_iRet=0;
  m_pSndEngine=m_pStateMachine->getSoundEngine();

  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();

  m_pSmgr->clear();
  m_pSmgr->loadScene("data/credits.xml");

  ICameraSceneNode *pCam=m_pSmgr->addCameraSceneNode(0,vector3df(3.5f,0.0f,0.0f),vector3df(0.0f,0.0f,0.0f));
  if (pCam) m_pSmgr->setActiveCamera(pCam);
  ISceneNode *pCredits=m_pSmgr->getSceneNodeFromName("credits");
  if (pCredits) pCredits->addAnimator(new CCreditsAnimator(m_pDriver,m_pSmgr));
}

void CCredits::deactivate(IState *pNext) {
  m_pDevice->getSceneManager()->clear();
}

u32 CCredits::update() {
  return m_iRet;
}

bool CCredits::OnEvent(const SEvent &event) {
  if (event.EventType==EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown && event.KeyInput.Key==KEY_ESCAPE) {
    if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
    m_iRet=1;
  }

  return false;
}




