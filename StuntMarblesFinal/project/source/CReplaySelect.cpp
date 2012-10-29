  #include <CReplaySelect.h>
  #include <CStateMachine.h>
  #include <GameClasses/CLevelList.h>
  #include <CMessage.h>
  #include <CGame.h>

  #include <IrrOdeNet/CEventPlayer.h>
  #include <IrrOdeNet/CEventIdentify.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

CReplaySelect::CReplaySelect(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_iRet=0;
  m_pPlayer=new CIrrOdeRePlayer(m_pDevice);
  m_pLevelList=new CLevelList(m_pDevice);
}

CReplaySelect::~CReplaySelect() {
  delete m_pPlayer;
  delete m_pLevelList;
}

void CReplaySelect::activate(IState *pPrevious) {
  m_iRet=0;
  m_pDevice->getCursorControl()->setVisible(true);
  m_pDevice->setEventReceiver(this);
  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pSmgr->loadScene("data/setup_scene.xml");

  ISceneNode *pSkybox=m_pSmgr->getSceneNodeFromType(ESNT_SKY_BOX);
  if (pSkybox)
    for (u32 i=0; i<pSkybox->getMaterialCount(); i++)
      for (u32 j=0; j<MATERIAL_MAX_TEXTURES; j++) {
        pSkybox->getMaterial(i).TextureLayer[j].LODBias=-128;
        if (pSkybox->getMaterial(i).TextureLayer[j].Texture!=NULL && j!=0) {
          pSkybox->getMaterial(i).TextureLayer[j].Texture=pSkybox->getMaterial(i).TextureLayer[0].Texture;
        }
      }

  dimension2du screenSize=m_pDriver->getScreenSize();
  s32 iLeftX=screenSize.Width/2-400,iRightX=screenSize.Width/2+400;
  m_pTab=m_pGuienv->addTab(rect<s32>(iLeftX,0,iRightX,600));

  IGUIFont *pBigFont=m_pGuienv->getFont("data/font2.xml");
  IGUIStaticText *pHeadline=m_pGuienv->addStaticText(L"Kinect Marble Racers - Replay",rect<s32>(10,10,790,40),false,true,m_pTab);
  pHeadline->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  pHeadline->setOverrideFont(pBigFont);
  pHeadline->setBackgroundColor(SColor(128,16,16,16));
  pHeadline->setOverrideColor(SColor(255,255,118,70));

  m_pStart =m_pGuienv->addButton(rect<s32>(390,310,590,330),m_pTab,1,L"Start Replay" ,NULL);
  m_pCancel=m_pGuienv->addButton(rect<s32>(390,335,590,355),m_pTab,2,L"Cancel"       ,NULL);
  m_pReplayList=m_pGuienv->addListBox(rect<s32>(10,80,380,540),m_pTab,3,true);;

  IGUIStaticText *pText=m_pGuienv->addStaticText(L"Select Replay",rect<s32>(10,55,380,75),false,true,m_pTab,-1,false);
  pText->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

  IGUITab *pDetail=m_pGuienv->addTab(rect<s32>(390,80,790,300),m_pTab);
  pDetail->setBackgroundColor(SColor(128,192,192,192));
  pDetail->setDrawBackground(true);

  pText=m_pGuienv->addStaticText(L"Level:",rect<s32>(10,10,90,30),false,true,pDetail,-1,false);
  pText->setTextAlignment(EGUIA_LOWERRIGHT,EGUIA_CENTER);

  pText=m_pGuienv->addStaticText(L"time:",rect<s32>(10,35,90,55),false,true,pDetail,-1,false);
  pText->setTextAlignment(EGUIA_LOWERRIGHT,EGUIA_CENTER);

  m_pLevel=m_pGuienv->addStaticText(L"",rect<s32>(100,10,390,30),false,true,pDetail,-1,true);
  m_pTime =m_pGuienv->addStaticText(L"",rect<s32>(100,35,390,55),false,true,pDetail,-1,true);

  m_pLevel->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);
  m_pTime ->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

  for (u32 i=0; i<4; i++) {
    m_pPlayers[i]=m_pGuienv->addStaticText(L"",rect<s32>(200,70+25*i,390,90+25*i),false,true,pDetail,-1,true);
    m_pPlayers[i]->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);
    m_pPlayers[i]->setVisible(false);

    wchar_t s[0xFF];
    swprintf(s,0xFF,L"Player %i:",i+1);
    m_pPlr[i]=m_pGuienv->addStaticText(s,rect<s32>(10,70+25*i,190,90+25*i),false,true,pDetail,-1,false);
    m_pPlr[i]->setTextAlignment(EGUIA_LOWERRIGHT,EGUIA_CENTER);
    m_pPlr[i]->setVisible(false);
  }

  IFileSystem *pFs=m_pDevice->getFileSystem();
  path cwd=pFs->getWorkingDirectory();
  pFs->changeWorkingDirectoryTo("data/replay");
  IFileList *pReplays=pFs->createFileList();

  for (u32 i=0; i<pReplays->getFileCount(); i++) {
    const c8 *sFile=pReplays->getFileName(i).c_str();
    if (sFile[0]!='.') {
//      if (strchr(sFile,'.')) *strchr(sFile,'.')='\0'; HACK
      stringw wsFile=stringw(sFile);
      m_pReplayList->addItem(wsFile.c_str());
      if (wsFile==m_sReplay) m_pReplayList->setSelected(m_pReplayList->getItemCount()-1);
    }
  }

  pReplays->drop();
  pFs->changeWorkingDirectoryTo(cwd);
  updateReplayInfo();
}

void CReplaySelect::deactivate(IState *pNext) {
  m_pSmgr->clear();
  m_pGuienv->clear();
  if (m_pPlayer==NULL) delete m_pPlayer;
}

u32 CReplaySelect::update() {
  return m_iRet;
}

bool CReplaySelect::OnEvent (const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_KEY_INPUT_EVENT) {
    if (!event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_ESCAPE:
          m_iRet=1;
          bRet=true;
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
          break;

        case KEY_RETURN:
          m_iRet=10;
          bRet=true;
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
          break;

        default:
          break;
      }
    }
  }

  if (event.EventType==EET_GUI_EVENT) {
    if (event.GUIEvent.EventType==EGET_LISTBOX_CHANGED) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
      m_sReplay=stringw(m_pReplayList->getListItem(m_pReplayList->getSelected()));
      updateReplayInfo();
      bRet=true;
    }

    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
      switch (event.GUIEvent.Caller->getID()) {
        case 1:
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
          m_iRet=10;
          bRet=true;
          break;

        case 2:
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
          m_iRet=1;
          bRet=true;
          break;
      }
    }
  }

  return bRet;
}

void CReplaySelect::writeConfig(IXMLWriter *pXml) {
  if (pXml) {
    pXml->writeElement(L"replay",false);
    pXml->writeLineBreak();

    pXml->writeElement(L"selected",true,L"name",m_sReplay.c_str());
    pXml->writeLineBreak();

    pXml->writeClosingTag(L"replay");
    pXml->writeLineBreak();
  }
}

void CReplaySelect::readConfig(IXMLReaderUTF8 *pXml) {
  if (pXml && !strcmp(pXml->getNodeName(),"replay") && pXml->getNodeType()==EXN_ELEMENT)
    while (pXml->read() && strcmp(pXml->getNodeName(),"replay")) {
      if (!strcmp(pXml->getNodeName(),"selected"))
        m_sReplay=stringw(pXml->getAttributeValue("name"));
    }
}

void CReplaySelect::handleMessage(IIrrOdeEvent *pMessage) {
  switch (pMessage->getType()) {
    case irr::ode::eIrrOdeEventStep:
      m_iSteps++;
      break;

    case eMessageIdentify:
      break;

    case irr::ode::eIrrOdeEventLoadScene: {
        irr::ode::CIrrOdeEventLoadScene *p=(irr::ode::CIrrOdeEventLoadScene *)pMessage;
        m_sLevelFile=stringw(p->getScene());
      }
      break;

    case eMessagePlayer: {
        CEventPlayer *p=(CEventPlayer *)pMessage;
        m_aPlayers.push_back(stringc(p->getName()));
      }
      break;
  }
}

void CReplaySelect::updateReplayInfo() {
  m_aPlayers.clear();
  m_iSteps=0;

  if (m_sReplay.size()==0) return;

  char s[0xFF];
  sprintf(s,"data/replay/%s.replay",stringc(m_sReplay).c_str());
  printf("replay file: \"%s\"\n",s);

  m_pPlayer->loadReplay(s);
  if (m_pPlayer->isValidReplay()) {
    m_pPlayer->scanReplay(this);
    printf("%i steps - %.2f sec\n",m_iSteps,((f32)m_iSteps*16)/1000);
  }

  for (u32 i=0; i<4; i++) {
    m_pPlayers[i]->setVisible(false);
    m_pPlr[i]->setVisible(false);
  }

  for (u32 i=0; i<m_aPlayers.size(); i++) {
    m_pPlayers[i]->setText(stringw(m_aPlayers[i]).c_str());
    m_pPlayers[i]->setVisible(true);
    m_pPlr[i]->setVisible(true);
  }

  wchar_t ws[0xFF];
  swprintf(ws,0xFF,L"%.2f sec",((f32)m_iSteps*16)/1000);
  m_pTime->setText(ws);

  char sLevel[0xFF];
  strcpy(sLevel,stringc(m_sLevelFile).c_str());

  for (u32 i=0; i<m_pLevelList->m_aLevels.size(); i++) {
    char sLevelInfo[0xFF],*cp=sLevelInfo;
    strcpy(sLevelInfo,stringc(m_pLevelList->m_aLevels[i]->m_sFile).c_str());
    if (strstr(sLevelInfo,"data/levels")) cp=strstr(sLevelInfo,"data/levels");
    if (!strcmp(cp,sLevel)) m_pLevel->setText(m_pLevelList->m_aLevels[i]->m_sName.c_str());
  }
}

const wchar_t *CReplaySelect::getReplayName() {
  static wchar_t s_sFile[0xFF];
  swprintf(s_sFile,0xFF,L"data/replay/%s.replay",m_sReplay.c_str());
  return s_sFile;
}
