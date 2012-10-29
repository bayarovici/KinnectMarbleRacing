  #include <GameClasses/CMarbleFollowAnimator.h>
  #include <GameClasses/CNetPlayer.h>
  #include <GameClasses/CGameLogic.h>
  #include <GameClasses/CRaceInfo.h>
  #include <GameClasses/CPlayer.h>
  #include <GameClasses/CGhostRecorder.h>

  #include <IrrOdeNet/CEventInit.h>
  #include <IrrOdeNet/NetMessages.h>
  #include <IrrOdeNet/CEventPlayer.h>
  #include <IrrOdeNet/CEventCpInfo.h>
  #include <IrrOdeNet/CMarbles2Recorder.h>
  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/CMarbles2WorldObserver.h>

  #include <CIrrOdeController.h>
  #include <CSerializer.h>
  #include <CRespawnNode.h>
  #include <CMessage.h>
  #include <COptions.h>

  #include <CGame.h>

CGameLogic::CGameLogic() {
  m_pGame=NULL;
  m_pDevice=NULL;
  m_pNetAdapter=NULL;

  m_iState=eGameStateStarting;
  m_iTime=0;
  m_iStartTime=0;
  m_iPlayersFinished=0;

  m_pController=new CIrrOdeController();
  m_pGhost=NULL;
}

CGameLogic::~CGameLogic() {
 list<CPlayer *>::Iterator it;

  while (m_lPlayers.getSize()>0) {
    it=m_lPlayers.begin();
    CPlayer *pPlayer=*it;
    m_lPlayers.erase(it);
    delete pPlayer;
  }

  CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
  delete m_pController;

  if (m_pNetAdapter) {
    m_pNetAdapter->removeConnectionHandler(this);
    for (u32 i=0; i<m_pNetAdapter->getPeerCount(); i++) {
      CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i);
      if (plr) plr->setState(ePlayerConnected);
    }
    m_pNetAdapter=NULL;
  }
}

void CGameLogic::findCheckpoint(ISceneNode *pNode) {
  if (pNode->getType()==RESPAWN_NODE_ID && pNode->getParent()) {
    printf("Respawn node found .. %i\n",((CRespawnNode *)pNode)->getNum());
    CRespawnNode *pRespawn=(CRespawnNode *)pNode;
    list<CRespawnNode *>::Iterator it;
    bool bFound=false;
    for (it=m_lCheckpoints.begin(); it!=m_lCheckpoints.end() && !bFound; it++)
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

void CGameLogic::setGame(CGame *pGame) {
  m_pGame=pGame;
  m_pGame->setGameLogic(this);
}

void CGameLogic::init() {
  if (m_pGame && m_pRaceInfo) {
    CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
    m_pOdeManager=CIrrOdeManager::getSharedInstance();

    if (m_pNetAdapter) {
      m_iState=eGameStateInitGame;
      m_pGame->setGameState(m_iState);
    }

    stringw sLevelName=stringw(m_pRaceInfo->getLevel());
    m_pGame->loadLevel(sLevelName.c_str());

    if (m_pNetAdapter) {
      m_pNetAdapter->addConnectionHandler(this);
      CEventServerMessage cMsg(eNetLoadLevel);
      cMsg.setData1(m_pRaceInfo->getLaps());
      m_pNetAdapter->sendPacket(&cMsg);
      m_pNetAdapter->update();
      m_iState=eGameStateWaitingPlayers;
      m_pGame->setGameState(m_iState);
      m_pNetAdapter->setSceneManager(m_pDevice->getSceneManager());
      m_pNetAdapter->setCommandVerify(this);
      m_pGame->setWaitingImageVisibility(true);

      m_pNetAdapter->resetCounters();
      if (m_pNetAdapter->isServer()) {
        CEventServerMessage cMsg(eNetStartRace);
        m_pNetAdapter->sendPacket(&cMsg);
      }
    }
    else {
      if (m_pRaceInfo->getGhost())
        m_pGame->setGhostFile(m_pRaceInfo->getGhostFile());

      m_iState=eGameStateReady1;
    }

    m_pGame->setNoViewports(m_pNetAdapter?1:m_pRaceInfo->getPlayerNo(),m_pRaceInfo->getSplitHorizontal());

    findCheckpoint(m_pDevice->getSceneManager()->getRootSceneNode());
    printf("%i checkpoints found\n",m_lCheckpoints.getSize());

    if (m_lCheckpoints.getSize()<2) {
      wchar_t s[0xFF];
      swprintf(s,L"invalid level: not enough checkpoints (%i found - 2 required)!",m_lCheckpoints.getSize());
      m_pGame->setInitError(s);
      return;
    }

    list<CRespawnNode *>::Iterator it;
    for (it=m_lCheckpoints.begin(); it!=m_lCheckpoints.end(); it++)
      printf("\t\tRespawn num: %i\n",(*it)->getNum());

    m_iTime=m_pTimer->getTime();
    m_iStartTime=m_iTime;

    if (m_iState==eGameStateReady1) {
      initGame();
      if (m_pNetAdapter) m_pNetAdapter->update();
    }
  }
  else m_iState=eGameStateStarting;
}

void CGameLogic::initGame() {
  m_pGame->initODE();

  core::rect<s32> viewPort=m_pDevice->getVideoDriver()->getViewPort();
  f32 fAr=1.0f*viewPort.getWidth()/viewPort.getHeight();
  if (!m_pNetAdapter) {
    if (m_pRaceInfo->getSplitHorizontal())
      fAr=fAr/m_pRaceInfo->getPlayerNo();
    else
      fAr=fAr*m_pRaceInfo->getPlayerNo();
  }

  for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) {
    f32 fPan=(m_pRaceInfo->getPlayerNo()==1 || m_pNetAdapter!=NULL)?0.0f:i==0?0.8f:-0.8f;
    printf("\n\t\t%.2f\n\n",fPan);
    CPlayer *pPlayer=new CPlayer(i+1,i+1,m_pDevice,fAr,m_pGame,fPan,m_pNetAdapter!=NULL);

    if (m_pNetAdapter && i>0) {
      c8 s[0xFF];
      sprintf(s,"player%i",i+1);
      ISceneNode *pNode=m_pDevice->getSceneManager()->getSceneNodeFromName(s);
      if (pNode) {
        CEventServerMessage cMsg(eNetClientPlayerId);
        cMsg.setData1(i);
        m_pNetAdapter->sendPacket(&cMsg,m_pNetAdapter->getPlayerIdByIndex(i-1));

        printf("\t\tplayerid==%i ... playernum=%i\n",m_pNetAdapter->getPlayerIdByIndex(i-1),i);

        CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i-1);
        if (plr) plr->setNodeId(pNode->getID());
      }
      else printf("node \"%s\" not found!\n",s);
    }

    if (!pPlayer->initialized()) {
      wchar_t s[0xFF];
      swprintf(s,L"invalid level: node \"player%i\" not found!",i+1);
      m_pGame->setInitError(s);
    }
    else {
      pPlayer->setTimer(m_pTimer);
      m_pGame->setCam(i,pPlayer->getCamera());

      if (i==0 || !m_pNetAdapter) {
        pPlayer->createPlayerGUI(m_pRaceInfo->getSplitHorizontal(),m_aPlayerNames[i].c_str(),m_lCheckpoints.getSize());
      }

      pPlayer->setController(m_pController);
      m_lPlayers.push_back(pPlayer);
      m_iNextCp[i]=m_lCheckpoints.begin();
      m_pRespawn[i]=*m_iNextCp[i];
    }

  }
  if (m_pNetAdapter) m_pNetAdapter->update();

  ISceneNode *pNode=NULL;
  u32 iPlayerNum=m_pRaceInfo->getPlayerNo();
  do {
    c8 sNodeName[0xFF];
    sprintf(sNodeName,"player%i",iPlayerNum+1);
    pNode=m_pDevice->getSceneManager()->getSceneNodeFromName(sNodeName);
    if (pNode) {
      removeFromPhysics(pNode);
      pNode->remove();
    }
    iPlayerNum++;
  }
  while (pNode!=NULL);

  if (m_pRaceInfo->getRecordReplay()) m_pGame->initReplayRecord();
}

void CGameLogic::setDevice(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
}

void CGameLogic::update() {
  if (m_iState!=eGameStateWaitingPlayers && m_iState!=eGameStateInitGame)
    m_pOdeManager->step();

  if (m_pNetAdapter) m_pNetAdapter->update();

  list<CPlayer *>::Iterator it;
  wchar_t s[0xFF];
  u32 i=0;

  if (m_pNetAdapter) {
    wchar_t sPing[0xFF],sSent[0xFF],sRecv[0xFF],sTrfi[0xFF];

    swprintf(sPing,L"Ping: %i",m_pNetAdapter->getPing());
    swprintf(sSent,L"Sent: %i (%i)",m_pNetAdapter->getSentCount(),m_pNetAdapter->getSentSize());
    swprintf(sRecv,L"Recv: %i (%i)",m_pNetAdapter->getRecvCount(),m_pNetAdapter->getRecvSize());
    swprintf(sTrfi,L"Traffic: %.2f",m_pNetAdapter->getTraffic());

    m_pGame->setNetInfoText(sPing,sSent,sRecv,sTrfi);
  }

  switch (m_iState) {
    case eGameStateWaitingPlayers: {
        bool bAllPlayersReady=true;
        //printf("number of peers: %i\n",m_pNetAdapter->getPeerCount());
        for (i=0; i<m_pNetAdapter->getPeerCount(); i++) {
          CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i);
          if (plr && plr->getState()!=ePlayerRaceReady) {
            //printf("%s\n",plr->getName());
            bAllPlayersReady=false;
          }
        }

        if (bAllPlayersReady) {
          initGame();
          m_iState=eGameStateReady1;
          if (m_pNetAdapter) m_pNetAdapter->update();
        }
      }
      break;

    case eGameStateReady1:
      m_iTime=m_pTimer->getTime();
      if (m_pGame->getRecorder()) m_pGame->getRecorder()->startRecording();
      if (m_pNetAdapter) m_pNetAdapter->startWorldUpdate();

      m_iState=eGameStateReady2;
      m_iCountdown=3;
      printf("Ready.\n");
      m_pGame->toggleCountdown(0);

      for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) {
        (*it)->setInfoText(L"Ready");
        if (m_pNetAdapter) break;
      }

      addReplayMessageInit(eMessageReady,255);
      for (u32 p=0; p<m_pRaceInfo->getPlayerNo(); p++) {
        CEventPlayer *pPlr=new CEventPlayer(p,stringc(m_pRaceInfo->getPlayerName(p)).c_str());
        CMarbles2WorldObserver::getSharedInstance()->addMessage(pPlr);
      }
      m_pGame->setWaitingImageVisibility(false);
      break;

    case eGameStateReady2:
      if (m_pTimer->getTime()-m_iTime>3000) m_iState=eGameStateCountdown1;
      break;

    case eGameStateCountdown1:
      m_iTime=m_pTimer->getTime();
     // m_iState=eGameStateCountdown2; go to countdown
	  m_iState = eGameStateGo;
      printf("%i\n",m_iCountdown);
      m_pGame->toggleCountdown(1+(3-m_iCountdown));
      swprintf(s,L"%i",m_iCountdown);
      for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) (*it)->setInfoText(s);
      addReplayMessageInit(eMessageCntdn,m_iCountdown);
      break;

    case eGameStateCountdown2:
      if (m_pTimer->getTime()-m_iTime>1000) {
        m_iCountdown--;
        if (m_iCountdown) m_iState=eGameStateCountdown1; else m_iState=eGameStateGo;
      }
      break;

    case eGameStateGo:
      printf("GO!\n");
      m_iStartTime=m_pTimer->getTime();
      m_pGame->toggleCountdown(4);
      i=0;
      for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) {
        (*it)->setActive(true);
        m_pRaceInfo->getRaceTime(i)->setStartTime(0);
        i++;
      }
      addReplayMessageInit(eMessageGo,m_iStartTime);

      if (m_lPlayers.getSize()==1 && m_bRecordGhost) {
        m_pGhost=new CGhostRecorder(m_pDevice);
        CPlayer *plr=*m_lPlayers.begin();
        if (plr) {
          m_pGhost->setData(plr->getMarble()->getID(),stringc(m_aPlayerNames[0]).c_str(),stringc(m_pRaceInfo->getLevel()).c_str());
        }
        m_pGhost->activate();
      }
      else {
        if (m_pGhost) delete m_pGhost;
        m_pGhost=NULL;
      }
      m_iState=eGameStatePlay;
      break;

    case eGameStatePlay: {
        u32 iGameTime=m_pTimer->getTime()-m_iStartTime;
        i=0;
        for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) {
          if (i==0 || !m_pNetAdapter) (*it)->updateGUI(iGameTime);

          aabbox3df cCp=(*m_iNextCp[i])->getParent()->getTransformedBoundingBox(),
                    cMb=(*it)->getMarble()->getTransformedBoundingBox();

          bool bCp=cCp.intersectsWithBox(cMb);
          if (bCp) {
            bool bNewLap=false;

            if (m_iNextCp[i]==m_lCheckpoints.begin()) {
              bNewLap=true;
              CLapTime *pCurrentLap=m_pRaceInfo->getRaceTime(i)->getCurrentLap();

              u32 iLap=m_pRaceInfo->getRaceTime(i)->getCurrentLapNo();
              (*it)->setLapNo(iLap+1,m_pRaceInfo->getLaps());

              bool bRaceFinished=iLap==m_pRaceInfo->getLaps();

              if (!bRaceFinished) m_pGame->restartGhost(); else m_pGame->stopGhost();
              if (!bRaceFinished && m_pRaceInfo->getPlayerNo()>1) {
                bRaceFinished=false;
                for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++)
                  bRaceFinished|=m_pRaceInfo->getRaceTime(i)->raceEnded();
              }

              if (bRaceFinished && m_pRaceInfo->getRaceTime(i)->getCurrentLapNo()>0) {
                printf("Race finished player %i\n",i);
                (*it)->setFinished(true);
                (*it)->setActive(false);

                m_pRaceInfo->getRaceTime(i)->endRace(iGameTime);

                wchar_t s[0xFF];
                swprintf(s,L"Racetime: %.2f sec",((f32)m_pRaceInfo->getRaceTime(i)->getTime())/1000);
                (*it)->setInfoText(s);

                CPlayer *plr=*it;
                CRespawnNode *rsp=*m_iNextCp[i];

                irr::ode::CIrrOdeBody *pMarble=reinterpret_cast<irr::ode::CIrrOdeBody *>(plr->getMarble());

                if (pMarble) {
                  pMarble->setAngularDamping(0.5f);
                }

                addReplayMessageCpInfo(eMessagefinished,i,plr->getMarble()->getID(),m_pRaceInfo->getRaceTime(i)->getTime(),rsp->getNum(),0);
              }

              CLapTime *pNewLap=new CLapTime(iGameTime,i);
              if (pCurrentLap) {
                pCurrentLap->endLap(iGameTime);
              }

              bool bFastest=m_pRaceInfo->getRaceTime(i)->addLap(pNewLap);

              if (bFastest) {
                if (m_pRaceInfo->getFastestLap()==NULL || m_pRaceInfo->getFastestLap()->getTime()>pCurrentLap->getTime()) {
                  m_pRaceInfo->setFastestLap(pCurrentLap);
                  wchar_t s[0xFF];
                  swprintf(s,L"Fastest Lap: %.2f (%s)",((f32)pCurrentLap->getTime())/1000,m_aPlayerNames[i].c_str());
                  m_pGame->showFastestLap(s);

                  CPlayer *plr=*it;
                  CRespawnNode *rsp=*m_iNextCp[i];

                  addReplayMessageCpInfo(eMessageFastestLap,i,plr->getMarble()->getID(),iGameTime-pCurrentLap->getStartTime(),rsp->getNum(),0);
                }
              }

              if (pCurrentLap) {
                wchar_t s[0xFF];
                swprintf(s,L"Laptime: %.2f sec",((f32)pCurrentLap->getTime())/1000);
                (*it)->showCheckpointInfo(s,iGameTime,bFastest,bRaceFinished);
              }

              CPlayer *plr=*it;
              CRespawnNode *rsp=*m_iNextCp[i];

              u8 iFlags=iLap<<4;
              if (bFastest) iFlags+=1;
              if (bRaceFinished) iFlags+=2;

              printf("**** %i\n",rsp->getNum());

              u32 iTimeStamp=pCurrentLap?iGameTime-pCurrentLap->getStartTime():iGameTime;
              addReplayMessageCpInfo(eMessageLap,i,plr->getMarble()->getID(),iTimeStamp,rsp->getNum(),iFlags);
              (*it)->resetCheckpoints();
            }
            else {
              CLapTime *pCurrentLap=m_pRaceInfo->getRaceTime(i)->getCurrentLap();
              wchar_t s[0xFF];
              pCurrentLap->setEndTime(iGameTime);
              swprintf(s,L"Time: %.2f sec",((f32)(iGameTime-pCurrentLap->getStartTime()))/1000);
              (*it)->showCheckpointInfo(s,iGameTime,false,false);

              CPlayer *plr=*it;
              CRespawnNode *rsp=*m_iNextCp[i];

              addReplayMessageCpInfo(eMessageCheckpoint,i,plr->getMarble()->getID(),iGameTime-pCurrentLap->getStartTime(),rsp->getNum(),0);
            }

            CLapTime *pCurrentLap=m_pRaceInfo->getRaceTime(i)->getCurrentLap();
            CSectionTime *pNewCp=new CSectionTime(iGameTime);
            pCurrentLap->addCheckpoint(pNewCp);
            (*it)->toggleCheckpoint(pCurrentLap->getCurrentCheckpointNo()-1,!bNewLap);

            printf("player %i: checkpoint! %i\n",i,(*m_iNextCp[i])->getNum());
            m_pRespawn[i]=(*m_iNextCp[i]);
            m_iNextCp[i]++;

            if (m_iNextCp[i]==m_lCheckpoints.end()) { printf("XYZ\n"); m_iNextCp[i]=m_lCheckpoints.begin(); }
          }
          i++;
        }
        {
          bool bGameOver=true;

          u32 iCnt=m_pNetAdapter?m_pNetAdapter->getPeerCount()+1:m_pRaceInfo->getPlayerNo();
          for (u32 plr=0; plr<iCnt; plr++)
            bGameOver&=m_pRaceInfo->getRaceTime(plr)->raceEnded();

          if (bGameOver) m_iState=eGameStateGameOver1;
        }
      }
      break;

    case eGameStateGameOver1:
      printf("game over!\n");
      m_pGame->showGameOver();
      m_iTime=m_pTimer->getTime();
      printf("fastest lap: %.2f\n",m_pRaceInfo->getFastestLap()?((f32)m_pRaceInfo->getFastestLap()->getTime())/1000:0.0f);
      printf("calc\n");
      m_pRaceInfo->calculateResult();
      printf("Ready.\n");

      if (m_pNetAdapter) {
        CEventServerMessage cMsg(eMessageGameOver);
        m_pNetAdapter->sendPacket(&cMsg);
        m_pNetAdapter->update();
      }

      if (m_pRaceInfo->raceFinished())
        for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) {
          u32 iPos=m_pRaceInfo->getPlayerAtFinishedPosition(i);
          printf("Position %i: \"%s\"\n",i,stringc(m_aPlayerNames[iPos]).c_str());
        }
      else
        printf("Race cancelled\n");

      if (m_pGhost) m_pGhost->deactivate();
      m_iState=eGameStateGameOver2;
      break;

    case eGameStateGameOver2:
      if (m_pTimer->getTime()-m_iTime>5000) m_iState=eGameStateLapList;
      break;

    case eGameStateLapList: {
        const wchar_t *sPlayers[4];

        for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) sPlayers[i]=m_aPlayerNames[i].c_str();

        for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) (*it)->clearGUI();

        m_pGame->showRaceInfo(m_pRaceInfo->getRaceTimeArray(),m_pRaceInfo->getFastestLap(),m_pRaceInfo->getPlayerNo(),(const wchar_t **)sPlayers);
        if (m_pGame->getRecorder()) m_pGame->getRecorder()->stopRecording();
        if (m_pNetAdapter) {
          CEventServerMessage cMsg(eMessageShowLapList);
          m_pNetAdapter->sendPacket(&cMsg);
          m_pNetAdapter->update();

          m_pNetAdapter->stopWorldUpdate();
          m_pNetAdapter->setCommandVerify(NULL);
        }
        if (m_pGhost && m_pGhost->getLapCount()>0) {
          m_pDevice->getCursorControl()->setVisible(true);
          m_pDevice->getGUIEnvironment()->addMessageBox(L"Ghost",L"Save recorded Ghost?",true,irr::gui::EMBF_YES|irr::gui::EMBF_NO,0,4723);
        }

        m_iState=eGameStateFinished;
      }
      break;
  }
}

void CGameLogic::removeFromPhysics(ISceneNode *pNode) {
  if (pNode->getType()==IRR_ODE_BODY_ID) {
    CIrrOdeBody *p=reinterpret_cast<CIrrOdeBody *>(pNode);
    p->removeFromPhysics();
  }
}

bool CGameLogic::onEvent(IIrrOdeEvent *pEvent) {
  bool bRet=false;

  if (pEvent->getType()==irr::ode::eIrrOdeEventStep) {
    list<CPlayer *>::Iterator it;
    for (it=m_lPlayers.begin(); it!=m_lPlayers.end(); it++) {
      (*it)->step();
      if (m_pNetAdapter) break;
    }
    bRet=true;
  }

  if (pEvent->getType()==eNetClientMessage) {
    CEventClientMessage *p=(CEventClientMessage *)pEvent;
    if (p->getCode()==eNetClientRaceReady) {
      printf("\n\t\tplayer %i race ready!\n\n",p->getPlayerId());
      CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerData(p->getPlayerId());
      plr->setState(ePlayerRaceReady);
      bRet=true;
    }
  }

  return bRet;
}

bool CGameLogic::handlesEvent(IIrrOdeEvent *pEvent) {
  return true;
}

void CGameLogic::setTriangleSelector(IMetaTriangleSelector *pSelector) {
  m_pSelector=pSelector;
}

void CGameLogic::setTimer(ITimer *pTimer) {
  m_pTimer=pTimer;
}

void CGameLogic::respawn(CIrrOdeBody *pMarble, ICameraSceneNode *pCam, u32 idx) {
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

void CGameLogic::setRaceInfo(CRaceInfo *pInfo) {
  m_pRaceInfo=pInfo;

  for (u32 i=0; i<m_pRaceInfo->getPlayerNo(); i++) m_aPlayerNames[i]=m_pRaceInfo->getPlayerName(i).c_str();
}

CRaceInfo *CGameLogic::getRaceInfo() {
  return m_pRaceInfo;
}

const wchar_t *CGameLogic::getHiScoreFile() {
  return m_pRaceInfo->getHiScoreFile();
}

u32 CGameLogic::getGameState() {
  return m_iState;
}

void CGameLogic::setGameState(u32 iState) {
  m_iState=iState;
}

void CGameLogic::addReplayMessageInit(u16 iMessageCode, u32 iData) {
  CEventInit *pEvt=new CEventInit(iMessageCode,iData);
  CMarbles2WorldObserver::getSharedInstance()->addMessage(pEvt);
}

void CGameLogic::addReplayMessageCpInfo(u16 iMessageCode, u8 iPlayer, u32 iId, u32 iTime, u8 iCp, u8 iFlags) {
  CEventCpInfo *pEvt=new CEventCpInfo(iMessageCode,iPlayer,iId,iTime,iCp,iFlags);
  CMarbles2WorldObserver::getSharedInstance()->addMessage(pEvt);
}

void CGameLogic::onPeerDisconnect(u16 iPlayerId) {
  u32 iDisconnectPos=m_pNetAdapter->getIndexOfPeer(iPlayerId);
  CNetPlayer *pData=(CNetPlayer *)m_pNetAdapter->getPeerData(iPlayerId);
  if (pData) delete pData;

  printf("player %i disconnected\n",iPlayerId);

  if (m_pNetAdapter->isServer()) {
    CEventServerMessage cMsg(eMessagePlayerDisconnected);
    cMsg.setData1(iDisconnectPos);
    m_pNetAdapter->sendPacket(&cMsg);
  }
}

void CGameLogic::onPeerConnect(u16 iPlayerId) {
  CEventServerMessage cMsg(eNetGameRunning);
  m_pNetAdapter->sendPacket(&cMsg,iPlayerId);
  m_pNetAdapter->update();
}

bool CGameLogic::validCommand(u16 iPlayerId, u16 iCommand, u16 iBodyId) {
  bool bRet=false;
  CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerData(iPlayerId);
  if (plr) bRet=iBodyId==plr->getNodeId();
  return bRet;
}

void CGameLogic::quitState() {
}

void CGameLogic::saveGhost() {
  if (m_pGhost) m_pGhost->save();
}
