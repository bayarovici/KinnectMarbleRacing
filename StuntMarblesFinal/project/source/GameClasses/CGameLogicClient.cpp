  #include <GameClasses/CMarbleFollowAnimator.h>
  #include <GameClasses/CGameLogicClient.h>
  #include <GameClasses/CNetPlayer.h>
  #include <GameClasses/CRaceInfo.h>
  #include <GameClasses/CPlayer.h>

  #include <IrrOdeNet/CMarbles2WorldObserver.h>
  #include <IrrOdeNet/CIrrOdeNetController.h>
  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/CMarbles2Recorder.h>
  #include <IrrOdeNet/CEventCpInfo.h>
  #include <IrrOdeNet/NetMessages.h>
  #include <IrrOdeNet/CEventInit.h>

  #include <event/CIrrOdeEventStep.h>

  #include <CRenderToTextureManager.h>
  #include <CIrrOdeKlangManager.h>
  #include <CSerializer.h>
  #include <CRespawnNode.h>
  #include <CMessage.h>
  #include <COptions.h>

  #include <CGame.h>

CGameLogicClient::CGameLogicClient() {
  m_pGame=NULL;
  m_pDevice=NULL;
  m_pNetAdapter=NULL;
  m_pPlayer=NULL;

  m_iState=eGameStateStarting;
  m_iStartTime=0;
  m_iLastTime=0;
  m_iLastCam=0;
  m_iPlayerIdx=0;
  m_iTime=0;
  m_iLastCp=0;
  m_iLocalStartTime=0;

  m_pController=NULL;

  m_pKlangManager=CIrrOdeKlangManager::getSharedInstance();
  m_pKlangManager->setListener(NULL,0);
  m_pKlangManager->setListener(NULL,1);
  m_pKlangManager->activate();
}

CGameLogicClient::~CGameLogicClient() {
  printf("CGameLogicClient destructor\n");
  CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
  if (m_pNetAdapter) m_pNetAdapter->removeConnectionHandler(this);
  delete m_pController;
}

void CGameLogicClient::setGame(CGame *pGame) {
  m_pGame=pGame;
  m_pGame->setGameLogic(this);
}

void CGameLogicClient::init() {
  if (m_pGame) {
    ISceneManager *pSmgr=m_pDevice->getSceneManager();
    CRenderToTextureManager::getSharedInstance()->clearLists();
    m_pGame->loadLevel(m_pRaceInfo->getLevel().c_str());

    for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) {
      c8 s[0xFF];
      sprintf(s,"player%i",i+1);
      ISceneNode *pNode=pSmgr->getSceneNodeFromName(s);
      if (pNode) CRenderToTextureManager::getSharedInstance()->addTarget(pNode);
      printf("%s: %i\n",s,(int)pNode);
    }

    m_pGame->setNoViewports(1,true);
    m_iTime=m_pTimer->getTime();
    CRenderToTextureManager::getSharedInstance()->setActive(true);

    findCheckpoint(m_pDevice->getSceneManager()->getRootSceneNode());
    printf("%i checkpoints found\n",m_lCheckpoints.getSize());

    if (m_lCheckpoints.getSize()<2) {
      wchar_t s[0xFF];
      swprintf(s,L"invalid level: not enough checkpoints (%i found - 2 required)!",m_lCheckpoints.getSize());
      m_pGame->setInitError(s);
      return;
    }

    printf("\n\t\tclient ready!\n\n");
    m_pNetAdapter->resetCounters();
    CEventClientMessage cMsg(eNetClientRaceReady);
    m_pNetAdapter->sendPacket(&cMsg);

    initGame();
  }
  else m_iState=eGameStateStarting;
}

void CGameLogicClient::initGame() {
  m_pGame->initODE();
  m_pNetAdapter->setSceneManager(m_pDevice->getSceneManager());

  CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
  m_pGame->setWaitingImageVisibility(true);
}

void CGameLogicClient::setDevice(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
}

void CGameLogicClient::update() {
  m_pNetAdapter->update();
  static u32 iFrameNo=0;

  wchar_t sPing[0xFF],sSent[0xFF],sRecv[0xFF],sTrfi[0xFF];

  swprintf(sPing,L"Ping: %i",m_pNetAdapter->getPing());
  swprintf(sSent,L"Sent: %i (%i)",m_pNetAdapter->getSentCount(),m_pNetAdapter->getSentSize());
  swprintf(sRecv,L"Recv: %i (%i)",m_pNetAdapter->getRecvCount(),m_pNetAdapter->getRecvSize());
  swprintf(sTrfi,L"Traffic: %.2f",m_pNetAdapter->getTraffic());

  m_pGame->setNetInfoText(sPing,sSent,sRecv,sTrfi);

  m_iTime=m_pTimer->getTime();
  if (m_iLastTime==0) m_iLastTime=m_iTime;

  while (m_iTime-m_iLastTime>16) {

    m_iLastTime+=16;
    if (m_pPlayer) {
      irr::ode::CIrrOdeEventStep *pStep=new irr::ode::CIrrOdeEventStep((u32)0);
      irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->postEvent(pStep);

      m_pPlayer->step(m_iTime-m_iLastCam>16);
      while (m_iTime-m_iLastCam>16) m_iLastCam+=16;
    }

    CIrrOdeEventStep *pStep=new CIrrOdeEventStep((u32)0);
    pStep->setFrameNo(iFrameNo);
    CIrrOdeManager::getSharedInstance()->getQueue()->postEvent(pStep);
  }
  iFrameNo++;

  if (m_iState==eGameStatePlay) m_pPlayer->updateGUI(m_iTime-m_iLocalStartTime);
}

void CGameLogicClient::removeFromPhysics(ISceneNode *pNode) {
  if (pNode->getType()==IRR_ODE_BODY_ID) {
    CIrrOdeBody *p=reinterpret_cast<CIrrOdeBody *>(pNode);
    p->removeFromPhysics();
  }
}

bool CGameLogicClient::onEvent(IIrrOdeEvent *pEvent) {
  if (pEvent->getType()==irr::ode::eIrrOdeEventBodyRemoved) {
    CIrrOdeEventBodyRemoved *p=(CIrrOdeEventBodyRemoved *)pEvent;
    ISceneNode *pNode=m_pDevice->getSceneManager()->getSceneNodeFromId(p->getBodyId());
    if (pNode) pNode->setVisible(false);
  }

  if (pEvent->getType()==irr::ode::eIrrOdeEventBodyMoved) {
    CIrrOdeEventBodyMoved *pMoved=(CIrrOdeEventBodyMoved *)pEvent;
    if (m_pPlayer->getMarble()==pMoved->getBody()) {
      m_pPlayer->camStep();
      m_iLastCam=m_pTimer->getTime();
    }
  }

  if (pEvent->getType()==eMessageInit) {
    CEventInit *p=(CEventInit *)pEvent;
    switch (p->getCode()) {
      case eMessageReady:
        m_pGame->toggleCountdown(0);
        m_pGame->setWaitingImageVisibility(false);
        break;

      case eMessageCntdn: {
          m_pGame->toggleCountdown(4-p->getData());
          wchar_t s[0xFF];
          printf("\t\t%i\n",p->getData());
          swprintf(s,L"%i",p->getData());
          m_pPlayer->setInfoText(s);
        }
        break;

      case eMessageGo:
        m_pGame->toggleCountdown(4);
        m_iStartTime=p->getData();
        m_iLocalStartTime=m_pTimer->getTime();
        m_pPlayer->setActive(true);
        m_iState=eGameStatePlay;
        break;
    }
  }

  if (pEvent->getType()==eNetServerMessage) {
    printf("CGameLogicClient::onEvent\n");
    CEventServerMessage *p=(CEventServerMessage *)pEvent;
    switch (p->getCode()) {
      case eNetClientPlayerId: {
          m_iPlayerIdx=p->getData1();
          c8 s[0xFF];
          sprintf(s,"player%i",m_iPlayerIdx);
          if (m_pPlayer==NULL) {
            printf("\n\t\tI am player %i | %i | %i\n\n",m_iPlayerIdx,(int)this,(int)m_pPlayer);
            ISceneNode *pNode=m_pDevice->getSceneManager()->getSceneNodeFromName(s);
            if (pNode) {
              core::rect<s32> viewPort=m_pDevice->getVideoDriver()->getViewPort();
              f32 fAr=1.0f*viewPort.getWidth()/viewPort.getHeight();

              m_pPlayer=new CPlayer(m_iPlayerIdx+1,1,m_pDevice,fAr,m_pGame,0.0f,true);
              m_pPlayer->setTimer(m_pTimer);
              m_pGame->setCam(0,m_pPlayer->getCamera());
              m_pPlayer->createPlayerGUI(m_pRaceInfo->getSplitHorizontal(),m_aPlayerNames[m_iPlayerIdx].c_str(),m_lCheckpoints.getSize());
              m_pPlayer->setController(m_pController);
              m_pPlayer->setInfoText(L"Ready.");
              m_pPlayer->camStep();
            }
            else printf("ERROR: node not found!\n");

            m_iNextCp[m_iPlayerIdx]=m_lCheckpoints.begin();
            m_pRespawn[m_iPlayerIdx]=*m_iNextCp[m_iPlayerIdx];
          }
        }
        break;

      case eMessageGameOver:
        m_pGame->showGameOver();
        m_pRaceInfo->calculateResult();
        break;

      case eMessageShowLapList: {
          const wchar_t *sPlayers[4];
          for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) sPlayers[i]=m_aPlayerNames[i].c_str();

          m_pGame->showRaceInfo(m_pRaceInfo->getRaceTimeArray(),m_pRaceInfo->getFastestLap(),m_pRaceInfo->getPlayerNo(),(const wchar_t **)sPlayers);
          m_pPlayer->clearGUI();
        }
        break;

      case eMessageQuitState:
        m_pGame->setQuitGame(true);
        break;
    }
  }

  if (pEvent->getType()==eMessageCpInfo) {
    CEventCpInfo *pEvt=(CEventCpInfo *)pEvent;
    u16 iCode=pEvt->getCode();
    switch (iCode) {
      case eMessageCheckpoint:
      case eMessageLap:
      case eMessagefinished:
      case eMessageFastestLap:{
          u8 iPlayer=pEvt->getPlayer();;
          u32 iTime=pEvt->getTime(),
              iGameTime=iTime-m_iStartTime;
          u8 iCp=pEvt->getCp(),
             iFlags=pEvt->getFlags(),
             iLapNo=iFlags>>4;
          bool bNewLap=false;

          wchar_t s[0xFF];

          CLapTime *pCurrentLap=m_pRaceInfo->getRaceTime(iPlayer)->getCurrentLap();

          if (iPlayer==m_iPlayerIdx) {
            if (iCode==eMessageCheckpoint) {
              m_pPlayer->toggleCheckpoint(iCp,true);
              m_iNextCp[m_iPlayerIdx]++;
              if (m_iNextCp[m_iPlayerIdx]==m_lCheckpoints.end()) {
                m_iNextCp[m_iPlayerIdx]=m_lCheckpoints.begin();
              }
              m_pRespawn[m_iPlayerIdx]=(*m_iNextCp[m_iPlayerIdx]);

              if (pCurrentLap) {
                wchar_t s[0xFF];
                swprintf(s,L"Time: %.2f sec",((f32)(iTime))/1000);
                m_pPlayer->showCheckpointInfo(s,iTime,false,false);
              }
            }
            else
              if (iCode==eMessageLap) {
                m_pPlayer->setLapNo(iLapNo+1,m_pRaceInfo->getLaps());
                m_pPlayer->resetCheckpoints();
                m_pPlayer->toggleCheckpoint(iCp,false);
                m_iNextCp[m_iPlayerIdx]=m_lCheckpoints.begin();
                m_pRespawn[m_iPlayerIdx]=*m_iNextCp[m_iPlayerIdx];
              }
          }

          if (iCode==eMessageCheckpoint) {
            swprintf(s,L"Time: %.2f sec",((f32)(iTime))/1000);
          }
          else
            if (iCode==eMessageLap) {
              swprintf(s,L"Laptime: %.2f sec",((f32)iTime)/1000);
              if (pCurrentLap) {
                pCurrentLap->endLap(iGameTime);
                pCurrentLap->setEndTime(iTime+1);
                pCurrentLap->setStartTime(1);
                if (m_iPlayerIdx==iPlayer) m_pPlayer->showCheckpointInfo(s,iGameTime,iFlags&1,iFlags&2);
              }
              bNewLap=true;
              CLapTime *pNewLap=new CLapTime(iGameTime,iPlayer);
              m_pRaceInfo->getRaceTime(iPlayer)->addLap(pNewLap);
            }
            else
              if (iCode==eMessageFastestLap) {
                wchar_t s[0xFF];
                swprintf(s,L"Fastest Lap: %.2f (%s)",((f32)iTime)/1000,m_aPlayerNames[iPlayer].c_str());
                m_pGame->showFastestLap(s);
                m_pRaceInfo->setFastestLap(m_pRaceInfo->getRaceTime(iPlayer)->getCurrentLap());
              }
              else
                if (iCode==eMessagefinished) {
                  m_pRaceInfo->getRaceTime(iPlayer)->endRace(iGameTime);
                  m_pRaceInfo->getRaceTime(iPlayer)->setStartTime(1);
                  m_pRaceInfo->getRaceTime(iPlayer)->setEndTime(iTime+1);
                  if (m_iPlayerIdx==iPlayer) {
                    wchar_t s[0xFF];
                    swprintf(s,L"Racetime: %.2f sec",((f32)m_pRaceInfo->getRaceTime(iPlayer)->getTime())/1000);
                    m_pPlayer->setInfoText(s);

                    m_pPlayer->setFinished(true);
                    m_pPlayer->setActive(false);
                  }
                }
        }
        break;
    }
  }

  return true;
}

bool CGameLogicClient::handlesEvent(IIrrOdeEvent *pEvent) {
  return true;
}

void CGameLogicClient::setTriangleSelector(IMetaTriangleSelector *pSelector) {
  m_pSelector=pSelector;
}

void CGameLogicClient::setTimer(ITimer *pTimer) {
  m_pTimer=pTimer;
}

void CGameLogicClient::respawn(CIrrOdeBody *pMarble, ICameraSceneNode *pCam, u32 idx) {
  m_pController->setPosition(pMarble,m_pRespawn[idx]->getAbsolutePosition());
  m_pController->setLinearVelocity(pMarble,vector3df(0.0f,0.0f,0.0f));
  m_pController->setAngularVeclocity(pMarble,vector3df(0.0f,0.0f,0.0f));

  pCam->setPosition(pMarble->getPosition()-5*m_pRespawn[idx]->getLookAt());
  vector3df rot=(-m_pRespawn[idx]->getLookAt()).getHorizontalAngle();
  pCam->setTarget(pMarble->getPosition());

  list<ISceneNodeAnimator *> animators=pCam->getAnimators();
  list<ISceneNodeAnimator *>::Iterator it;

  for (it=animators.begin(); it!=animators.end(); it++)
    ((CMarbleFollowCameraAnimator *)(*it))->setAngleH(rot.Y);
}

void CGameLogicClient::setRaceInfo(CRaceInfo *pInfo) {
  m_pRaceInfo=pInfo;

  printf("%i players\n",pInfo->getPlayerNo());
  for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) m_aPlayerNames[i]=m_pRaceInfo->getPlayerName(i).c_str();
}

CRaceInfo *CGameLogicClient::getRaceInfo() {
  return m_pRaceInfo;
}

const wchar_t *CGameLogicClient::getHiScoreFile() {
  return m_pRaceInfo->getHiScoreFile();
}

u32 CGameLogicClient::getGameState() {
  return m_iState;
}

void CGameLogicClient::setGameState(u32 iState) {
  m_iState=iState;
}

void CGameLogicClient::onPeerConnect(u16 iPlayerId) {
  printf("%i connected\n",iPlayerId);
}

void CGameLogicClient::onPeerDisconnect(u16 iPlayerId) {
  printf("%i disconnected\n",iPlayerId);
}

void CGameLogicClient::findCheckpoint(ISceneNode *pNode) {
  if (pNode->getType()==RESPAWN_NODE_ID && pNode->getParent()) {
    printf("Respawn node found .. %i\n",((CRespawnNode *)pNode)->getNum());
    CRespawnNode *pRespawn=(CRespawnNode *)pNode;
    list<CRespawnNode *>::Iterator it;
    bool bFound=false;
    for (it=m_lCheckpoints.begin(); it!=m_lCheckpoints.end(); it++)
      if ((*it)->getNum()>pRespawn->getNum()) {
        m_lCheckpoints.insert_before(it,pRespawn);
        bFound=true;
      }

    if (!bFound) m_lCheckpoints.push_back(pRespawn);
  }

  list<ISceneNode *> childList=pNode->getChildren();
  list<ISceneNode *>::Iterator it;

  for (it=childList.begin(); it!=childList.end(); it++) findCheckpoint(*it);
}

void CGameLogicClient::setNetAdapter(CIrrOdeNetAdapter *pAdapter) {
  m_pNetAdapter=pAdapter;
  m_pNetAdapter->addConnectionHandler(this);

  CIrrOdeNetController *pController;
  pController=new CIrrOdeNetController();
  pController->setNetAdapter(pAdapter);
  m_pController=pController;
}

void CGameLogicClient::resetPlayerObject() {
  if (m_pPlayer) {
    delete m_pPlayer;
    m_pPlayer=NULL;
  }
}

void CGameLogicClient::quitState() {
  CRenderToTextureManager::getSharedInstance()->setActive(false);
}
