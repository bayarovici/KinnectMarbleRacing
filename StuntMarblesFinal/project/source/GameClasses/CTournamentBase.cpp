	#include <CStateMachine.h>
	#include <GameClasses/CLevelList.h>
	#include <GameClasses/CTournamentInfo.h>
	#include <GameClasses/CTournamentBase.h>
	#include <CTournamentRace.h>
	#include <COptions.h>

CTournamentBase::CTournamentRaceGUI::CTournamentRaceGUI(IrrlichtDevice *pDevice, u32 iId, IGUIElement *pParent, bool bCanRemoveRace) {
  IGUIEnvironment *pGuienv=pDevice->getGUIEnvironment();
  m_pTab=pGuienv->addTab(rect<s32>(position2di(5,5+iId*25),dimension2di(520,20)),pParent);
  m_pTab->setBackgroundColor(SColor(128,128,128,128));
  m_pTab->setDrawBackground(true);

  wchar_t s[0xFF];
  swprintf(s,0xFF,L"%i | Track",iId+1);
  m_pNum=pGuienv->addStaticText(s,rect<s32>(1,1,80,18),false,true,m_pTab);
  m_pNum->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  m_iId=iId;

  m_pTrack=pGuienv->addComboBox(rect<s32>(85,1,390,18),m_pTab,100+iId);

  CLevelList *pLevelList=new CLevelList(pDevice);
  for (u32 i=0; i<pLevelList->m_aLevels.size(); i++) m_pTrack->addItem(pLevelList->m_aLevels[i]->m_sName.c_str());
  delete pLevelList;

  pGuienv->addStaticText(L"Laps",rect<s32>(395,1,430,18),false,true,m_pTab);

  m_pLaps=pGuienv->addComboBox(rect<s32>(435,1,475,18),m_pTab,150+iId);
  for (u32 i=2; i<=15; i++) {
    swprintf(s,0xFF,L"%i",i);
    m_pLaps->addItem(s);
  }

  m_pDel=pGuienv->addButton(rect<s32>(480,1,515,18),m_pTab,23+iId,L"Del");
  if (!bCanRemoveRace) {
    m_pDel->setVisible(false);
    m_pTrack->setEnabled(false);
    m_pLaps->setEnabled(false);
  }
}

CTournamentBase::CTournamentRaceGUI::~CTournamentRaceGUI() {
  m_pTab->remove();
}

const wchar_t *CTournamentBase::CTournamentRaceGUI::getTrack() {
  return m_pTrack->getItem(m_pTrack->getSelected());
}

u32 CTournamentBase::CTournamentRaceGUI::getLaps() {
  return m_pLaps->getSelected()+2;
}

void CTournamentBase::CTournamentRaceGUI::setId(u32 iId) {
  wchar_t s[0xFF];
  swprintf(s,0xFF,L"%i | Track",iId+1);
  m_pNum->setText(s);
  m_iId=iId;
  m_pTab->setRelativePosition(position2di(5,5+iId*25));
  m_pDel->setID(23+iId);
}

u32 CTournamentBase::CTournamentRaceGUI::getId() {
  return m_pDel->getID();
}

void CTournamentBase::CTournamentRaceGUI::setTrack(const wchar_t *sTrack) {
  for (u32 i=0; i<m_pTrack->getItemCount(); i++) {
    if (!wcscmp(sTrack,m_pTrack->getItem(i))) {
      m_pTrack->setSelected(i);
      return;
    }
  }
  m_pTrack->setText(sTrack);
}

void CTournamentBase::CTournamentRaceGUI::setLaps(u32 iLaps) {
  m_pLaps->setSelected(iLaps-2);
}

CTournamentBase::~CTournamentBase() {
  list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
  while (m_lRaces.getSize()>0) {
    CTournamentRaceGUI *p=*it;
    m_lRaces.erase(it);
    delete p;
    it=m_lRaces.begin();
  }
  m_lRaces.clear();
}

CTournamentBase::CTournamentBase(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_pTournament=NULL;
  m_pRace=NULL;
  m_iPlayerNo=0;
  m_bRecordReplay=false;
}

void CTournamentBase::fillTournamentList() {
  if (!m_bNetClient) {
    m_pTournaments->clear();
    IFileSystem *pFs=m_pDevice->getFileSystem();
    c8 sDir[0xFFF];
    strcpy(sDir,pFs->getWorkingDirectory().c_str());
    c8 sTournamentDir[0xFF];

    sprintf(sTournamentDir,"%s/data/tournaments",sDir);
    pFs->changeWorkingDirectoryTo(sTournamentDir);

    IFileList *pTournaments=pFs->createFileList();

    for (u32 i=0; i<pTournaments->getFileCount(); i++) {
      if (!pTournaments->isDirectory(i) && strstr(pTournaments->getFileName(i).c_str(),".trn")!=NULL) {
        c8 sName[0xFFF];
        strcpy(sName,pTournaments->getFileName(i).c_str());
        if (strstr(sName,".trn")) *strstr(sName,".trn")='\0';
        m_pTournaments->addItem(stringw(sName).c_str());
      }
    }

    pTournaments->drop();
    pFs->changeWorkingDirectoryTo(sDir);
  }
}

void CTournamentBase::fillTournamentGUI() {
  if (m_bNetClient) return;

  m_pName->setText(m_pTournament?m_pTournament->getName():L"new tournament");

  if (m_pTournament)
    for (u32 i=0; i<m_pTournament->getRaceCount(); i++) {
      CTournamentRaceGUI *pRace=new CTournamentRaceGUI(m_pDevice,m_lRaces.getSize(),m_pRaceTab,!m_bNetClient);
      pRace->setTrack(m_pTournament->getRace(i)->getTrack());
      pRace->setLaps(m_pTournament->getRace(i)->getLaps());
      m_lRaces.push_back(pRace);

      addTrack(i);
      trackChanged(i,stringc(m_pTournament->getRace(i)->getTrack()).c_str());
      lapsChanged(i,m_pTournament->getRace(i)->getLaps());
    }

  m_pRaceListButtons->setRelativePosition(position2di(5,5+m_lRaces.getSize()*25));
  if (m_lRaces.getSize()>=20) m_pRaceListButtons->setVisible(false);
}

void CTournamentBase::activate(IState *pPrevious) {
  m_pDevice->getCursorControl()->setVisible(true);
  m_pSmgr->loadScene("data/setup_scene.xml");
  m_iSelect=0;

  dimension2du screenSize=m_pDriver->getScreenSize();
  s32 iLeftX=screenSize.Width/2-400,iRightX=screenSize.Width/2+400;
  m_pTab=m_pGuienv->addTab(rect<s32>(iLeftX,0,iRightX,600));

  IGUIStaticText *pText=m_pGuienv->addStaticText(L"Tournament Name:",rect<s32>(10,50,250,65),false,true,m_pTab);
  pText->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  pText=m_pGuienv->addStaticText(L"Races",rect<s32>(260,50,790,65),false,true,m_pTab);
  pText->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

  m_pRaceTab=m_pGuienv->addTab(rect<s32>(260,70,790,590),m_pTab);
  m_pRaceTab->setBackgroundColor(SColor(128,128,128,128));
  m_pRaceTab->setDrawBackground(true);

  if (!m_bNetClient) {
    m_pRaceListButtons=m_pGuienv->addTab(rect<s32>(10,10,780,30),m_pRaceTab);
    m_pAddRace=m_pGuienv->addButton(rect<s32>(0,0,180,20),m_pRaceListButtons,1,L"Add Race",L"");
    m_pClearList=m_pGuienv->addButton(rect<s32>(190,0,370,20),m_pRaceListButtons,2,L"Clear List",L"");

    m_pStart =m_pGuienv->addButton(rect<s32>(10,430,200,450),m_pTab,3,L"Start Tournament");
    m_pLoad  =m_pGuienv->addButton(rect<s32>(10,545,200,565),m_pTab,5,L"Load Tournament");
    m_pSave  =m_pGuienv->addButton(rect<s32>(10,570,200,590),m_pTab,6,L"Save Tournament");

    m_pRecordReplay=m_pGuienv->addCheckBox(m_bRecordReplay,rect<s32>(10,100,30,120),m_pTab);
    IGUIStaticText *text=m_pGuienv->addStaticText(L"Record Replay",rect<s32>(31,100,190,120),false,true,m_pTab,-1,true);
    text->setTextAlignment(EGUIA_UPPERLEFT,EGUIA_CENTER);
    m_pTournaments=m_pGuienv->addComboBox(rect<s32>(10,515,200,540),m_pTab);
  }
  else printf("\n\t\tnet client!\n\n");
  m_pName=m_pGuienv->addEditBox(L"",rect<s32>(10,70,250,90),true,m_pTab);
  if (m_bNetClient) m_pName->setEnabled(false);

  m_pCancel=m_pGuienv->addButton(rect<s32>(10,455,200,475),m_pTab,4,L"Cancel");

  fillTournamentList();
  fillTournamentGUI();
}

void CTournamentBase::deactivate(IState *pNext) {
  fillTournamentStructure();
  while (m_lRaces.getSize()>0) {
    list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
    CTournamentRaceGUI *p=*it;
    m_lRaces.erase(it);
    delete p;
  }
  m_lRaces.clear();

  m_pGuienv->clear();
  m_pSmgr->clear();
}

u32 CTournamentBase::update() {
  if (!m_pDevice->getCursorControl()->isVisible()) m_pDevice->getCursorControl()->setVisible(true);
  return m_iSelect;
}

bool CTournamentBase::OnEvent (const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown) {
	  if (event.KeyInput.Key==KEY_ESCAPE) {
      if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
      m_iSelect=1;
      bRet=true;
	  }

	  if (event.KeyInput.Key==KEY_RETURN) {
	    printf("Start Tournament\n");
	    fillTournamentList();
	    bRet=startTournament();
	  }
  }

  if (event.EventType==EET_GUI_EVENT ) {
    if (event.GUIEvent.EventType==EGET_COMBO_BOX_CHANGED) {
      u32 id=event.GUIEvent.Caller->getID();
      if (event.GUIEvent.Caller->getID()>=150) {
        lapsChanged(id-150,((IGUIComboBox *)event.GUIEvent.Caller)->getSelected()+2);
      }
      else
        if (id>=100) {
          list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
          for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
            if (id-77==(*it)->getId()) {
              CTournamentRaceGUI *p=*it;
              trackChanged(id-100,stringc(p->getTrack()).c_str());
            }
          }
        }
    }

    if (event.GUIEvent.EventType==EGET_ELEMENT_FOCUS_LOST) {
      if (event.GUIEvent.Caller==m_pName) tournamentNameChanged();
    }

    if (event.GUIEvent.EventType==EGET_ELEMENT_FOCUSED) {
      IGUIElement *pCaller=event.GUIEvent.Caller,*pParent=pCaller->getParent();
      while (pParent!=NULL) {
        pParent->bringToFront(pCaller);
        pCaller=pParent;
        pParent=pParent->getParent();
      }
    }

    if (event.GUIEvent.EventType==EGET_CHECKBOX_CHANGED) {
      if (event.GUIEvent.Caller==m_pRecordReplay) {
        m_bRecordReplay=m_pRecordReplay->isChecked();
      }
    }

    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
      u32 id=event.GUIEvent.Caller->getID();
      if (m_pSndEngine && id!=4) m_pSndEngine->play2D("data/sounds/menuchange.ogg");

      if (id==1) {
        addTrack(m_lRaces.getSize());
        CTournamentRaceGUI *pRace=new CTournamentRaceGUI(m_pDevice,m_lRaces.getSize(),m_pRaceTab,!m_bNetClient);
        m_lRaces.push_back(pRace);
        m_pRaceListButtons->setRelativePosition(position2di(5,5+m_lRaces.getSize()*25));
        if (m_lRaces.getSize()>=20) m_pRaceListButtons->setVisible(false);
      }

      if (id==2) {
        list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
        while (m_lRaces.getSize()>0) {
          CTournamentRaceGUI *p=*it;
          m_lRaces.erase(it);
          delete p;
          it=m_lRaces.begin();
        }
        m_lRaces.clear();
        m_pRaceListButtons->setRelativePosition(position2di(5,5+m_lRaces.getSize()*25));
        clearList();
      }

      if (id==3) {
        printf("Start Tournament\n");
        fillTournamentList();
        bRet=startTournament();
      }

      if (id==4) {
        m_iSelect=1;
        if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
        bRet=true;
      }

      if (id==5) {
        printf("\t\tload tournament\n");
        c8 sFileName[0xFFF];
        sprintf(sFileName,"data/tournaments/%s.trn",stringc(stringw(m_pTournaments->getText())).c_str());

        while (m_lRaces.getSize()>0) {
          list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
          CTournamentRaceGUI *p=*it;
          m_lRaces.erase(it);
          delete p;
        }
        clearList();

        printf("loading tournament \"%s\"...\n",sFileName);
        IXMLReaderUTF8 *pXml=m_pDevice->getFileSystem()->createXMLReaderUTF8(sFileName);
        if (pXml) {
          bool bConfig=false;
          while (pXml->read()) {
            bConfig=bConfig || (!strcmp(pXml->getNodeName(),"settings") && pXml->getNodeType()==EXN_ELEMENT);
            if (bConfig) {
              CTournamentBase::readConfig(pXml);
            }
          }
          pXml->drop();
          fillTournamentGUI();
        }
        bRet=true;
      }

      if (id==6) {
        fillTournamentStructure();

        c8 sFileName[0xFFF];
        sprintf(sFileName,"data/tournaments/%s.trn",stringc(stringw(m_pName->getText())).c_str());
        IXMLWriter *pXml=m_pDevice->getFileSystem()->createXMLWriter(sFileName);

        if (pXml) {
          pXml->writeXMLHeader();
          pXml->writeElement(L"settings");
          pXml->writeLineBreak();
          writeConfig(pXml);
          pXml->writeClosingTag(L"settings");
          pXml->writeLineBreak();
          pXml->drop();
        }
        bRet=true;
      }

      if (id>=23) {
        list<CTournamentRaceGUI *>::Iterator it;
        for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
          if (id==(*it)->getId()) {
            CTournamentRaceGUI *p=*it;
            m_lRaces.erase(it);
            delete p;
            u32 cnt=0;
            for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) (*it)->setId(cnt++);
            m_pRaceListButtons->setRelativePosition(position2di(5,5+m_lRaces.getSize()*25));
            m_pRaceListButtons->setVisible(true);
            delTrack(id);
            return true;
          }
        }
      }
    }
  }

  return bRet;
}

void CTournamentBase::writeConfig(IXMLWriter *pXml) {
  if (pXml && m_pTournament && m_pTournament->getRaceCount()>0) {
    wchar_t s[0xFF];
    swprintf(s,0xFF,L"%i",m_pTournament->getRaceCount());
    pXml->writeElement(L"data",true,L"name",m_pTournament->getName(),L"count",s);
    pXml->writeLineBreak();
    swprintf(s,0xFF,L"%i",m_iPlayerNo);
    pXml->writeElement(L"players",true,L"number",s);
    pXml->writeLineBreak();
    swprintf(s,0xFF,L"%i",m_bRecordReplay?1:0);
    pXml->writeElement(L"replay",true,L"record",s);
    pXml->writeLineBreak();

    for (u32 i=0; i<m_pTournament->getRaceCount(); i++) {
      swprintf(s,0xFF,L"%i",m_pTournament->getRace(i)->getLaps());
      pXml->writeElement(L"race",true,L"track",m_pTournament->getRace(i)->getTrack(),L"laps",s);
      pXml->writeLineBreak();
    }
  }
}

void CTournamentBase::readConfig(IXMLReaderUTF8 *pXml) {
  if (!strcmp(pXml->getNodeName(),"data")) {
    if (m_pTournament!=NULL) delete m_pTournament;
    m_pTournament=new CTournamentInfo(stringw(pXml->getAttributeValue("name")).c_str());
  }

  if (!strcmp(pXml->getNodeName(),"players")) {
    m_iPlayerNo=atoi(pXml->getAttributeValue("number"));
  }

  if (!strcmp(pXml->getNodeName(),"replay")) {
    m_bRecordReplay=atoi(pXml->getAttributeValue("record"));
  }

  if (m_pTournament) {
    if (!strcmp(pXml->getNodeName(),"race")) {
      u32 iLaps=atoi(pXml->getAttributeValue("laps"));
      CTournamentRaceInfo *pRace=new CTournamentRaceInfo(stringw(pXml->getAttributeValue("track")).c_str(),iLaps);
      m_pTournament->addRace(pRace);
    }
  }
}

void CTournamentBase::setRace(CTournamentRace *pRace) {
  m_pRace=pRace;
}

void CTournamentBase::setOptions(COptions *pOptions) {
  m_pOptions=pOptions;
}

bool CTournamentBase::startTournament() {
  if (m_pRace) {
    fillTournamentStructure();
    if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
    m_pRace->setTournamentInfo(m_pTournament);
    return true;
  }
  return false;
}
