  #include <CIrrOdeKlangManager.h>
  #include <CStateMachine.h>
  #include <CSerializer.h>
  #include <CGame.h>
  #include <COptions.h>
  #include <CTournamentRace.h>

  #include <CIrrOdeManager.h>

  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/NetMessages.h>

  #include <GameClasses/CNetPlayer.h>
  #include <GameClasses/CRaceInfo.h>
  #include <GameClasses/CLevelList.h>
  #include <GameClasses/CGameLogic.h>
  #include <GameClasses/CTournamentInfo.h>
  #include <GameClasses/CGameLogicClient.h>

class CResult {
  public:
    CResult() {
      iPoints=0;
      iVictories=0;
      iFastestLaps=0;
      iIdx=0;
    }

    u32 iPoints,iVictories,iFastestLaps,iIdx;
};

CRaceInfoGUI::CRaceInfoGUI(IrrlichtDevice *pDevice, u32 iRace, CTournamentRaceInfo *pRace, IGUIElement *pParent) {
  IGUIEnvironment *pGuienv=pDevice->getGUIEnvironment();

  wchar_t s[0xFFF];
  swprintf(s,L"Race %i: %s, %i Laps",iRace+1,pRace->getTrack(),pRace->getLaps());
  m_pInfo=pGuienv->addStaticText(s,rect<s32>(10,15+20*iRace,450,32+20*iRace),false,true,pParent,-1,true);
}

CRaceInfoGUI::~CRaceInfoGUI() {
  m_pInfo->remove();
}

void CRaceInfoGUI::setState(u32 iState) {
  switch (iState) {
    case 1: m_pInfo->setBackgroundColor(SColor(128,0,192,0)); break;
    case 2: m_pInfo->setBackgroundColor(SColor(128,64,64,192)); break;
  }
}

CTournamentRace::CTournamentRace(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_iSelect=0;
  m_pLogic=NULL;
  m_pNetAdapter=NULL;
  m_pOptions=NULL;
}

CTournamentRace::~CTournamentRace() {
  if (m_pLogic) delete m_pLogic;
}

void CTournamentRace::activate(IState *pPrevious) {
  printf("CTournamentRace::activate start\n");
  irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
  if (m_pRace!=NULL && pPrevious!=m_pRace) {
    m_iReturnState=pPrevious->getNum()+1;
    printf("m_iReturnState: %i\n",m_iReturnState);
  }
  m_pDevice->getCursorControl()->setVisible(true);
  m_pTimer=m_pDevice->getTimer();
  m_pSmgr->loadScene("data/setup_scene.xml");
  m_iSelect=0;

	m_pSndEngine=m_pStateMachine->getSoundEngine();

  dimension2du screenSize=m_pDriver->getScreenSize();
  s32 iLeftX=screenSize.Width/2-400,iRightX=screenSize.Width/2+400;

  m_pTab=m_pGuienv->addTab(rect<s32>(iLeftX,0,iRightX,600));

  m_pPages=m_pGuienv->addTabControl(rect<s32>(10,50,790,590),m_pTab,true,true);

  m_pRaces    =m_pPages->addTab(L"Tournament Races");
  m_pStandings=m_pPages->addTab(L"Tournament Standings");

  m_pTable=m_pGuienv->addTable(rect<s32>(10,10,770,530),m_pStandings,-1,true);
  m_pTable->addColumn(L"");

  for (u32 j=0; j<m_pTrnInfo->getPlayerNo(); j++) {
    m_pTable->addColumn(m_pTrnInfo->getPlayerName(j));
    m_pTable->setColumnWidth(j+1,400/m_pTrnInfo->getPlayerNo());
  }

  m_pTable->addColumn(L"Fastest Lap");

  m_pTable->setColumnWidth(0,160);
  m_pTable->setColumnWidth(m_pTable->getColumnCount()-1,200);

  printf("%i races\n",m_pTrnInfo->getRaceCount());
  u32 iPoints[4]={0,0,0,0},
      iVictories[4]={0,0,0,0},
      iFastestLaps[4]={0,0,0,0};

  for (u32 i=0; i<m_pTrnInfo->getRaceCount(); i++) {
    CTournamentRaceInfo *pInfo=m_pTrnInfo->getRace(i);
    CRaceInfoGUI *pGui=new CRaceInfoGUI(m_pDevice,i,pInfo,m_pRaces);
    if (i<=m_iNextRace) pGui->setState(m_iNextRace==i?1:2);
    m_aRaces.push_back(pGui);
    printf("Race %i: %s, %i Laps\n",i+1,stringc(stringw(pInfo->getTrack())).c_str(),pInfo->getLaps());
    u32 iRow=m_pTable->addRow(m_pTable->getRowCount());
    stringw sTrackName=stringw(pInfo->getTrack());
    if (sTrackName.findFirst(L':')!=-1) sTrackName=sTrackName.subString(sTrackName.findFirst(L':')+1,sTrackName.size());
    sTrackName=sTrackName.trim();
    m_pTable->setCellText(iRow,0,sTrackName);
    if (m_iNextRace>0) {
      printf("race num: %i\n",m_iNextRace-1);
      CRaceInfo *pInfo=m_pTrnInfo->getRaceInfo(i);
      if (pInfo && pInfo->raceFinished()) {
        for (u32 j=0; j<m_pTrnInfo->getPlayerNo(); j++) {
          wchar_t s[0xFF];
          u32 iPos=pInfo->getPlayerFinishedPosition(j);
          swprintf(s,L"%s",iPos==0?L"1st":iPos==1?L"2nd":iPos==2?L"3rd":iPos==3?L"4th":L"D.N.F.");
          m_pTable->setCellText(iRow,j+1,s);
          m_pTable->setCellColor(iRow,j+1,SColor(255,64,64,64));
          if (iPos==0) {
            iPoints[j]+=m_pTrnInfo->getPlayerNo();
            iVictories[j]++;
          }
          else {
            iPoints[j]+=m_pTrnInfo->getPlayerNo()-1-iPos;
          }
          if (pInfo->getFastestLap()->getPlayerNo()==j) iPoints[j]++;
        }

        m_pTable->setCellText(iRow,m_pTable->getColumnCount()-1,m_pTrnInfo->getPlayerName(pInfo->getFastestLap()->getPlayerNo()));
        iFastestLaps[pInfo->getFastestLap()->getPlayerNo()]++;
      }
      else
        if (i<m_iNextRace)
          for (u32 j=0; j<m_pTrnInfo->getPlayerNo(); j++) {
            m_pTable->setCellText(iRow,j+1,L"Cancelled");
            m_pTable->setCellText(iRow,m_pTable->getColumnCount()-1,L"Cancelled");
          }
    }
  }

  u32 iRow=m_pTable->addRow(m_pTable->getRowCount());
  m_pTable->setCellText(iRow,0,L"Points");
  for (u32 i=0; i<m_pTrnInfo->getPlayerNo(); i++) {
    wchar_t s[0xFF];
    swprintf(s,L"%i",iPoints[i]);
    m_pTable->setCellText(iRow,i+1,s);
  }

  IGUIFont *pBigFont=m_pGuienv->getFont("data/font2.xml");
  IGUIStaticText *pHeadline=m_pGuienv->addStaticText(L"Kinect Marble Racers - Tournament",rect<s32>(10,10,790,40),false,true,m_pTab);
  pHeadline->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  pHeadline->setOverrideFont(pBigFont);
  pHeadline->setBackgroundColor(SColor(128,16,16,16));
  pHeadline->setOverrideColor(SColor(255,255,118,70));

  if (m_iNextRace<m_aRaces.size() || m_iState==1) {
    if (m_pNetAdapter==NULL || m_pNetAdapter->isServer())
      m_pGuienv->addButton(rect<s32>(10,445,210,465),m_pRaces,1,L"Start next Race");

    m_pGuienv->addButton(rect<s32>(10,470,210,490),m_pRaces,2,L"Cancel Tournament");
    m_iState=0;
  }
  else {
    array<CResult *> aResult;
    bool b=false;

    for (u32 i=0; i<m_pTrnInfo->getPlayerNo(); i++) {
      CResult *p=new CResult();
      p->iPoints=iPoints[i];
      p->iVictories=iVictories[i];
      p->iFastestLaps=iFastestLaps[i];
      p->iIdx=i;
      b=false;

      for (u32 j=0; j<aResult.size() && !b; j++)
        if (p->iPoints>aResult[j]->iPoints) {
          aResult.insert(p,j);
          b=true;
        }

      if (!b) aResult.push_back(p);
    }

    m_pGuienv->addButton(rect<s32>(10,470,210,490),m_pRaces,3,L"Back to Main Menu");
    m_pTab->setVisible(false);
    u32 x=screenSize.Width/2-350,y=50;
    m_pGuienv->addImage(m_pDriver->getTexture("data/overall.png"),position2di(x,y),true);
    y+=150;
    x+=50;
    printf("\n\t\taResult.size: %i\n\n",aResult.size());
    for (u32 i=0; i<aResult.size(); i++) {
      wchar_t s[0xFF];
      IGUITab *pTab=m_pGuienv->addTab(rect<s32>(x,y,x+600,y+75));
      pTab->setDrawBackground(true);
      pTab->setBackgroundColor(SColor(128,16,16,16));
      swprintf(s,L"%i: %s",i+1,m_pTrnInfo->getPlayerName(aResult[i]->iIdx));
      IGUIStaticText *text=m_pGuienv->addStaticText(s,rect<s32>(10,10,590,45),false,true,pTab);
      text->setOverrideFont(pBigFont);
      text->setOverrideColor(SColor(255,255,82,33));
      swprintf(s,L"%i points, %i victories, %i fastest laps",aResult[i]->iPoints,aResult[i]->iVictories,aResult[i]->iFastestLaps);
      text=m_pGuienv->addStaticText(s,rect<s32>(50,50,590,65),false,true,pTab);
      text->setOverrideColor(SColor(255,255,82,33));
      y+=80;
    }
    m_iState=1;
  }

  if (m_pLogic) delete m_pLogic;
  if (m_pNetAdapter==NULL || m_pNetAdapter->isServer())
    m_pLogic=new CGameLogic();
  else
    m_pLogic=new CGameLogicClient();

  m_pLogic->setDevice(m_pDevice);
  m_pLogic->setGame(m_pRace);

  m_pDevice->setEventReceiver(this);
  if (m_pNetAdapter) {
    m_pNetAdapter->addConnectionHandler(this);
    m_pLogic->setNetAdapter(m_pNetAdapter);

    if (m_pNetAdapter->isServer()) {
      CEventServerMessage cMsg(eMessageQuitState);
      m_pNetAdapter->sendPacket(&cMsg);
      m_pNetAdapter->update();
    }
  }
  m_pLogic->resetPlayerObject();
  printf("CTournamentRace::activate end\n");
}

void CTournamentRace::deactivate(IState *pNext) {
  for (u32 i=0; i<m_aRaces.size(); i++) delete m_aRaces[i];

  IImage *pLoading=m_pDriver->createImageFromFile("data/loading.png");

  if (pLoading) {
    ITexture *pLoadingTexture=m_pDriver->addTexture("loading",pLoading);
    if (pLoadingTexture) {
      dimension2du screenSize=m_pDriver->getScreenSize();
      position2di pos=position2di(screenSize.Width/2-115,screenSize.Height/2-32);
      m_pGuienv->addImage(pLoadingTexture,pos);
      m_pDriver->beginScene();
      m_pSmgr->drawAll();
      m_pGuienv->drawAll();
      m_pDriver->endScene();
    }
    pLoading->drop();
  }

  if (m_pNetAdapter) m_pNetAdapter->removeConnectionHandler(this);
  irr::ode::CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
  m_aRaces.clear();
  m_pGuienv->clear();
  m_pSmgr->clear();
}

u32 CTournamentRace::update() {
  if (m_pNetAdapter) m_pNetAdapter->update();
  return m_iSelect;
}

bool CTournamentRace::startRace() {
  printf("start next race\n");
  CIrrOdeKlangManager::getSharedInstance()->removeAllNodes();
  if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");

  CLevelList *pList=new CLevelList(m_pDevice);
  CRaceInfo *pInfo=NULL;

  CTournamentRaceInfo *pRace=m_pTrnInfo->getRace(m_iNextRace);
  stringw sRaceTrack=stringw(pRace->getTrack());

  for (u32 i=0; i<pList->m_aLevels.size(); i++) {
    if (sRaceTrack==pList->m_aLevels[i]->m_sName) {
      IFileSystem *pFs=m_pDevice->getFileSystem();
      bool bNetBookLevel=m_pOptions!=NULL && m_pOptions->netbook() && pFs->existFile(path(pList->m_aLevels[i]->m_sNetbookFile));

      pInfo=new CRaceInfo();
      pInfo->setLevel(bNetBookLevel?pList->m_aLevels[i]->m_sNetbookFile:pList->m_aLevels[i]->m_sFile);
      pInfo->setSplitHorizontal(m_pTrnInfo->getSplitHorizontal());
      pInfo->setLaps(pRace->getLaps());
      pInfo->setRecordReplay(m_pTrnInfo->getRecordReplay());
      for (u32 j=0; j<m_pTrnInfo->getPlayerNo(); j++) pInfo->addPlayerName(m_pTrnInfo->getPlayerName(j));
      pInfo->setHiScoreFile(pList->m_aLevels[i]->m_sHiScoreFile.c_str());
      m_pTrnInfo->addRaceInfo(pInfo);
      break;
    }
  }

  delete pList;

  if (pInfo) {
    m_iNextRace++;
    m_iSelect=6;
    m_pLogic->setRaceInfo(pInfo);
    return true;
  }
  return false;
}

bool CTournamentRace::OnEvent (const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown) {
	  if (event.KeyInput.Key==KEY_ESCAPE && !event.KeyInput.PressedDown) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
      m_iSelect=m_iReturnState;

      if (m_pNetAdapter) {
        CEventServerMessage cMsg(eNetBackToStart);
        m_pNetAdapter->sendPacket(&cMsg);
      }

	    bRet=true;
	  }

	  if (event.KeyInput.Key==KEY_RETURN && (!m_pNetAdapter || m_pNetAdapter->isServer())) bRet=startRace();
  }

  if (event.EventType==EET_GUI_EVENT) {
    if (event.GUIEvent.EventType==EGET_TAB_CHANGED && m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
      u32 id=event.GUIEvent.Caller->getID();

      if (id==1 && (!m_pNetAdapter || m_pNetAdapter->isServer())) bRet=startRace();

      if (id==2) {
        m_pSndEngine->play2D("data/sounds/menuback.ogg");
        m_iSelect=m_iReturnState;

        if (m_pNetAdapter) {
          CEventServerMessage cMsg(eNetBackToStart);
          m_pNetAdapter->sendPacket(&cMsg);
        }

        bRet=true;
      }

      if (id==3) {
        m_pSndEngine->play2D("data/sounds/menuchange.ogg");
        m_iSelect=1;
        bRet=true;
      }
    }
  }

  return bRet;
}

void CTournamentRace::setTournamentInfo(CTournamentInfo *pInfo) {
  m_pTrnInfo=pInfo;
  m_iNextRace=0;
}

void CTournamentRace::setRace(CGame *pRace) {
  m_pRace=pRace;
}

bool CTournamentRace::onEvent(IIrrOdeEvent *pEvent) {
  bool bRet=false;
  printf("CTournamentRace::onEvent\n");

  if (!m_pNetAdapter->isServer() && pEvent->getType()==eNetServerMessage) {
    CEventServerMessage *p=(CEventServerMessage *)pEvent;
    switch (p->getCode()) {
      case eNetStartRace: startRace(); break;
      case eNetBackToStart: m_iSelect=m_iReturnState; break;
    }
  }

  return bRet;
}

void CTournamentRace::onPeerDisconnect(u16 iPlayerId) {
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

void CTournamentRace::onPeerConnect(u16 iPlayerId) {
  CEventServerMessage cMsg(eNetGameRunning);
  m_pNetAdapter->sendPacket(&cMsg,iPlayerId);
  m_pNetAdapter->update();
}

bool CTournamentRace::handlesEvent(IIrrOdeEvent *pEvent) {
  return true;
}
