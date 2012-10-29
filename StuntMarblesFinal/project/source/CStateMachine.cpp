  #include <CStateMachine.h>
  #include <CSettings.h>
  #include <COptions.h>
  #include <CHiScore.h>
  #include <CCredits.h>
  #include <CMenu.h>
  #include <CLevel.h>
  #include <CGame.h>
  #include <IrrOde.h>
	#include <CIrrOdeKlangNode.h>
	#include <CIrrOdeKlangManager.h>
	#include <CTournamentRace.h>
	#include <CTournament.h>
	#include <CReplaySelect.h>
	#include <CReplay.h>
	#include <CPlugBULLETBYTEin.h>
	#include <CNetGame.h>
	#include <CRenderToTextureManager.h>
	#include <CRaceTrackGenerator.h>
	#include <CRaceTrackLoader.h>

	#include <IrrOdeNet/CMarbles2WorldObserver.h>
	#include <IrrOdeNet/CIrrOdeMessageFactory.h>
	#include <IrrOdeNet/CMarbleMessageFactory.h>

CStateMachine::CStateMachine() {
  m_pActive=NULL;
	m_bDrawScene=true;
}

void CStateMachine::addState(IState *pState) {
  pState->setNum(m_aStates.size());
  m_aStates.push_back(pState);
}

void CStateMachine::initStates(IrrlichtDevice *pDevice) {
  //First of all we add the data file with all the stuff we need for our states
	 m_pDevice->getFileSystem()->addZipFileArchive("data/menu.dat" ,false,false);

  CIrrOdeManager::getSharedInstance()->install(pDevice);

	//make sure no old (already deleted) config file readers or writers are stored
  CConfigFileManager::getSharedInstance()->clearReadersWriters();

	//now create all of the main states, set their index number and add them to the array
  m_pMenu      =new CMenu          (pDevice,this); addState(m_pMenu      );
  m_pLevl      =new CLevel         (pDevice,this); addState(m_pLevl      );
  m_pOpts      =new COptions       (pDevice,this); addState(m_pOpts      );
  m_pHiSc      =new CHiScore       (pDevice,this); addState(m_pHiSc      );
  m_pCred      =new CCredits       (pDevice,this); addState(m_pCred      );
  m_pGame      =new CGame          (pDevice,this); addState(m_pGame      );
  m_pTournament=new CTournament    (pDevice,this); addState(m_pTournament);
  m_pTrnRace   =new CTournamentRace(pDevice,this); addState(m_pTrnRace   );
  m_pReplaySel =new CReplaySelect  (pDevice,this); addState(m_pReplaySel );
  m_pReplay    =new CReplay        (pDevice,this); addState(m_pReplay    );
  m_pNetGame   =new CNetGame       (pDevice,this); addState(m_pNetGame   );

	//activate the menu state
  m_pActive=m_pMenu;

	//now we add all objects that read and write configuration data to the config file manager
  CConfigFileManager::getSharedInstance()->addReader(m_pOpts      );
  CConfigFileManager::getSharedInstance()->addReader(m_pLevl      );
  CConfigFileManager::getSharedInstance()->addReader(m_pTournament);
  CConfigFileManager::getSharedInstance()->addReader(m_pReplaySel );
  CConfigFileManager::getSharedInstance()->addWriter(m_pOpts      );
  CConfigFileManager::getSharedInstance()->addReader(m_pNetGame   );
  CConfigFileManager::getSharedInstance()->addWriter(m_pLevl      );
  CConfigFileManager::getSharedInstance()->addWriter(m_pTournament);
  CConfigFileManager::getSharedInstance()->addWriter(m_pReplaySel );
  CConfigFileManager::getSharedInstance()->addWriter(m_pNetGame   );

	//Some states need to get references to other states
  m_pOpts      ->setGame        (m_pGame);
	m_pGame      ->setOptions     (m_pOpts);
	m_pLevl      ->setOptions     (m_pOpts);
	m_pLevl      ->setRace        (m_pGame);
	m_pTrnRace   ->setRace        (m_pGame);
	m_pTrnRace   ->setOptions     (m_pOpts);
	m_pTournament->setOptions     (m_pOpts);
	m_pTournament->setRace        (m_pTrnRace);
	m_pReplay    ->setReplaySelect(m_pReplaySel);
	m_pNetGame   ->setOptions     (m_pOpts);
	m_pNetGame   ->setRace        (m_pTrnRace);

	//Let's create something for sound
	m_pSndEngine=irrklang::createIrrKlangDevice();
	CIrrOdeKlangManager::getSharedInstance()->setSoundEngine(m_pSndEngine);

	//init our members
  m_pDevice=pDevice;
  m_pDriver=m_pDevice->getVideoDriver();
  m_pSmgr  =m_pDevice->getSceneManager();
  m_pGuienv=m_pDevice->getGUIEnvironment();

	//create a new scene manager for the level preview
  m_pPreviewManager=m_pSmgr->createNewSceneManager(false);

	//activate the active state
  m_pActive->activate(NULL);

	//change the default font
  m_pFont=m_pGuienv->getFont("data/fontcourier.bmp");
  m_pGuienv->getSkin()->setFont(m_pFont);

	//register all necessary scene node factories
  CIrrOdeSceneNodeFactory *cFactory=new CIrrOdeSceneNodeFactory(m_pSmgr);
  m_pDevice->getSceneManager()->registerSceneNodeFactory(cFactory);
  m_pPreviewManager->registerSceneNodeFactory(cFactory);
  cFactory->drop();

  CPlugBULLETBYTEin *pPlugin=new CPlugBULLETBYTEin(m_pSmgr);
  m_pDevice->getSceneManager()->registerSceneNodeFactory(pPlugin);
  m_pPreviewManager->registerSceneNodeFactory(pPlugin);

  CIrrOdeKlangNodeFactory *cKlang=new CIrrOdeKlangNodeFactory(m_pSmgr);
  m_pSmgr->registerSceneNodeFactory(cKlang);
  m_pPreviewManager->registerSceneNodeFactory(cKlang);
  cKlang->drop();

  CConfigFileManager::getSharedInstance()->loadConfig(m_pDevice,"data/settings.xml");

  CMarbles2WorldObserver::getSharedInstance()->install("marbles2");
  CMarbles2WorldObserver::getSharedInstance()->setIrrlichtDevice(m_pDevice);

  CIrrOdeMessageFactory::getSharedMessageFactory()->setSceneManager(m_pSmgr);
	CMarbleMessageFactory *pFact=new CMarbleMessageFactory();
	CIrrOdeMessageFactory::getSharedMessageFactory()->registerMessageFactory(pFact);

	CRenderToTextureManager::getSharedInstance()->setSceneManager(m_pSmgr);

	CRaceTrackLoader *pRTLoader=new CRaceTrackLoader(m_pDevice);
	m_pSmgr->addExternalMeshLoader(pRTLoader);
	m_pPreviewManager->addExternalMeshLoader(pRTLoader);
}

void CStateMachine::clearStates() {
  //in this method we remove all states from the various lists they are in
  CConfigFileManager::getSharedInstance()->removeReader(m_pOpts);
  CConfigFileManager::getSharedInstance()->removeWriter(m_pOpts);

  for (u32 i=0; i<m_aStates.size(); i++) delete m_aStates[i];
  m_aStates.clear();

  CMarbles2WorldObserver::getSharedInstance()->destall();
}

void CStateMachine::graphicsChanged() {
  //This method gets called if another state wants to re-init the graphics mode
  m_bGraphicsChanged=true;
}

void CStateMachine::setDrawScene(bool b) {
  //sometimes (e.g. splitscreen) the states will do the draw-stuff on their own
	m_bDrawScene=b;
}

u32 CStateMachine::run() {
  bool bQuit,bSettings=false;
  s32 lastFPS=-1;
  m_bGraphicsChanged=false;

  do {
    do {
      //read the settings from the graphics settings file and create an Irrlicht device from these settings
      CSettings *setup=new CSettings("data/setup_scene.xml","data/graphics.xml",L"Stunt Marble Racers",SColor(0x00,0x21,0xAD,0x10),NULL);

      setup->addValidDriver(L"Direct3D 9",EDT_DIRECT3D9);
      setup->addValidDriver(L"OpenGL",EDT_OPENGL);
      setup->setMinResolution(dimension2di(800,600));
      setup->createGUI();

      if (setup->runFirst() || bSettings) if (setup->run()!=1) return 0;
      m_pDevice=setup->createDeviceFromSettings();
      delete setup;
      bSettings=m_pDevice==NULL;
    }
    while (m_pDevice==NULL);

    m_pDevice->setWindowCaption(L"Dustbin::Games -Kinect Stunt Marble Racers");
    initStates(m_pDevice);
    m_bGraphicsChanged=false;
    bQuit=false;

		//This loop will run until the game is quit or the graphics mode is to be re-inited
    while(m_pDevice->run() && !bQuit && !m_bGraphicsChanged) {
		  //m_bDrawScene is "false" if the active state wants to do all the drawing stuff on it's own
			if (m_bDrawScene) {
				m_pDriver->beginScene(true,true,SColor(0,200,200,200));

				m_pSmgr->drawAll();
				m_pGuienv->drawAll();

				m_pDriver->endScene();
			}
      s32 fps=m_pDriver->getFPS();

      if (lastFPS != fps) {
        m_iFps=fps;
        core::stringw str = L"Dustbin::Games - Kinect Marble Racers [";
        str += m_pDriver->getName();
        str += "] FPS:";
        str += fps;

        m_pDevice->setWindowCaption(str.c_str());
        lastFPS = fps;
      }

      u32 iRet=m_pActive?m_pActive->update():0;

			//a return value of "0" from the active state shows that no state change is made
      if (iRet) {
			  //the index of the state to be changed to is the return value -1 (remember: "0" means "no change")
        iRet--;
				//an out of bounds index is interpreted as "quit program" signal
        if (iRet<m_aStates.size()) {
				  //get the next state
          IState *pNext=m_aStates[iRet];
					//deactivate the current state
          m_pActive->deactivate(pNext);
					//activate the next state
          pNext->activate(m_pActive);
					//save config file on each state change
          CConfigFileManager::getSharedInstance()->writeConfig(m_pDevice,"data/settings.xml");
          m_pActive=pNext;
        }
        else bQuit=true;
      }
    }
    if (m_pActive) m_pActive->deactivate(NULL);

    clearStates();
    CShadowManager::getSharedInstance()->clear();
    m_pSmgr->getMeshCache()->clear();
    m_pDevice->closeDevice();
    m_pDevice->drop();
  }
  while (m_bGraphicsChanged);
  return 0;
}

ISoundEngine *CStateMachine::getSoundEngine() {
  return m_pSndEngine;
}

f32 CStateMachine::getAspectRatio() {
  core::rect<s32> viewPort=m_pDriver->getViewPort();
  return 1.0f*viewPort.getWidth()/viewPort.getHeight();
}

ISceneManager *CStateMachine::getPreviewSceneManager() {
  return m_pPreviewManager;
}

u32 CStateMachine::getFps() {
  return m_iFps;
}
