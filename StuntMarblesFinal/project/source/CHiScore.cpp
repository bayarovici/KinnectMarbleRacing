  #include <CHiScore.h>
  #include <CStateMachine.h>
  #include <CFollowPathAnimator.h>
  #include <IrrOde.h>
  #include <GameClasses/CLevelList.h>
  #include <GameClasses/CHiScoreList.h>
  #include <GameClasses/CLevelSelectGUI.h>

CHiScore::CHiScore(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
}

void CHiScore::activate(IState *pPrevious) {
  dimension2du screenSize=m_pDriver->getScreenSize();
  m_iRet=0;
  m_pDevice->getSceneManager()->loadScene("data/menu_scene.xml");
  m_pSmgr->clear();
  m_pGuienv->clear();
  m_pDevice->getCursorControl()->setVisible(true);

	m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pLevelSelect=new CLevelSelectGUI(m_pDevice,m_pSndEngine,m_pStateMachine->getPreviewSceneManager(),true);

  m_pSmgr->loadScene("data/setup_scene.xml");
  m_pBigFont=m_pGuienv->getFont("data/font2.xml");

  IGUITab *pTab=m_pLevelSelect->getTab();

  m_pHiScoreList=m_pGuienv->addStaticText(L"Hello World!",rect<s32>(10,200,790,590),false,true,pTab);
  m_pHiScoreList->setDrawBackground(true);
  m_pHiScoreList->setOverrideFont(m_pBigFont);
  m_pHiScoreList->setBackgroundColor(SColor(192,192,192,192));

  m_pSmgr->getActiveCamera()->setAspectRatio(m_pStateMachine->getAspectRatio());
  m_pStateMachine->setDrawScene(false);

  updateHiScore();
  m_pDevice->setEventReceiver(this);
}

void CHiScore::deactivate(IState *pNext) {
  m_pStateMachine->setDrawScene(true);
  m_pSmgr->clear();
  m_pGuienv->clear();
  delete m_pLevelSelect;
}

u32 CHiScore::update() {
  if (!m_pDevice->getCursorControl()->isVisible()) m_pDevice->getCursorControl()->setVisible(true);
  m_pDriver->beginScene(true,true,SColor(0,200,200,200));

  m_pSmgr->drawAll();
  m_pLevelSelect->drawAll();

  m_pDriver->endScene();
  return m_iRet;
}

bool CHiScore::OnEvent(const SEvent &event) {
  if (event.EventType==EET_KEY_INPUT_EVENT && event.KeyInput.Key==KEY_ESCAPE) {
    if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
    m_iRet=1;
  }
  updateHiScore();
  return false;
}

void CHiScore::updateHiScore() {
  if (m_pLevelSelect->update()) {
    CHiScoreList *pList=new CHiScoreList(m_pDevice,stringc(m_pLevelSelect->getHiScoreFile()).c_str());
    pList->show(m_pHiScoreList,m_pGuienv,m_pLevelSelect->getLevelName());
    delete pList;
  }
}
