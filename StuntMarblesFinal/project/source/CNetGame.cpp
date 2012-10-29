  #include <irrlicht.h>
  #include <CNetGame.h>
  #include <CStateMachine.h>
  #include <CSerializer.h>
  #include <CTournamentRace.h>
  #include <COptions.h>
  #include <CGame.h>

  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/NetMessages.h>

  #include <GameClasses/CRaceInfo.h>
  #include <GameClasses/CGameLogic.h>
  #include <GameClasses/CLevelList.h>
  #include <GameClasses/CNetPlayer.h>
  #include <GameClasses/CTournamentInfo.h>
  #include <GameClasses/CGameLogicClient.h>

CNetGame::CNetGame(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : CTournamentBase(pDevice,pStateMachine) {
  m_sServer=stringw("");
  m_eState=eSelectState;

  m_pNetAdapter=NULL;
  m_pLogic=NULL;
  m_pInfo=NULL;

  m_pServerPlayer=NULL;
  m_pLevelList=NULL;

  m_bRecordReplay=false;
  m_bNetStarted=false;
  m_bTournamentInit=false;
}

void CNetGame::activate(IState *pPrevious) {
  printf("adding event listener\n");
  if (pPrevious!=this) CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
  m_pDevice->getSceneManager()->loadScene("data/setup_scene.xml");
  m_pDevice->setEventReceiver(this);
  m_pSndEngine=m_pStateMachine->getSoundEngine();

  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();

  m_pDevice->getCursorControl()->setVisible(true);

  dimension2du screenSize=m_pDriver->getScreenSize();
  s32 iLeftX=screenSize.Width/2-400,iRightX=screenSize.Width/2+400;
  m_pTab=m_pGuienv->addTab(rect<s32>(iLeftX,0,iRightX,600));

  IGUIFont *pBigFont=m_pGuienv->getFont("data/font2.xml");
  IGUIStaticText *pHeadline=m_pGuienv->addStaticText(L"Kinect Marble Racers - Net Game",rect<s32>(10,10,790,40),false,true,m_pTab);
  pHeadline->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  pHeadline->setOverrideFont(pBigFont);
  pHeadline->setBackgroundColor(SColor(128,16,16,16));
  pHeadline->setOverrideColor(SColor(255,255,118,70));

  m_pEdServer=NULL;
  m_pCltPort=NULL;
  m_pSrvPort=NULL;

  switch (m_eState) {
    case eSelectState: {
        m_pServer=m_pGuienv->addButton(rect<s32>(175,100,350,120),m_pTab,101,L"Start Server");
        m_pClient=m_pGuienv->addButton(rect<s32>(175,200,350,220),m_pTab,102,L"Connect to Server");
        IGUIStaticText *t=m_pGuienv->addStaticText(L"Server's IP address:",rect<s32>(100,228,270,252),false,false,m_pTab,-1,true);
        t->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
        m_pEdServer=m_pGuienv->addEditBox(m_sServer.c_str(),rect<s32>(275,228,475,252),true,m_pTab);
        t=m_pGuienv->addStaticText(L"PortNo:",rect<s32>(480,228,550,252),false,false,m_pTab,-1,true);
        t->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
        m_pCltPort=m_pGuienv->addEditBox(m_sCltPort.c_str(),rect<s32>(555,228,625,252),true,m_pTab);
        m_pSrvPort=m_pGuienv->addEditBox(m_sSrvPort.c_str(),rect<s32>(275,128,345,152),true,m_pTab);
        t=m_pGuienv->addStaticText(L"Server PortNo:",rect<s32>(100,128,270,152),false,false,m_pTab,-1,true);
        t->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
      }
      break;

    case eServer: {
        m_bNetClient=false;
        CTournamentBase::activate(pPrevious);
        m_bTournamentInit=true;

        m_iLaps=2;
        m_iPlayersConnected=1;

        if (m_pServerPlayer==NULL) {
          m_pServerPlayer=new CNetPlayer(0);
          m_pServerPlayer->setName(stringc(stringw(m_pOptions->getPlayerName(0))).c_str());
        }

        c8 s2[0xFF];
        rect<s32> cRect=rect<s32>(10,140,250,160);
        sprintf(s2,"Player 1: %s",stringc(stringw(m_pOptions->getPlayerName(0))).c_str());
        IGUIStaticText *pText=m_pGuienv->addStaticText(stringw(s2).c_str(),cRect,false,true,m_pTab,-1,true);
        pText->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

        for (u32 i=0; i<m_pNetAdapter->getPeerCount(); i++) {
          CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i);
          addPlayerGUI(i+1,plr->getName());
        }
        m_pNetAdapter->addConnectionHandler(this);
      }
      break;

    case eClient: {
        m_bNetClient=true;
        CTournamentBase::activate(pPrevious);
        m_bTournamentInit=true;
        m_pLevelList=new CLevelList(m_pDevice);
        m_pWait=m_pGuienv->addStaticText(L"Waiting for server..",rect<s32>(10,95,250,135),false,true,m_pTab);
        m_pWait->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
        m_pWait->setOverrideFont(pBigFont);
        m_pWait->setBackgroundColor(SColor(128,16,16,16));
        m_pWait->setOverrideColor(SColor(255,255,118,70));
        m_pNetAdapter->setGlobalPacketRelay(true);
        printf("\n\t\tpeer count: %i\n\n",m_pNetAdapter->getPeerCount());
        for (u32 iPlr=0; iPlr<m_pNetAdapter->getPeerCount(); iPlr++) {
          CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(iPlr);
          addPlayerGUI(iPlr,plr->getName());
        }
        m_pNetAdapter->addConnectionHandler(this);
      }
      break;
  }

  m_bStartRace=false;
  m_iSelect=0;
}

void CNetGame::deactivate(IState *pNext) {
  if (m_bTournamentInit) {
    CTournamentBase::deactivate(pNext);
    m_bTournamentInit=false;
  }

  if (m_pEdServer) m_sServer=stringw(m_pEdServer->getText());
  if (m_pCltPort) m_sCltPort=stringw(m_pCltPort->getText());
  if (m_pSrvPort) m_sSrvPort=stringw(m_pSrvPort->getText());

  m_pDevice->getSceneManager()->clear();
  m_pGuienv->clear();
  m_aPlayerNames.clear();
  if (m_pLevelList) {
    delete m_pLevelList;
    m_pLevelList=NULL;
  }

  if (m_pNetAdapter) m_pNetAdapter->removeConnectionHandler(this);
  printf("removing event listener\n");
  if (pNext!=this) if (pNext!=this) CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
}

u32 CNetGame::update() {
  if (m_eState!=eSelectState) {
    m_pNetAdapter->update();
  }
  return m_iSelect;
}

bool CNetGame::OnEvent(const SEvent &event) {
  bool bRet=false;
  if (event.EventType==EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown && event.KeyInput.Key==KEY_ESCAPE) {
    m_iSelect=1;
    m_eState=eSelectState;
    if (m_pNetAdapter) {
      m_pNetAdapter->close();
      m_pNetAdapter=NULL;
      m_bNetStarted=false;
    }
    bRet=true;
  }

  if (event.EventType==EET_GUI_EVENT ) {
    if (event.GUIEvent.EventType==EGET_EDITBOX_CHANGED && event.GUIEvent.Caller==m_pEdServer) {
      m_sServer=stringw(m_pEdServer->getText());
      printf("\t\t--> %s\n",stringc(m_sServer).c_str());
    }

    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
      u32 id=event.GUIEvent.Caller->getID(),iPort;

      switch (m_eState) {
        case eSelectState:
          switch (id) {
            case 101:
              printf("start server\n");

              m_pNetAdapter=CIrrOdeNetAdapter::getSharedInstance();
              m_pNetAdapter->setTimer(m_pDevice->getTimer());
              m_pNetAdapter->addConnectionHandler(this);

              m_eState=eServer;
              m_iSelect=this->getNum()+1;
              iPort=atoi(stringc(m_sSrvPort).c_str());
              printf("starting server ... portNo=%i\n",iPort);
              m_pNetAdapter->createServer(iPort,5000000,5000000,4);
              m_pNetAdapter->getPing(0);
              m_pNetAdapter->setGlobalPacketRelay(true);
              m_bNetStarted=true;
              break;

            case 102:
              printf("connect to server\n");

              m_pNetAdapter=CIrrOdeNetAdapter::getSharedInstance();
              m_pNetAdapter->setTimer(m_pDevice->getTimer());
              m_pNetAdapter->addConnectionHandler(this);

              m_eState=eClient;
              m_iSelect=this->getNum()+1;
              iPort=atoi(stringc(m_sCltPort).c_str());
              printf("connecting to server %s:%i\n",stringc(m_sServer).c_str(),iPort);
              m_pNetAdapter->createClient(stringc(m_sServer).c_str(),iPort,5000000,5000000,10000);
              m_bNetStarted=true;
              break;
            }
            break;
          default:
            break;
        }
    }
  }

  bool b=CTournamentBase::OnEvent(event);
  bRet|=b;

  return bRet;
}

void CNetGame::readConfig(IXMLReaderUTF8 *pXml) {
  if (pXml && !strcmp(pXml->getNodeName(),"netgame") && pXml->getNodeType()==EXN_ELEMENT) {
    while (pXml->read() && strcmp(pXml->getNodeName(),"netgame")) {
      CTournamentBase::readConfig(pXml);
      if (!strcmp(pXml->getNodeName(),"server")) {
        m_sServer=stringw(pXml->getAttributeValue("ip")).c_str();
        m_sCltPort=stringw(pXml->getAttributeValue("port_client")).c_str();
        m_sSrvPort=stringw(pXml->getAttributeValue("port_server")).c_str();
      }
    }
  }
}

void CNetGame::writeConfig(IXMLWriter *pXml) {
  if (pXml) {
    pXml->writeElement(L"netgame",false);
    pXml->writeLineBreak();

    CTournamentBase::writeConfig(pXml);

    pXml->writeElement(L"server",true,L"ip",m_sServer.c_str(),L"port_client",m_sCltPort.c_str(),L"port_server",m_sSrvPort.c_str());
    pXml->writeLineBreak();

    pXml->writeClosingTag(L"netgame");
    pXml->writeLineBreak();
  }
}

bool CNetGame::onEvent(IIrrOdeEvent *pEvent) {
  printf("CNetGame::onEvent\n");
  bool bRet=false;

  switch (m_eState) {
    case eClient:
      if (pEvent->getType()==eNetServerMessage) {
        CEventServerMessage *p=(CEventServerMessage *)pEvent;
        printf("\tserver message received ... code=%i\n",p->getCode());
        switch (p->getCode()) {
          case eNetStartTournament:
            if (m_pRace && CTournamentBase::startTournament()) {
              m_pRace->setNetAdapter(m_pNetAdapter);
              m_iSelect=8;
            }
            bRet=true;
            break;

          case eNetTournamentName:
            m_pName->setText(stringw(p->getData2()).c_str());
            m_pOptions->setServerNetbook(p->getData1()!=0);
            printf("\n\t\tserver netbook level: %s\n\n",(p->getData1()!=0)?"true":"false");
            bRet=true;
            break;

          case eNetAddRace: {
              CTournamentRaceGUI *pRace=new CTournamentRaceGUI(m_pDevice,p->getData1(),m_pRaceTab,false);
              m_lRaces.push_back(pRace);
              bRet=true;
            }
            break;

          case eNetDelRace: {
              list<CTournamentRaceGUI *>::Iterator it;
              for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
                if (p->getData1()==(*it)->getId()) {
                  CTournamentRaceGUI *p=*it;
                  m_lRaces.erase(it);
                  delete p;
                  u32 cnt=0;
                  for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) (*it)->setId(cnt++);
                  return true;
                }
              }
            }
            bRet=true;
            break;

          case eNetClearList: {
              list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
              while (m_lRaces.getSize()>0) {
                CTournamentRaceGUI *p=*it;
                m_lRaces.erase(it);
                delete p;
                it=m_lRaces.begin();
              }
              m_lRaces.clear();
            }
            bRet=true;
            break;

          case eNetSetRaceLaps: {
              list<CTournamentRaceGUI *>::Iterator it;
              for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
                if (p->getData1()+23==(s32)(*it)->getId()) {
                  CTournamentRaceGUI *pGui=*it;
                  pGui->setLaps(p->getData3());
                  return true;
                }
              }
              bRet=true;
            }
            break;

          case eNetSetRaceTrack: {
              list<CTournamentRaceGUI *>::Iterator it;
              for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
                if ((u32)p->getData1()+23==(*it)->getId()) {
                  CTournamentRaceGUI *pGUI=*it;
                  pGUI->setTrack(stringw(p->getData2()).c_str());
                  return true;
                }
              }
            }
            bRet=true;
            break;

          case eNetQueryPlayerName: {
              printf("\tsending player name to server...\n");
              CEventClientMessage cMsg(eNetPlayerName);
              cMsg.setData2(stringc(stringw(m_pOptions->getPlayerName(0))).c_str());
              m_pNetAdapter->sendPacket(&cMsg);
            }
            break;

          case eNetSetPlayerName: {
              printf("setting player name %i: \"%s\"\n",p->getData1(),p->getData2());

              CNetPlayer *plr=new CNetPlayer(p->getData1());
              plr->setName(p->getData2());
              m_pNetAdapter->clientAddPeerData(p->getData1(),plr);

              addPlayerGUI(p->getData1(),p->getData2());
            }
            break;

          case eMessagePlayerDisconnected:
            printf("player %i disconnected!\n",p->getData1()+1);
            onPeerDisconnect(p->getData1());
            break;

          case eMessageServerFull:
            printf("server full!\n");
            m_pWait->setText(L"Server full!");
            break;

          case eNetGameRunning:
            printf("game running!\n");
            m_pWait->setText(L"Game started!");
            break;
        }
      }
      break;

    case eServer: {
        CEventClientMessage *p=(CEventClientMessage *)pEvent;
        if (p->getCode()==eNetPlayerName) {
          s32 i=m_pNetAdapter->getNumOfPeer(p->getPlayerId())+1;
          CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerData(p->getPlayerId());
          printf("player %s connected (id=%i)\n",p->getData2(),i);
          if (plr && i!=0) {
            plr->setState(ePlayerConnected);

            plr->setName(p->getData2());
            addPlayerGUI(i,p->getData2());

            CEventServerMessage cMsg(eNetSetPlayerName);
            cMsg.setData1(0);
            cMsg.setData2(m_pServerPlayer->getName());
            m_pNetAdapter->sendPacket(&cMsg,p->getPlayerId());

            for (u32 j=0; j<m_pNetAdapter->getPeerCount(); j++) {
              plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(j);
              printf("\tgetPeerDataByIndex(%i): %i\n",j+1,(int)plr);
              if (plr) {
                cMsg.resetSerializer();
                cMsg.setData1(j+1);
                cMsg.setData2(plr->getName());
                m_pNetAdapter->sendPacket(&cMsg);
              }
            }
          }
          else printf("player id %i not found!\n",p->getPlayerId());

          sendClearList(p->getPlayerId());
          list<CTournamentRaceGUI *>::Iterator it;
          u32 idx=0;
          for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
            CTournamentRaceGUI *pGUI=*it;
            sendAddTrack(p->getPlayerId(),idx);
            sendTrackChanged(p->getPlayerId(),idx,stringc(pGUI->getTrack()).c_str());
            sendLapsChanged(p->getPlayerId(),idx,pGUI->getLaps());
            idx++;
          }
          tournamentNameChanged();
        }
      }
      break;

    case eSelectState:
      break;
  }

  return bRet;
}

void CNetGame::onPeerConnect(u16 iPlayerId) {
  if (m_eState==eServer) {
    CNetPlayer *pPlayer=new CNetPlayer(iPlayerId);
    m_pNetAdapter->setPeerData(iPlayerId,pPlayer);
    m_pNetAdapter->update();

    CEventServerMessage cMsg(eNetQueryPlayerName);
    m_pNetAdapter->sendPacket(&cMsg,iPlayerId);

    m_iPlayersConnected++;
  }
}

void CNetGame::onPeerDisconnect(u16 iPlayerId) {
  CNetPlayer *pData=(CNetPlayer *)m_pNetAdapter->getPeerData(iPlayerId);
  if (pData) delete pData;

  for (u32 i=0; i<m_aPlayerNames.size(); i++) m_aPlayerNames[i]->setVisible(false);

  u32 iPos=0,iDisconnectPos=0;
  for (u32 i=0; i<m_pNetAdapter->getPeerCount(); i++)
    if (m_pNetAdapter->getIdOfPeerAtIndex(i)!=iPlayerId) {
      CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i);
      addPlayerGUI(iPos+1,plr->getName());
      iPos++;
    }
    else iDisconnectPos=i;

  if (m_eState==eServer) {
    CEventServerMessage cMsg(eMessagePlayerDisconnected);
    cMsg.setData1(iDisconnectPos);
    m_pNetAdapter->sendPacket(&cMsg);

    m_iPlayersConnected--;
  }
}

const wchar_t *CNetGame::getLevelName() {
	return m_sLevelName;
}

const wchar_t *CNetGame::getHiScoreFile() {
  return m_sHiScoreFile;
}

u32 CNetGame::getLaps() {
  return m_iLaps;
}

void CNetGame::addPlayerGUI(u32 iNum, const c8 *s) {
  c8 s2[0xFF];
  rect<s32> cRect=rect<s32>(10,140+25*iNum,250,160+25*iNum);
  sprintf(s2,"Player %i: %s",iNum+1,s);
  IGUIStaticText *pText=m_pGuienv->addStaticText(stringw(s2).c_str(),cRect,false,true,m_pTab,-1,true);
  pText->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

  if (m_aPlayerNames.size()>iNum) {
    m_aPlayerNames[iNum]->setVisible(false);;
    m_aPlayerNames[iNum]=pText;
  }
  else m_aPlayerNames.push_back(pText);
}

void CNetGame::tournamentNameChanged() {
  CEventServerMessage cMsg(eNetTournamentName);
  cMsg.setData2(stringc(stringw(m_pName->getText())).c_str());
  printf("\n\t\tserver using netbook level: %s\n\n",m_pOptions->netbook()?"yes":"no");
  cMsg.setData1(m_pOptions->netbook()?1:0);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CNetGame::addTrack(u32 iIdx) {
  sendAddTrack(-1,iIdx);
}

void CNetGame::delTrack(u32 iIdx) {
  CEventServerMessage cMsg(eNetDelRace);
  cMsg.setData1(iIdx);
  m_pNetAdapter->sendPacket(&cMsg);
}

void CNetGame::clearList() {
  sendClearList(-1);
}

void CNetGame::trackChanged(u32 iIdx, const c8 *sTrack) {
  sendTrackChanged(-1,iIdx,sTrack);
}

void CNetGame::lapsChanged(u32 iIdx, u16 iLaps) {
  sendLapsChanged(-1,iIdx,iLaps);
}

void CNetGame::sendClearList(s16 iPeer) {
  printf("CNetGame::sendClearList\n");
  CEventServerMessage cMsg(eNetClearList);
  if (iPeer==-1) m_pNetAdapter->sendPacket(&cMsg); else m_pNetAdapter->sendPacket(&cMsg,iPeer);
}

void CNetGame::sendTrackChanged(s16 iPeer, u32 iIdx, const c8 *sTrack) {
  printf("CNetGame::sendTrackChanged\n");
  CEventServerMessage cMsg(eNetSetRaceTrack);
  cMsg.setData1(iIdx);
  cMsg.setData2(sTrack);
  if (iPeer==-1) m_pNetAdapter->sendPacket(&cMsg); else m_pNetAdapter->sendPacket(&cMsg,iPeer);
}

void CNetGame::sendLapsChanged(s16 iPeer, u32 iIdx, u16 iLaps) {
  printf("CNetGame::sendLapsChanged\n");
  CEventServerMessage cMsg(eNetSetRaceLaps);
  cMsg.setData1(iIdx);
  cMsg.setData3(iLaps);
  if (iPeer==-1) m_pNetAdapter->sendPacket(&cMsg); else m_pNetAdapter->sendPacket(&cMsg,iPeer);
}

void CNetGame::sendAddTrack(s16 iPeer, u32 iIdx) {
  printf("CNetGame::sendAddTrack\n");
  CEventServerMessage cMsg(eNetAddRace);
  cMsg.setData1(iIdx);
  if (iPeer==-1)
    m_pNetAdapter->sendPacket(&cMsg);
  else
    m_pNetAdapter->sendPacket(&cMsg,iPeer);
}

bool CNetGame::startTournament() {
  m_pTournament->setPlayerNo(m_aPlayerNames.size());
  if (CTournamentBase::startTournament()) {
    m_pRace->setNetAdapter(m_pNetAdapter);
    CEventServerMessage cMsg(eNetStartTournament);
    m_iSelect=8;
    m_pNetAdapter->sendPacket(&cMsg);
    return true;
  }
  return false;
}

void CNetGame::fillTournamentStructure() {
  if (m_pTournament) delete m_pTournament;
  m_pTournament=new CTournamentInfo(m_pName->getText());
  list<CTournamentRaceGUI *>::Iterator it;

  for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
    CTournamentRaceGUI *p=*it;
    CTournamentRaceInfo *pRace=new CTournamentRaceInfo(p->getTrack(),p->getLaps());
    m_pTournament->addRace(pRace);
  }

  if (m_pNetAdapter!=NULL) {
    u32 iPlayerNo=m_pNetAdapter->getPeerCount();
    if (m_pNetAdapter->isServer()) iPlayerNo++;
    m_pTournament->setPlayerNo(iPlayerNo);
    if (m_eState==eServer) {
      m_pTournament->addPlayerName(stringw(m_pOptions->getPlayerName(0)).c_str());
      for (u32 i=0; i<m_pNetAdapter->getPeerCount(); i++) {
        CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerDataByIndex(i);
        if (plr) m_pTournament->addPlayerName(stringw(stringc(plr->getName())).c_str());
      }
    }
    else {
      for (u32 i=0; i<m_pNetAdapter->getPeerCount(); i++) {
        CNetPlayer *plr=(CNetPlayer *)m_pNetAdapter->getPeerData(i);
        if (plr) m_pTournament->addPlayerName(stringw(stringc(plr->getName())).c_str());
      }
    }
  }

  m_pTournament->setRecordReplay(m_bRecordReplay);
}

bool CNetGame::handlesEvent(IIrrOdeEvent *pEvent) {
  return true;
}
