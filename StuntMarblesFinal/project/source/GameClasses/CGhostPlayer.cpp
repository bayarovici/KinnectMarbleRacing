  #include <GameClasses/CGhostPlayer.h>
  #include <GameClasses/CGhostClasses.h>
  #include <CIrrOdeManager.h>
  #include <event/IIrrOdeEventQueue.h>

CGhostPlayer::CGhostPlayer(IrrlichtDevice *pDevice, ISoundEngine *pSndEngine) {
  m_pDevice=pDevice;
  m_pReplayLap=NULL;
  m_pNode=pDevice->getSceneManager()->getSceneNodeFromName("ghost");
  irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
  m_bStopped=true;
  m_pSndEngine=pSndEngine;
}

CGhostPlayer::~CGhostPlayer() {
  if (m_pReplayLap!=NULL) {
    delete m_pReplayLap;
    m_pReplayLap=NULL;
  }
  irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
}

bool CGhostPlayer::validateGhostFile(const c8 *sFile, c8 *sPlayer, c8 *sLevel, u32 *iSteps) {
  bool bRet=true;
  u32 iSize=0;
  c8 sBuffer[0xFF];
  CSerializer cData;
  *iSteps=0;

  printf("open %s\n",sFile);

  IFileSystem *pFs=m_pDevice->getFileSystem();
  IReadFile *f=pFs->createAndOpenFile(path(sFile));

  f->read(&iSize,sizeof(u32));
  f->read(sBuffer,iSize);

  cData.setBuffer(sBuffer,iSize);

  CGhostHeader cHead(&cData);
  if (!strcmp(cHead.getIdent(),"StuntMarbleRacersGhostFile")) {
    strcpy(sPlayer,cHead.getPlayer());
    strcpy(sLevel ,cHead.getLevel ());
    while (f->getPos()<f->getSize()) {
      f->read(&iSize,sizeof(u32));
      bRet=bRet && iSize==24;
      f->read(sBuffer,iSize);
      cData.setBuffer(sBuffer,iSize);
      CGhostState cState(&cData);
      (*iSteps)++;
    }
  }

  f->drop();
  return bRet && !strcmp(cHead.getIdent(),"StuntMarbleRacersGhostFile");
}

void CGhostPlayer::loadGhostFile(const c8 *sFile) {
  if (m_pReplayLap!=NULL) delete m_pReplayLap;
  m_pReplayLap=new CGhostLaps(0);

  u32 iSize=0,iSteps=0;
  c8 sBuffer[0xFF];
  CSerializer cData;

  printf("open %s\n",sFile);

  IFileSystem *pFs=m_pDevice->getFileSystem();
  IReadFile *f=pFs->createAndOpenFile(path(sFile));

  f->read(&iSize,sizeof(u32));
  f->read(sBuffer,iSize);

  cData.setBuffer(sBuffer,iSize);

  CGhostHeader cHead(&cData);

  printf("\t\t--> %s\n\t\t--> %s\n\t\t--> %s\n",cHead.getPlayer(),cHead.getLevel(),cHead.getIdent());
  while (f->getPos()<f->getSize()) {
    f->read(&iSize,sizeof(u32));
    f->read(sBuffer,iSize);
    cData.setBuffer(sBuffer,iSize);
    CGhostState *pState=new CGhostState(&cData);
    m_pReplayLap->m_lStates.push_back(pState);
    iSteps++;
  }
  printf("%i steps\n",iSteps);
  m_it=m_pReplayLap->m_lStates.end();

  f->drop();
}

void CGhostPlayer::startGhost(bool bPlaySound) {
  if (bPlaySound && m_pSndEngine) m_pSndEngine->play2D("data/sounds/ghost1.ogg");
  m_it=m_pReplayLap->m_lStates.begin();
  m_bStopped=false;
}

void CGhostPlayer::stopGhost(bool bPlaySound) {
  if (bPlaySound && m_pSndEngine!=NULL) m_pSndEngine->play2D("data/sounds/ghost2.ogg");
  m_it=m_pReplayLap->m_lStates.end();
  m_bStopped=true;
}

bool CGhostPlayer::onEvent(IIrrOdeEvent *pEvent) {
  bool bRet=false;

  if (pEvent->getType()==irr::ode::eIrrOdeEventStep) {
    if (m_it!=m_pReplayLap->m_lStates.end()) {
      CGhostState *p=*m_it;
      if (m_pNode) {
        m_pNode->setVisible(true);
        m_pNode->setPosition(p->getPosition());
        m_pNode->setRotation(p->getRotation());
      }
      else printf("m_pNode==NULL\n");
      m_it++;
    }
    else {
      if (m_pSndEngine!=NULL && !m_bStopped) m_pSndEngine->play2D("data/sounds/ghost2.ogg");
      m_bStopped=true;
      if (m_pNode) m_pNode->setVisible(false);
    }
  }

  return bRet;
}

bool CGhostPlayer::handlesEvent(IIrrOdeEvent *pEvent) {
  return pEvent->getType()==irr::ode::eIrrOdeEventStep;
}
