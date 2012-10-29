  #include <CLevel.h>
  #include <CFollowPathAnimator.h>
  #include <IrrOde.h>
	#include <CStateMachine.h>
	#include <CIrrOdeKlangManager.h>
	#include <COptions.h>
	#include <CShadowManager.h>

	#include <GameClasses/CLevelSelectGUI.h>
	#include <GameClasses/CHiScoreList.h>
	#include <GameClasses/CLevelList.h>
	#include <GameClasses/CGameLogic.h>
	#include <GameClasses/CRaceInfo.h>
	#include <GameClasses/CGhostPlayer.h>

#include <KinectFramework.h>


CLevel::CLevel(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
	m_bHorizontalSplit=false;
	m_bRecordReplay=false;
  m_iSelectedLevel=0;
  m_iPlayerNo=1;
  m_pOptions=NULL;
  m_pInfo=NULL;
  m_pLogic=NULL;
  m_pGhostSelect=NULL;
}

CLevel::~CLevel() {
  if (m_pInfo ) delete m_pInfo ;
  if (m_pLogic) delete m_pLogic;
}

void CLevel::activate(IState *pPrevious) {
	m_bOdeInitialized=false;
  m_pDevice->getCursorControl()->setVisible(true);
  m_pSmgr->loadScene("data/setup_scene.xml");
  m_iSelect=0;

	m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pLevelSelect=new CLevelSelectGUI(m_pDevice,m_pSndEngine,m_pStateMachine->getPreviewSceneManager(),m_pOptions->netbook());
  m_pLevelSelect->setSelectedLevel(m_iSelectedLevel);

  IGUITab *pTab=m_pLevelSelect->getTab();

  for (u32 i=0; i<2; i++) {
    m_pSelectNOP[i]=m_pGuienv->addCheckBox(i==0?true:false,rect<s32>(10,197+22*i,30,222+22*i),pTab,i+5,L"");
		m_pSelectNOP[i]->setEnabled(false);
		m_pSelectNOP[i]->setChecked(false);
		m_pGuienv->addStaticText(i==0?L"1 player":L"2 players",rect<s32>(29,200+22*i,134,218+22*i),false,true,pTab,-1,true);
  }

  m_pSelectNOP[m_iPlayerNo-1]->setChecked(true);

	m_pLevelInfo=m_pGuienv->addStaticText(L"",rect<s32>(140,200,440,400),true,true,pTab,-1,true);
	m_pHiScore=m_pGuienv->addStaticText(L"",rect<s32>(450,200,790,400),true,true,pTab,-1,true);

	m_pLevelInfo->setBackgroundColor(SColor(192,192,192,192));
	m_pHiScore->setBackgroundColor(SColor(192,192,192,192));

	m_pSplitTab=m_pGuienv->addTab(rect<s32>(10,247,134,284),pTab);

	m_pSplitHorizontal=m_pGuienv->addCheckBox(m_bHorizontalSplit,rect<s32>(0,0,20,30),m_pSplitTab,4711,L"");
	m_pSplitHorizontal->setEnabled(m_iPlayerNo==2);
	m_pSplitHorizontal->setChecked(m_bHorizontalSplit);
	m_pGuienv->addStaticText(L"Horizontal Splitscreen",rect<s32>(19,4,124,37),false,true,m_pSplitTab,-1,true);
	m_pSplitTab->setVisible(m_iPlayerNo==2);

	m_pGhostTab=m_pGuienv->addTab(rect<s32>(10,247,134,284),pTab);
	m_pPlayGhost=m_pGuienv->addCheckBox(m_bPlayGhost,rect<s32>(0,0,20,30),m_pGhostTab,4723,L"");
	m_pGuienv->addStaticText(L"Play Ghost Player",rect<s32>(19,4,124,37),false,true,m_pGhostTab,-1,true);
	m_pGhostTab->setVisible(m_iPlayerNo==1);

	m_pGuienv->addStaticText(L"Laps:",rect<s32>(10,295,70,313),false,true,pTab,-1,true);
	m_pLaps=m_pGuienv->addComboBox(rect<s32>(70,295,135,315),pTab,2305);

	m_pRecordReplay=m_pGuienv->addCheckBox(m_bRecordReplay,rect<s32>(10,320,30,340),pTab,815,L"");
	m_pGuienv->addStaticText(L"Record Replay",rect<s32>(29,321,134,339),false,true,pTab,-1,true);
  m_pStart=m_pGuienv->addButton(rect<s32>(10,346,135,371),pTab,23,L"Start Game");
	m_pStart->setEnabled(false);
  m_pGuienv->addButton(rect<s32>(10,376,135,401),pTab,24,L"Cancel");

  
  
  
  m_pLevels=new CLevelList(m_pDevice);

  m_pSmgr->getActiveCamera()->setAspectRatio(m_pStateMachine->getAspectRatio());
  m_pStateMachine->setDrawScene(false);
  m_pDevice->setEventReceiver(this);

  updateLevel();

  if (m_pLaps->getSelected()==-1) m_pLaps->setSelected(0);
  if (m_pInfo) {
    delete m_pInfo;
    m_pInfo=NULL;
  }

  if (m_pLogic) {
    delete m_pLogic;
    m_pLogic=NULL;
  }

  dimension2du cDim=m_pDriver->getScreenSize();
  position2di cPos=position2di(cDim.Width/2-160,cDim.Height/2-120);
  m_pGhostSelect=m_pGuienv->addWindow(rect<s32>(cPos,dimension2di(320,240)),false,L"Select Ghost");
  m_pGhostSelect->setDrawBackground(true);
  IGUIButton *p=m_pGhostSelect->getCloseButton();
  if (p) {
    p->setEnabled(false);
    p->setVisible(false);
  }
  m_pGhostOk=m_pGuienv->addButton(rect<s32>(110,215,210,235),m_pGhostSelect,10005,L"OK");
  m_pGhostCancel=m_pGuienv->addButton(rect<s32>(215,215,310,235),m_pGhostSelect,10006,L"Cancel");
  m_pGhostFile=m_pGuienv->addListBox(rect<s32>(10,30,160,200),m_pGhostSelect,65535);
  m_pGhostFile->setDrawBackground(true);
  m_pGuienv->addStaticText(L"Player",rect<s32>(170,40,310,60),false,true,m_pGhostSelect);
  m_pGuienv->addStaticText(L"Laptime",rect<s32>(170,90,310,110),false,true,m_pGhostSelect);
  m_pGhostPlayer=m_pGuienv->addStaticText(L"my ghost",rect<s32>(170,65,310,85),false,true,m_pGhostSelect,-1,true);
  m_pGhostTime=m_pGuienv->addStaticText(L"my time",rect<s32>(170,115,310,135),false,true,m_pGhostSelect,-1,true);
  m_pGhostSelect->setVisible(false);
}

void CLevel::deactivate(IState *pNext) {
  m_iSelectedLevel=m_pLevelSelect->getSelectedLevel();

  IImage *pLoading=m_pDriver->createImageFromFile("data/loading.png");
  m_iLaps=m_pLaps->getSelected()+2;

  if (pLoading) {
    ITexture *pLoadingTexture=m_pDriver->addTexture("loading",pLoading);
    if (pLoadingTexture) {
      dimension2du screenSize=m_pDriver->getScreenSize();
      position2di pos=position2di(screenSize.Width/2-115,screenSize.Height/2-32);
      m_pGuienv->addImage(pLoadingTexture,pos);
      m_pDriver->beginScene();
      m_pSmgr->drawAll();
      m_pLevelSelect->drawAll();
      m_pGuienv->drawAll();
      m_pDriver->endScene();
    }
    pLoading->drop();
  }

  m_pSmgr->clear();
  m_pGuienv->clear();
	if (m_bOdeInitialized) CIrrOdeManager::getSharedInstance()->closeODE();
	delete m_pLevels;
	delete m_pLevelSelect;
	m_pStateMachine->setDrawScene(true);

  for (u32 i=0; i<m_lGhostFiles.size(); i++) {
    delete m_lGhostFiles[i];
  }
  m_lGhostFiles.clear();
}

u32 CLevel::update() {
  if (!m_pDevice->getCursorControl()->isVisible()) m_pDevice->getCursorControl()->setVisible(true);
  m_pDriver->beginScene(true,true,SColor(0,200,200,200));

  m_pSmgr->drawAll();
  m_pLevelSelect->drawAll();

  m_pDriver->endScene();
  return m_iSelect;
}

s32 CLevel::getLaps() {
	return m_iLaps;
}

bool CLevel::updateLevel() {
  bool bRet=m_pLevelSelect->update();

  if (bRet) {
    if (m_pOptions->netbook()) {
      IFileSystem *pFs=m_pDevice->getFileSystem();
      if (pFs->existFile(path(m_pLevelSelect->getLevelNotebookFile())))
        swprintf(m_sLevelName,1024,L"%s",m_pLevelSelect->getLevelNotebookFile());
      else
        swprintf(m_sLevelName,1024,L"%s",m_pLevelSelect->getLevelFile());
    }
    else swprintf(m_sLevelName,1024,L"%s",m_pLevelSelect->getLevelFile());

    swprintf(m_sHiScoreFile,1024,L"%s",m_pLevelSelect->getHiScoreFile());
    m_pLevelInfo->setText(m_pLevelSelect->getLevelInfo());
    m_pStart->setEnabled(true);

    CHiScoreList *pList=new CHiScoreList(m_pDevice,stringc(m_pLevelSelect->getHiScoreFile()).c_str());
    pList->show(m_pHiScore,m_pGuienv,m_pLevelSelect->getLevelName());
    delete pList;

    m_pLaps->clear();
    for (s32 i=2; i<=15; i++) {
      wchar_t s[0xFF];
      swprintf(s,0xFF,L"%i",i);
      m_pLaps->addItem(s);
    }

    m_pLaps->setEnabled(true);
    m_pLaps->setSelected(m_iLaps-2);
    if (m_pLaps->getSelected()==-1) m_pLaps->setSelected(0);

    m_pSelectNOP[0]->setEnabled(true);
    if (!m_pLevelSelect->isMultiplayer()) {
      m_pSelectNOP[0]->setChecked(true);
      m_pSelectNOP[1]->setEnabled(false);
      m_pSelectNOP[1]->setChecked(false);
      m_iPlayerNo=1;
    }
    else {
      m_pSelectNOP[1]->setEnabled(true);
    }
    return true;
  }
  return false;
}

void CLevel::initGame() {
  if (m_pLogic) delete m_pLogic;
  if (m_pInfo ) delete m_pInfo ;

  
  KFCVWrapper::getInstance()->noPlayers = getPlayerNo(); // HACK
  m_pInfo=new CRaceInfo();
  m_pInfo->setLevel(getLevelName());
  m_pInfo->setSplitHorizontal(splitHorizontal());
  m_pInfo->setGhost(playGhost());
  m_pInfo->setRecordReplay(m_bRecordReplay);
  m_pInfo->setLaps(getLaps());
  m_pInfo->addPlayerName(stringw(m_pOptions->getPlayerName(0)));
  if (getPlayerNo()>1) m_pInfo->addPlayerName(stringw(m_pOptions->getPlayerName(1)));
  m_pInfo->setHiScoreFile(getHiScoreFile());

  m_pLogic=new CGameLogic();
  m_pLogic->setDevice(m_pDevice);
  m_pLogic->setRaceInfo(m_pInfo);
  m_pLogic->setGame(m_pRace);
  m_pLogic->recordGhost(m_pOptions->recordGhost());
  m_pLogic->setNetbook(m_pOptions->netbook());
}

bool CLevel::OnEvent(const SEvent &event) {
  bool bRet=updateLevel();

  if (bRet) return true;

	if (event.EventType==EET_KEY_INPUT_EVENT) {
	  if (event.KeyInput.Key==KEY_ESCAPE) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
      m_iSelect=1;
      bRet=true;
	  }

	  if (event.KeyInput.Key==KEY_RETURN && event.KeyInput.PressedDown) {
      m_iLaps=m_pLaps->getSelected()+2;
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
      if (playGhost() && !m_pGhostSelect->isVisible())
        activateGhostSelect();
      else {
        CIrrOdeKlangManager::getSharedInstance()->removeAllNodes();
        initGame();
        if (m_pGhostSelect->isVisible() && playGhost()) {
          c8 sFile[0xFF];
          sprintf(sFile,"data/ghost/%s",stringc(stringw(m_pGhostFile->getListItem(m_pGhostFile->getSelected()))).c_str());
          m_pInfo->setGhostFile(sFile);
        }
        m_iSelect=6;
      }
      bRet=true;
	  }
	}

  if (event.EventType==EET_GUI_EVENT ) {
    if (event.GUIEvent.EventType==EGET_COMBO_BOX_CHANGED) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
      bRet=true;
    }

    if (event.GUIEvent.EventType==EGET_CHECKBOX_CHANGED) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
      u32 id=event.GUIEvent.Caller->getID();
      if (id==4711)
        m_bHorizontalSplit=m_pSplitHorizontal->isChecked();
      else
        if (id==815)
          m_bRecordReplay=m_pRecordReplay->isChecked();
        else {
          if (id==5) { if (!m_pSelectNOP[0]->isChecked()) m_pSelectNOP[1]->setChecked(true); else m_pSelectNOP[1]->setChecked(false); }
          if (id==6) { if (!m_pSelectNOP[1]->isChecked()) m_pSelectNOP[0]->setChecked(true); else m_pSelectNOP[0]->setChecked(false); }

          for (u32 i=0; i<2; i++) if (m_pSelectNOP[i]->isChecked()) m_iPlayerNo=i+1;

          m_pSplitHorizontal->setEnabled(m_pSelectNOP[1]->isChecked());

          m_pSplitTab->setVisible(m_pSelectNOP[1]->isChecked());
          m_pGhostTab->setVisible(m_pSelectNOP[0]->isChecked());
        }

      if (id==4723) m_bPlayGhost=m_pPlayGhost->isChecked();
			bRet=true;
    }

    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
			if (event.GUIEvent.Caller->getID()==23 || event.GUIEvent.Caller->getID()==10005 || event.GUIEvent.Caller->getID()==10006) {
        m_iLaps=m_pLaps->getSelected()+2;
        if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
        if (playGhost() && !m_pGhostSelect->isVisible() && event.GUIEvent.Caller->getID()==23)
          activateGhostSelect();
        else {
          CIrrOdeKlangManager::getSharedInstance()->removeAllNodes();
          initGame();
          m_iSelect=6;

          if (event.GUIEvent.Caller->getID()==10005 && playGhost()) {
            c8 sFile[0xFF];
            sprintf(sFile,"data/ghost/%s",stringc(stringw(m_pGhostFile->getListItem(m_pGhostFile->getSelected()))).c_str());
            m_pInfo->setGhostFile(sFile);
          }
        }
				bRet=true;
			}

      if (event.GUIEvent.Caller->getID()==24) {
        if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
        m_iSelect=1;
        bRet=true;
      }
    }

    if (event.GUIEvent.EventType==EGET_LISTBOX_CHANGED && event.GUIEvent.Caller->getID()==65535) {
      s32 selected=m_pGhostFile->getSelected();
      if (selected>=0 && selected<(s32)m_lGhostFiles.size()) {
        CGhostInfo *pInfo=m_lGhostFiles[selected];
        m_pGhostPlayer->setText(stringw(pInfo->m_sPlayer).c_str());
        wchar_t ws[0xFF];
        swprintf(ws,0xFF,L"%.3f sec",((f32)pInfo->m_iSteps)*0.016f);
        m_pGhostTime->setText(ws);
      }
    }
  }

  return bRet;
}

const wchar_t *CLevel::getLevelName() {
	return m_sLevelName;
}

const wchar_t *CLevel::getHiScoreFile() {
  return m_sHiScoreFile;
}

u32 CLevel::getPlayerNo() {
	return m_iPlayerNo;
}

bool CLevel::splitHorizontal() {
  //we do also return "true" if we have one player only to
  //get the position of the GUI elements during gameplay right.
  return m_bHorizontalSplit || m_iPlayerNo==1;
}

bool CLevel::playGhost() {
  //Ghost is only available in single player mode
  return m_bPlayGhost && m_iPlayerNo==1;
}

void CLevel::writeConfig(IXMLWriter *pXml) {
  if (pXml) {
    pXml->writeElement(L"level",false);
    pXml->writeLineBreak();

    wchar_t s[0xFF];

    swprintf(s,0xFF,L"%i",m_iSelectedLevel);
    pXml->writeElement(L"selectedLevel",true,L"idx",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_iPlayerNo);
    pXml->writeElement(L"players",true,L"no",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_bHorizontalSplit?1:0);
    pXml->writeElement(L"hsplit",true,L"enabled",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_bPlayGhost?1:0);
    pXml->writeElement(L"ghost",true,L"play",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_bRecordReplay?1:0);
    pXml->writeElement(L"replay",true,L"record",s);
    pXml->writeLineBreak();

    swprintf(s,0xFF,L"%i",m_iLaps);
    pXml->writeElement(L"laps",true,L"count",s);
    pXml->writeLineBreak();

    pXml->writeClosingTag(L"level");
    pXml->writeLineBreak();
  }
}

void CLevel::readConfig(IXMLReaderUTF8 *pXml) {
  if (pXml && !strcmp(pXml->getNodeName(),"level") && pXml->getNodeType()==EXN_ELEMENT)
    while (pXml->read() && strcmp(pXml->getNodeName(),"level")) {
      if (!strcmp(pXml->getNodeName(),"selectedLevel"))
        m_iSelectedLevel=atoi(pXml->getAttributeValue("idx"));

      if (!strcmp(pXml->getNodeName(),"players"))
        m_iPlayerNo=atoi(pXml->getAttributeValue("no"));

      if (!strcmp(pXml->getNodeName(),"hsplit"))
        m_bHorizontalSplit=atoi(pXml->getAttributeValue("enabled"));

      if (!strcmp(pXml->getNodeName(),"ghost"))
        m_bPlayGhost=atoi(pXml->getAttributeValue("play"));

      if (!strcmp(pXml->getNodeName(),"replay"))
        m_bRecordReplay=atoi(pXml->getAttributeValue("record"));

      if (!strcmp(pXml->getNodeName(),"laps"))
      m_iLaps=atoi(pXml->getAttributeValue("count"));
    }
}

void CLevel::setOptions(COptions *pOptions) {
  m_pOptions=pOptions;
}

void CLevel::setRace(CGame *pRace) {
  m_pRace=pRace;
}

void CLevel::activateGhostSelect() {
  IFileSystem *fs=m_pDevice->getFileSystem();
  path cwd=fs->getWorkingDirectory();
  fs->changeWorkingDirectoryTo("data/ghost");

  c8 sPlayer[0xFF],sLevel[0xFF];
  u32 iSteps;
  CGhostPlayer plr(m_pDevice,m_pSndEngine);
  IFileList *list=fs->createFileList();
  CGhostInfo *pFirst=NULL;
  for (u32 i=0; i<list->getFileCount(); i++)
    if (!list->isDirectory(i) && plr.validateGhostFile(list->getFullFileName(i).c_str(),sPlayer,sLevel,&iSteps)) {
      if (strstr(stringc(stringw(getLevelName())).c_str(),sLevel)) {
        m_pGhostFile->addItem(stringw(list->getFileName(i)).c_str());
        CGhostInfo *pInfo=new CGhostInfo();
        if (!pFirst) pFirst=pInfo;
        pInfo->m_iSteps=iSteps;
        strcpy(pInfo->m_sPlayer,sPlayer);
        m_lGhostFiles.push_back(pInfo);
      }
    }

  if (m_pGhostFile->getItemCount()>0) {
    m_pGhostOk->setEnabled(true);
    m_pGhostFile->setSelected(0);

    if (pFirst) {
      m_pGhostPlayer->setText(stringw(pFirst->m_sPlayer).c_str());
      wchar_t ws[0xFF];
      swprintf(ws,0xFF,L"%.3f sec",((f32)pFirst->m_iSteps)*0.016f);
      m_pGhostTime->setText(ws);
    }

    m_pGhostSelect->setVisible(true);
    m_pGuienv->setFocus(m_pGhostSelect);
  }
  else {
    CIrrOdeKlangManager::getSharedInstance()->removeAllNodes();
    initGame();
    m_iSelect=6;
  }

  fs->changeWorkingDirectoryTo(cwd);
}
