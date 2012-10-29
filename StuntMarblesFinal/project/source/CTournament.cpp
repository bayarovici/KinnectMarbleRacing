  #include <CTournament.h>
	#include <CStateMachine.h>
	#include <GameClasses/CLevelList.h>
	#include <GameClasses/CTournamentInfo.h>
	#include <CTournamentRace.h>
	#include <COptions.h>

CTournament::~CTournament() {
  list<CTournamentRaceGUI *>::Iterator it=m_lRaces.begin();
  while (m_lRaces.getSize()>0) {
    CTournamentRaceGUI *p=*it;
    m_lRaces.erase(it);
    delete p;
    it=m_lRaces.begin();
  }
  m_lRaces.clear();
}

CTournament::CTournament(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : CTournamentBase(pDevice,pStateMachine) {
  m_pRace=NULL;
  m_iHSplit=0;
  m_iPlayerNo=0;
  m_bRecordReplay=false;
}

void CTournament::activate(IState *pPrevious) {
  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_bNetClient=false;
  CTournamentBase::activate(pPrevious);

  IGUIFont *pBigFont=m_pGuienv->getFont("data/font2.xml");
  IGUIStaticText *pHeadline=m_pGuienv->addStaticText(L"Kinect Marble Racers - Tournament",rect<s32>(10,10,790,40),false,true,m_pTab);
  pHeadline->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  pHeadline->setOverrideFont(pBigFont);
  pHeadline->setBackgroundColor(SColor(128,16,16,16));
  pHeadline->setOverrideColor(SColor(255,255,118,70));

  m_pHSplit=m_pGuienv->addComboBox(rect<s32>(10,160,250,180),m_pTab);
  m_pHSplit->addItem(L"Horizontal Splitscreen");
  m_pHSplit->addItem(L"Vertical Splitscreen");

  m_pHSplit->setSelected(m_iHSplit);
  m_pHSplit->setEnabled(m_iPlayerNo!=0);

  IGUIStaticText *pText=m_pGuienv->addStaticText(L"Number of Players:",rect<s32>(10,130,190,150),false,true,m_pTab);
  pText->setDrawBorder(false);
  pText->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

  m_pPlayerNo=m_pPlayerNo=m_pGuienv->addComboBox(rect<s32>(200,130,250,150),m_pTab,10001);
  m_pPlayerNo->setSelected(0);
  m_pPlayerNo->addItem(L"1");
  m_pPlayerNo->addItem(L"2");
  m_pPlayerNo->setSelected(m_iPlayerNo);

  m_pDevice->setEventReceiver(this);
}

void CTournament::deactivate(IState *pNext) {
  CTournamentBase::deactivate(pNext);
}

bool CTournament::startTournament() {
  m_pTournament->setPlayerNo(m_pPlayerNo->getSelected()+1);
  if (CTournamentBase::startTournament()) {
    m_pRace->setNetAdapter(NULL);
    m_iSelect=8;
    return true;
  }
  return false;
}

void CTournament::fillTournamentStructure() {
  if (m_pTournament) delete m_pTournament;
  m_pTournament=new CTournamentInfo(m_pName->getText());
  list<CTournamentRaceGUI *>::Iterator it;

  for (it=m_lRaces.begin(); it!=m_lRaces.end(); it++) {
    CTournamentRaceGUI *p=*it;
    CTournamentRaceInfo *pRace=new CTournamentRaceInfo(p->getTrack(),p->getLaps());
    m_pTournament->addRace(pRace);
  }
  m_pTournament->setPlayerNo(m_iPlayerNo+1);
  m_pTournament->addPlayerName(m_pOptions->getPlayerName(0));
  m_pTournament->addPlayerName(m_pOptions->getPlayerName(1));
  m_pTournament->setRecordReplay(m_bRecordReplay);
  m_pTournament->setSplitHorizontal(m_iHSplit==0);
}

bool CTournament::OnEvent (const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_GUI_EVENT ) {
    if (event.GUIEvent.EventType==EGET_ELEMENT_FOCUSED) {
      IGUIElement *pCaller=event.GUIEvent.Caller,*pParent=pCaller->getParent();
      while (pParent!=NULL) {
        pParent->bringToFront(pCaller);
        pCaller=pParent;
        pParent=pParent->getParent();
      }
    }


    if (event.GUIEvent.EventType==EGET_COMBO_BOX_CHANGED) {
      if (event.GUIEvent.Caller==m_pPlayerNo) {
        bool b=m_pPlayerNo->getSelected()==1;
        m_pHSplit->setEnabled(b);
        m_iPlayerNo=m_pPlayerNo->getSelected();
      }

      if (event.GUIEvent.Caller==m_pHSplit) {
        if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
        m_iHSplit=m_pHSplit->getSelected();
        bRet=true;
      }
    }
  }
  if (!bRet) bRet=CTournamentBase::OnEvent(event);
  return bRet;
}

void CTournament::writeConfig(IXMLWriter *pXml) {
  wchar_t s[0xFF];

  if (pXml && m_pTournament && m_pTournament->getRaceCount()>0) {
    pXml->writeElement(L"tournament",false);
    pXml->writeLineBreak();

    CTournamentBase::writeConfig(pXml);

    swprintf(s,0xFF,L"%i",m_iHSplit);
    pXml->writeElement(L"split",true,L"horizontal",s);
    pXml->writeLineBreak();

    pXml->writeClosingTag(L"tournament");
    pXml->writeLineBreak();
  }
}

void CTournament::readConfig(IXMLReaderUTF8 *pXml) {
  if (pXml && !strcmp(pXml->getNodeName(),"tournament") && pXml->getNodeType()==EXN_ELEMENT) {
    if (m_pTournament) {
      delete m_pTournament;
      m_pTournament=NULL;
    }
    while (pXml->read() && strcmp(pXml->getNodeName(),"tournament")) {
      CTournamentBase::readConfig(pXml);
      if (!strcmp(pXml->getNodeName(),"split")) {
        m_iHSplit=atoi(pXml->getAttributeValue("horizontal"));
      }
    }
  }
}
