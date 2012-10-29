  #include <COptions.h>
	#include <CGame.h>
  #include <CSettings.h>
  #include <CStateMachine.h>
  #include <CRenderToTextureManager.h>

COptions::COptions(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
	m_pPlayerNames[0]=NULL;
	m_pPlayerNames[1]=NULL;

	m_bGhostRec=false;
  m_bVideoScreen=false;
  m_bNetBook=false;
  m_iVideoFPS=0;
  m_iVideoSize=0;
  m_bNetgame=false;
}

void COptions::addPlayerTab(IGUIElement *pParent, u32 iNum) {
  wchar_t s[0xFF];

  swprintf(s,0xFF,L"Player %i Name:",iNum+1);
  m_pGuienv->addStaticText(s,rect<s32>(15,15,150,30),false,false,pParent);

  m_pPlayerNames[iNum]=m_pGuienv->addEditBox(m_sPlayerNames[iNum].c_str(),rect<s32>(160,13,300,36),true,pParent);
  m_pGame->addControlGui(iNum,pParent);
}

void COptions::activate(IState *pPrevious) {
  dimension2du size=m_pDriver->getCurrentRenderTargetSize();

  m_pTabCtrl=m_pGuienv->addTabControl(rect<s32>(5,5,size.Width-5,size.Height-5),NULL,true,true,-1);
  IGUITab *pTab[4];

  pTab[0]=m_pTabCtrl->addTab(L"Player 1");
  pTab[1]=m_pTabCtrl->addTab(L"Player 2");
  pTab[2]=m_pTabCtrl->addTab(L"Video Options");
  pTab[3]=m_pTabCtrl->addTab(L"Game Options");

  addPlayerTab(pTab[0],0);
  addPlayerTab(pTab[1],1);

  m_pSettings=new CSettings(NULL,"data/graphics.xml",NULL,SColor(0x00,0x21,0xAD,0x10),m_pDevice);
  m_pSettings->createGUI(m_pGuienv,pTab[2],position2di(15,15),false);

  IGUIStaticText *t=m_pGuienv->addStaticText(L"Record Ghost",rect<s32>(30,30,230,50),false,true,pTab[3],-1,true);
  t->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

  m_pGhostRec=m_pGuienv->addCheckBox(m_bGhostRec,rect<s32>(232,28,256,52),pTab[3],101);

  t=m_pGuienv->addStaticText(L"Enable Video Screen",rect<s32>(30,70,230,90),false,true,pTab[3],-1,true);
  t->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);
  m_pVideoScreen=m_pGuienv->addCheckBox(m_bVideoScreen,rect<s32>(232,68,256,92),pTab[3],202);

  t=m_pGuienv->addStaticText(L"Play on Netbook",rect<s32>(30,190,230,210),false,true,pTab[3],-1,true);
  t->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);
  m_pNetBook=m_pGuienv->addCheckBox(m_bNetBook,rect<s32>(232,188,256,212),pTab[3],203);

  t=m_pGuienv->addStaticText(L"Videoscreen Texture Size",rect<s32>(30,110,230,130),false,true,pTab[3],-1,true);
  t->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

  m_pVideoSize=m_pGuienv->addComboBox(rect<s32>(232,108,350,132),pTab[3],303);
  m_pVideoSize->addItem(L"512x512");
  m_pVideoSize->addItem(L"256x256");
  m_pVideoSize->addItem(L"128x128");
  m_pVideoSize->addItem(L"64x64");
  m_pVideoSize->setSelected(m_iVideoSize);

  t=m_pGuienv->addStaticText(L"Video Screen FPS",rect<s32>(30,150,230,170),false,true,pTab[3],-1,true);
  t->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);

  m_pVideoFPS=m_pGuienv->addComboBox(rect<s32>(232,148,350,172),pTab[3],404);
  m_pVideoFPS->addItem(L"60 fps");
  m_pVideoFPS->addItem(L"30 fps");
  m_pVideoFPS->addItem(L"15 fps");
  m_pVideoFPS->addItem(L"5 fps");
  m_pVideoFPS->setSelected(m_iVideoFPS);

  m_pDevice->setEventReceiver(this);
  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();
  m_bQuit=false;
  m_pDevice->getCursorControl()->setVisible(true);

  m_pDevice->getSceneManager()->loadScene("data/setup_scene.xml");

  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pSmgr->getActiveCamera()->setAspectRatio(m_pStateMachine->getAspectRatio());
}

void COptions::deactivate(IState *pNext) {
  m_pDevice->getSceneManager()->clear();
  m_pGuienv->clear();
	m_pPlayerNames[0]=NULL;
	m_pPlayerNames[1]=NULL;
  delete m_pSettings;
}

u32 COptions::update() {
  if (!m_pDevice->getCursorControl()->isVisible()) m_pDevice->getCursorControl()->setVisible(true);
  return m_bQuit?1:0;
}

bool COptions::OnEvent(const SEvent &event) {
  bool bRet=false;

  for (u32 i=0; i<2; i++) m_sPlayerNames[i]=m_pPlayerNames[i]->getText();
	if (m_pTabCtrl->getActiveTab()<2) {
	  if (m_pGame->getCC()->ConfigEvent(event)) return true;
	}
	else bRet=m_pSettings->OnEvent(event);

  if (m_pSettings->OnEvent(event)) {
    switch (m_pSettings->getClose()) {
      case 1:
        if (m_pTabCtrl->getActiveTab()==2) {
          m_pStateMachine->graphicsChanged();
          m_pSettings->save();
          CConfigFileManager::getSharedInstance()->writeConfig(m_pDevice,"data/settings.xml");
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
        }
        break;

      case 2:
        m_pSettings->load();
        m_pSettings->updateGui();
        if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
        break;
    }
    return true;
  }

  if (event.EventType==EET_GUI_EVENT) {
    if (event.GUIEvent.EventType==EGET_CHECKBOX_CHANGED) {
      IGUICheckBox *p=(IGUICheckBox *)event.GUIEvent.Caller;
      switch (p->getID()) {
        case 101: m_bGhostRec=p->isChecked(); break;
        case 202: m_bVideoScreen=p->isChecked(); break;
        case 203: m_bNetBook=p->isChecked(); break;
      }
    }

    if (event.GUIEvent.EventType==EGET_COMBO_BOX_CHANGED) {
      IGUIComboBox *p=(IGUIComboBox *)event.GUIEvent.Caller;
      switch (p->getID()) {
        case 303: m_iVideoSize=p->getSelected(); break;
        case 404: m_iVideoFPS=p->getSelected(); break;
      }
    }
  }

  if (event.EventType==EET_KEY_INPUT_EVENT) {
    if (event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_ESCAPE:
          m_bQuit=true;
          bRet=true;
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
          break;

        default:
          break;
      }
    }
  }

  return bRet;
}

void COptions::setGame(CGame *pGame) {
  m_pGame=pGame;
}

u32 COptions::getShadowLevel() {
	return m_pSettings->getShadowLevel();
}

void COptions::writeConfig(IXMLWriter *pXml) {
  if (pXml) {
    pXml->writeElement(L"options",false);
    pXml->writeLineBreak();

    for (u32 i=0; i<2; i++) {
      printf("write config: \"%s\"\n",stringc(m_sPlayerNames[i]).c_str());
      pXml->writeElement(L"player",true,L"name",m_sPlayerNames[i].c_str());
      pXml->writeLineBreak();
    }

    pXml->writeElement(L"rec_ghost",true,L"active",m_bGhostRec?L"1":L"0"); pXml->writeLineBreak();
    pXml->writeElement(L"video_screen",true,L"active",m_bVideoScreen?L"1":L"0"); pXml->writeLineBreak();

    wchar_t s[0xFF];
    swprintf(s,0xFF,L"%i",m_iVideoFPS);
    pXml->writeElement(L"video_fps",true,L"select",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_iVideoSize);
    pXml->writeElement(L"videosize",true,L"select",s);
    pXml->writeLineBreak();

    pXml->writeElement(L"netbook",true,L"value",m_bNetBook?L"1":L"0"); pXml->writeLineBreak();

    pXml->writeClosingTag(L"options");
    pXml->writeLineBreak();
  }

  printf("setting videoscreen options...\n");
  CRenderToTextureManager *pRtt=CRenderToTextureManager::getSharedInstance();
  pRtt->setGlobalSwitch(m_bVideoScreen);

  u32 i=0;
  switch (m_iVideoSize) {
    case 0: i=512; break;
    case 1: i=255; break;
    case 2: i=128; break;
    case 3: i=64 ; break;
  }

  pRtt->setTextureSize(i);

  switch (m_iVideoFPS) {
    case 0: i=0; break;
    case 1: i=2; break;
    case 2: i=4; break;
    case 3: i=12; break;
  }

  pRtt->setStepsToRender(i);
}

void COptions::readConfig(IXMLReaderUTF8 *pXml) {
  u32 iNum=0;
  if (pXml && !strcmp(pXml->getNodeName(),"options") && pXml->getNodeType()==EXN_ELEMENT)
    while (pXml->read() && strcmp(pXml->getNodeName(),"options")) {
      if (!strcmp(pXml->getNodeName(),"player")) {
        m_sPlayerNames[iNum]=stringw(pXml->getAttributeValue("name"));
        iNum++;
      }

      if (!strcmp(pXml->getNodeName(),"rec_ghost")) m_bGhostRec=atoi(pXml->getAttributeValue("active"));
      if (!strcmp(pXml->getNodeName(),"video_screen")) m_bVideoScreen=atoi(pXml->getAttributeValue("active"));
      if (!strcmp(pXml->getNodeName(),"video_fps")) m_iVideoFPS=atoi(pXml->getAttributeValue("select"));
      if (!strcmp(pXml->getNodeName(),"videosize")) m_iVideoSize=atoi(pXml->getAttributeValue("select"));
      if (!strcmp(pXml->getNodeName(),"netbook")) m_bNetBook=atoi(pXml->getAttributeValue("value"));
    }
}

const wchar_t *COptions::getPlayerName(u32 i) {
  if (i==0) return m_sPlayerNames[0].c_str();
  if (i==1) return m_sPlayerNames[1].c_str();
  return NULL;
}
