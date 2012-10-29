  #include <time.h>
  #include <CGame.h>
  #include <CStateMachine.h>
  #include <CShadowManager.h>
	#include <GameClasses/CHiScoreList.h>
	#include <CConfigFileManager.h>
	#include <CIrrOdeKlangManager.h>
	#include <CIrrOdeKlangNode.h>
	#include <CRenderToTextureManager.h>
	#include <irrKlang.h>
	#include <COptions.h>
	#include <CRespawnNode.h>
	#include <CReplayCam.h>
	#include <irrCC.h>
	#include <CProgressBar.h>

	#include <IrrOdeNet/CMarbles2Recorder.h>

	#include <GameClasses/CMarbleFollowAnimator.h>
	#include <GameClasses/CGhostPlayer.h>
	#include <GameClasses/IGameLogic.h>
	#include <GameClasses/CRaceInfo.h>

	#define _BORDER 5

GUIElementToToggle::GUIElementToToggle(IGUIElement *pElement, u32 iTime, bool bVisible) {
  m_pElement=pElement;
  m_iRemoveTime=iTime;
  m_bVisible=bVisible;

  printf("element \"%s\" marked for %s at %i\n",stringc(pElement->getToolTipText()).c_str(),bVisible?"showing":"hiding",iTime);
}

CGame::CGame(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_pConfigCtrl=new CIrrCC(pDevice);
  m_cScreenSize=m_pDriver->getScreenSize();
  m_iNextState=0;
  m_pRecorder=NULL;

  CConfigFileManager::getSharedInstance()->addWriter(m_pConfigCtrl);
  CConfigFileManager::getSharedInstance()->addReader(m_pConfigCtrl);

  for (u32 i=0; i<2; i++) {
		m_aCtrls[i][C_FW]=m_pConfigCtrl->addItem(i,stringw(L"Foreward"      ),!i?KEY_KEY_W:KEY_NUMPAD8,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_BW]=m_pConfigCtrl->addItem(i,stringw(L"Backward"      ),!i?KEY_KEY_S:KEY_NUMPAD5,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_LF]=m_pConfigCtrl->addItem(i,stringw(L"Left"          ),!i?KEY_KEY_A:KEY_NUMPAD4,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_RG]=m_pConfigCtrl->addItem(i,stringw(L"Right"         ),!i?KEY_KEY_D:KEY_NUMPAD6,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_BR]=m_pConfigCtrl->addItem(i,stringw(L"Brake"         ),!i?KEY_SPACE:KEY_NUMPAD0,CIrrCC::eCtrlButton);
    m_aCtrls[i][C_CL]=m_pConfigCtrl->addItem(i,stringw(L"Cam Left"      ),!i?KEY_KEY_Q:KEY_NUMPAD7,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_CR]=m_pConfigCtrl->addItem(i,stringw(L"Cam Right"     ),!i?KEY_KEY_E:KEY_NUMPAD9,CIrrCC::eCtrlAxis);
    m_aCtrls[i][C_RS]=m_pConfigCtrl->addItem(i,stringw(L"Respawn"       ),!i?KEY_TAB  :KEY_NUMPAD1,CIrrCC::eCtrlButton);

    m_pConfigCtrl->createAxis(m_aCtrls[i][C_FW],m_aCtrls[i][C_BW]);
    m_pConfigCtrl->createAxis(m_aCtrls[i][C_LF],m_aCtrls[i][C_RG]);
    m_pConfigCtrl->createAxis(m_aCtrls[i][C_CL],m_aCtrls[i][C_CR]);
  }

  m_pConfigCtrl->setAllowFKeys(false);
  m_pConfigCtrl->setAllowMouse(false);
}

CGame::~CGame() {
  CConfigFileManager::getSharedInstance()->removeWriter(m_pConfigCtrl);
  CConfigFileManager::getSharedInstance()->removeReader(m_pConfigCtrl);

  delete m_pConfigCtrl;
}

void CGame::createTriangleSelectors(ISceneNode *pParent) {
  list<ISceneNode *> pChildren=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  if (!strcmp(pParent->getName(),"player1") || !strcmp(pParent->getName(),"player2") ||
      !strcmp(pParent->getName(),"player3") || !strcmp(pParent->getName(),"player4") ||
      !strcmp(pParent->getName(),"ghost")) {
    printf("no triangle selector for \"%s\"\n",pParent->getName());
    return;
  }

  for (it=pChildren.begin(); it!=pChildren.end(); it++) {
    ISceneNode *pChild=*it;
    if (strcmp(pChild->getName(),"ghost")) {
      IMesh *pMesh=NULL;
      CDynamicMeshBuffer cDynMeshBuffer(EVT_2TCOORDS,EIT_16BIT);
      if (pChild->getType()==ESNT_TERRAIN)
        (reinterpret_cast<ITerrainSceneNode *>(pChild))->getMeshBufferForLOD(cDynMeshBuffer,0);

      switch (pChild->getType()) {
        case ESNT_TERRAIN:
          pMesh=(reinterpret_cast<ITerrainSceneNode *>(pChild))->getMesh();
          break;

        //case ESNT_OCT_TREE:
        case ESNT_MESH :
          pMesh=(reinterpret_cast<IMeshSceneNode *>(pChild))->getMesh();
          break;

        case ESNT_ANIMATED_MESH:
          pMesh=(reinterpret_cast<IAnimatedMeshSceneNode *>(pChild))->getMesh();
          break;

        default:
          break;
      }

      if (pMesh) {
        ITriangleSelector *pSelector=m_pSmgr->createOctreeTriangleSelector(pMesh,pChild);
        if (pSelector) {
          pChild->setTriangleSelector(pSelector);
          m_pSelector->addTriangleSelector(pSelector);
          pSelector->drop();
        }
      }

      createTriangleSelectors(pChild);
    }
  }
}

void CGame::activate(IState *pPrevious) {
  printf("CGame::activate start\n");
  m_pRecorder=NULL;
  m_pGhostPlayer=NULL;
  m_bOdeInitialized=false;
  m_bInitError=false;
  m_iNextState=pPrevious->getNum()+1;
  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pDevice->getCursorControl()->setVisible(false);

  //m_pOptions->resetNetgame();

  m_pCams[0]=NULL;
  m_pCams[1]=NULL;

  m_pMirrorCams[0]=NULL;
  m_pMirrorCams[1]=NULL;

  m_bMirrorVisible=false;

  m_pDevice->setEventReceiver(this);
	m_iTime=0;

	for (s32 i=0; i<5; i++) {
		c8 sName[0xFF];
		sprintf(sName,"data/countdown_%i.png",i);
		ITexture *tex=m_pDriver->getTexture(sName);
		m_aCountdown.push_back(tex);
	}

	s32 cx=m_cScreenSize.Width/2,cy=m_cScreenSize.Height/2;
	m_pCountdown=m_pGuienv->addImage(rect<s32>(cx-44,cy-20,cx+44,cy+20));
	m_pCountdown->setVisible(false);
	m_pCountdown->setUseAlphaChannel(true);
	m_pCountdown->setToolTipText(L"Countdown");

  m_pLoadingBackground=m_pGuienv->addImage(rect<s32>(0,0,m_cScreenSize.Width,m_cScreenSize.Height));
  m_pLoadingBackground->setScaleImage(true);
  //m_pLoadingBackground->setImage(m_pDriver->getTexture("data/load_screen.png"));
  m_pLoadingBackground->setUseAlphaChannel(false);

  m_pPrgBar=new CProgressBar(m_pGuienv,rect<s32>(cx-150,cy-20,cx+150,cy+20));
  m_pPrgBar->addBorder(1);

	m_pBigFont=m_pGuienv->getFont("data/font.xml");
	m_pLoadingProgress=m_pGuienv->addStaticText(L"",rect<s32>(cx-150,cy-20,cx+150,cy+20),true,true,NULL,-1,true);
	m_pLoadingProgress->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  m_pLoadingProgress->setOverrideFont(m_pBigFont);
  m_pLoadingProgress->setBackgroundColor(SColor(128,16,16,16));
  m_pLoadingProgress->setOverrideColor(SColor(255,255,118,70));

	m_pGameOver=m_pGuienv->addImage(rect<s32>(cx-68,15,cx+68,40));
	m_pGameOver->setImage(m_pDriver->getTexture("data/gameover.png"));
	m_pGameOver->setUseAlphaChannel(true);
	m_pGameOver->setVisible(false);

	cy=m_cScreenSize.Height-75;
	m_pFastestLap=m_pGuienv->addStaticText(L"Hello World!",rect<s32>(cx-150,cy,cx+150,cy+25),true,false,0,-1,true);
	m_pFastestLap->setBackgroundColor(SColor(128,192,128,128));
	m_pFastestLap->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
	m_pFastestLap->setToolTipText(L"Fastest Lap");
	m_pFastestLap->setVisible(false);

  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();
  m_bQuitGame=false;
  m_aViewPorts.clear();
  m_aMirrors.clear();
  m_aMirrorBack.clear();
  m_pStateMachine->setDrawScene(false);
  m_pDevice->getCursorControl()->setPosition(position2di(320,200));
	m_pCollisionMngr=m_pSmgr->getSceneCollisionManager();

	m_pNetInfo=m_pGuienv->addTab(rect<s32>(position2di(m_cScreenSize.Width-205,m_cScreenSize.Height-90),dimension2di(200,85)));
	m_pNetInfo->setBackgroundColor(SColor(128,192,192,192));
	m_pNetInfo->setDrawBackground(true);
	m_pNetInfo->setVisible(false);

  m_pNetInfoPing=m_pGuienv->addStaticText(L"Ping",rect<s32>(5, 5,195,20),false,true,m_pNetInfo);
  m_pNetInfoSent=m_pGuienv->addStaticText(L"Sent",rect<s32>(5,20,195,35),false,true,m_pNetInfo);
  m_pNetInfoRecv=m_pGuienv->addStaticText(L"Recv",rect<s32>(5,35,195,50),false,true,m_pNetInfo);
  m_pNetInfoTrfi=m_pGuienv->addStaticText(L"Traffic",rect<s32>(5,50,195,65),false,true,m_pNetInfo);
  m_pNetInfoFps=m_pGuienv->addStaticText(L"Framerate",rect<s32>(5,65,195,80),false,true,m_pNetInfo);

  //m_pVideo=NULL;
	CIrrOdeKlangManager::getSharedInstance()->activate();
	m_pConfigCtrl->reset();
	m_pLogic->setTriangleSelector(m_pSelector);
	m_pLogic->setTimer(m_pTimer);
	m_pLogic->init();
	printf("CGame::activate end\n");
}

void CGame::deactivate(IState *pNext) {
  m_pSelector->removeAllTriangleSelectors();
  m_pSelector->drop();
  CIrrOdeKlangManager::getSharedInstance()->deactivate();
  m_pSmgr->clear();
  m_pGuienv->clear();
	m_pStateMachine->setDrawScene(true);
  m_pDriver->setViewPort(rect<s32>(0,0,m_cScreenSize.Width,m_cScreenSize.Height));
  if (m_bOdeInitialized) {
    m_pOdeManager->closeODE();
  }
	CIrrOdeKlangManager::getSharedInstance()->deactivate();

	m_pDevice->getCursorControl()->setVisible(true);
	m_aCountdown.clear();

	list<GUIElementToToggle *>::Iterator it;
	while (m_lElementsToToggle.getSize()>0) {
	  it=m_lElementsToToggle.begin();
	  GUIElementToToggle *tgl=(*it);
	  m_lElementsToToggle.erase(it);
	  delete tgl;
	}

	if (m_pRecorder) {
	  m_pRecorder->save();
	  delete m_pRecorder;
    m_pRecorder=NULL;
	}

	if (m_pGhostPlayer!=NULL) {
	  delete m_pGhostPlayer;
	  m_pGhostPlayer=NULL;
	}

  CRenderToTextureManager::getSharedInstance()->setActive(false);
}

u32 CGame::update() {
  if (m_bInitError) {
    m_pDriver->beginScene(true,true,SColor(128,64,64,64));
    m_pGuienv->drawAll();
    m_pDriver->endScene();
  }
  else {
    m_pLogic->update();

    list<GUIElementToToggle *>::Iterator it;
    for (it=m_lElementsToToggle.begin(); it!=m_lElementsToToggle.end(); it++) {
      if (m_pTimer->getTime()>(*it)->m_iRemoveTime) {
        printf("%s element \"%s\" (%i)\n",(*it)->m_bVisible?"Showing":"Hiding",stringc((*it)->m_pElement->getToolTipText()).c_str(),m_pTimer->getTime());
        IGUIElement *pElement=(*it)->m_pElement;
        pElement->setVisible((*it)->m_bVisible);
        GUIElementToToggle *pToRemove=*it;
        m_lElementsToToggle.erase(it);
        delete pToRemove;
        break;
      }
    }

    for (u32 i=0; i<m_aMirrors.size(); i++)
      if (m_pMirrorCams[i]!=NULL) {
        m_pMirrorCams[i]->setPosition(m_pCams[i]->getTarget());
        m_pMirrorCams[i]->setTarget(m_pCams[i]->getPosition());
      }

    m_pDriver->beginScene();

    for (u32 i=0; i<m_aViewPorts.size(); i++) {
      if (m_pCams[i]) {
        m_pSmgr->setActiveCamera(m_pCams[i]);
        m_pDriver->setViewPort(m_aViewPorts[i]);
        m_pSmgr->drawAll();
      }

      if (m_bMirrorVisible) {
        m_pDriver->setViewPort(rect<s32>(0,0,m_cScreenSize.Width,m_cScreenSize.Height));
        m_pDriver->draw2DRectangle(SColor(192,255,79,38),m_aMirrorBack[i]);
        m_pDriver->clearZBuffer();
        m_pDriver->setViewPort(m_aMirrors[i]);
        m_pSmgr->setActiveCamera(m_pMirrorCams[i]);
        m_pSmgr->drawAll();
      }
    }

    m_pDriver->setViewPort(rect<s32>(0,0,m_cScreenSize.Width,m_cScreenSize.Height));
    m_pGuienv->drawAll();

    m_pDriver->endScene();
  }

  wchar_t s[0xFF];
  swprintf(s,0xFF,L"Framerate: %u fps",m_pStateMachine->getFps());
  m_pNetInfoFps->setText(s);

  return m_bQuitGame?m_iNextState:0;
}

bool CGame::OnEvent(const SEvent &event) {
  bool bRet=false;

  bRet=m_pConfigCtrl->OnEvent(event);

  if (!bRet && event.EventType==EET_KEY_INPUT_EVENT) {
    if (!event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_ESCAPE:
          switch (m_pLogic->getGameState()) {
            case eGameStatePlay:
              m_pLogic->setGameState(eGameStateGameOver1);
              break;

            case eGameStateGameOver1:
              break;

            case eGameStateGameOver2:
              m_pLogic->setGameState(eGameStateLapList);
              break;

            case eGameStateLapList:
              m_pLogic->setGameState(eGameStateFinished);
              break;

            case eGameStateFinished:
              if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
              m_pLogic->quitState();
              m_bQuitGame=true;
              bRet=true;
              break;

            default:
              m_pLogic->setGameState(eGameStateGameOver1);
              break;
          }
          break;

        case KEY_RETURN:
          if (m_pLogic->getRaceInfo()->raceFinished()) {
            if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
            m_pLogic->quitState();
            m_bQuitGame=true;
            bRet=true;
          }
          break;

        case KEY_F2:
          if (m_pNetInfo->isVisible())
            m_pNetInfo->setVisible(false);
          else
            m_pNetInfo->setVisible(true);
          break;

        case KEY_F3:
          m_bMirrorVisible=!m_bMirrorVisible;
          break;

        default:
          break;
      }
    }
  }

  if (event.EventType==EET_GUI_EVENT) {
    if (event.GUIEvent.EventType==EGET_MESSAGEBOX_YES && event.GUIEvent.Caller->getID()==4723) {
      m_pLogic->saveGhost();
    }
  }

  return bRet;
}

void CGame::addControlGui(u32 iSet, IGUIElement *pParent) {
  m_pConfigCtrl->addGui(iSet,pParent,position2di(15,50));
}

CIrrCC *CGame::getCC() {
  return m_pConfigCtrl;
}

void CGame::setOptions(COptions *pOptions) {
  m_pOptions=pOptions;
}

bool CGame::loadLevel(const wchar_t *sLevel) {
  #ifdef _DEBUG
    printf("\nloading level: \"%s\"\n\n",stringc(sLevel).c_str());
  #endif
  m_sLevelName=stringc(sLevel);
  CRenderToTextureManager::getSharedInstance()->clearLists();
  CShadowManager::getSharedInstance()->clear();
  bool bRet=irr::ode::CIrrOdeManager::getSharedInstance()->loadScene(stringc(sLevel).c_str(),m_pSmgr);

  ISceneNode *pSkybox=m_pSmgr->getSceneNodeFromType(ESNT_SKY_BOX);
  if (pSkybox)
    for (u32 i=0; i<pSkybox->getMaterialCount(); i++)
      for (u32 j=0; j<MATERIAL_MAX_TEXTURES; j++) {
        pSkybox->getMaterial(i).TextureLayer[j].LODBias=-128;
        if (pSkybox->getMaterial(i).TextureLayer[j].Texture!=NULL && j!=0) {
          pSkybox->getMaterial(i).TextureLayer[j].Texture=pSkybox->getMaterial(i).TextureLayer[0].Texture;
        }
      }

	m_pSelector=m_pSmgr->createMetaTriangleSelector();
	createTriangleSelectors(m_pSmgr->getRootSceneNode());
	CShadowManager::getSharedInstance()->setProgressCallback(this);
  CShadowManager::getSharedInstance()->update();
  CShadowManager::getSharedInstance()->setProgressCallback(NULL);

  m_pPrgBar->setVisible(false);
  m_pLoadingProgress->setVisible(false);
  m_pLoadingBackground->setVisible(false);

  CRenderToTextureManager::getSharedInstance()->setActive(true);

  return bRet;
}

void CGame::setNoViewports(u32 iNum, bool bSplitHorizontal) {
  if (iNum==1) {
    m_aViewPorts.push_back(rect<s32>(0,0,m_cScreenSize.Width,m_cScreenSize.Height));
    position2di cMirrorPos=position2di((3*m_cScreenSize.Width/4)-5,5);
    dimension2di cMirrorDim=dimension2di(m_cScreenSize.Width/4,m_cScreenSize.Height/4);
    m_aMirrors.push_back(rect<s32>(cMirrorPos,cMirrorDim));
    m_aMirrorBack.push_back(rect<s32>(cMirrorPos-position2di(2,2),cMirrorDim+dimension2di(4,4)));
  }
  else {
    for (u32 i=0; i<2; i++) {
      s32 x1,y1,x2,y2,mx,my,mdx,mdy;

      if (bSplitHorizontal) {
        x1=i==0?0:m_cScreenSize.Width/2+_BORDER;
        x2=i==0?m_cScreenSize.Width/2-_BORDER:m_cScreenSize.Width;
        y1=0;
        y2=m_cScreenSize.Height;

        mx=m_cScreenSize.Width/2;
        if (i==0) mx-=(m_cScreenSize.Width/8)+_BORDER+5; else mx+=_BORDER+5;

        my=5;
        mdx=m_cScreenSize.Width/8;
        mdy=m_cScreenSize.Height/8;
      }
      else {
        x1=0;
        x2=m_cScreenSize.Width;
        y1=i==0?0:m_cScreenSize.Height/2+_BORDER;
        y2=i==0?m_cScreenSize.Height/2-_BORDER:m_cScreenSize.Height;
        mx=m_cScreenSize.Width-5-(m_cScreenSize.Width/8);
        my=i==0?5:m_cScreenSize.Height/2+_BORDER+5;
        mdx=m_cScreenSize.Width/8;
        mdy=m_cScreenSize.Width/8;
      }

      m_aViewPorts.push_back(rect<s32>(x1,y1,x2,y2));
      m_aMirrors.push_back(rect<s32>(position2di(mx,my),dimension2di(mdx,mdy)));
      m_aMirrorBack.push_back(rect<s32>(position2di(mx-2,my-2),dimension2di(mdx+4,mdy+4)));
    }
  }
}

void CGame::setGameState(u32 iState) {
  m_iState=iState;
}

void CGame::initODE() {
	m_pOdeManager=CIrrOdeManager::getSharedInstance();
	m_pOdeManager->initODE();
	m_pOdeManager->initPhysics();

	m_bOdeInitialized=true;
}

void CGame::setCam(u32 iNum, ICameraSceneNode *pCam) {
  if (iNum<2) {
    m_pCams[iNum]=pCam;
    m_pMirrorCams[iNum]=m_pSmgr->addCameraSceneNode();
  }
}

IMetaTriangleSelector *CGame::getTriangleSelector() {
  return m_pSelector;
}

ISoundEngine *CGame::getSoundEngine() {
  return m_pSndEngine;
}

f32 CGame::getControl(u32 iSet, u32 iControl) {
  return m_pConfigCtrl->get(m_aCtrls[iSet-1][iControl]);
}

IGameLogic *CGame::getGameLogic() {
  return m_pLogic;
}

void CGame::toggleCountdown(u32 iState) {
  if (iState<m_aCountdown.size()) {
    if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/countdown.ogg");
    m_pCountdown->setImage(m_aCountdown[iState]);
    m_pCountdown->setVisible(true);

    if (iState==m_aCountdown.size()-1) {
      addElementToToggle(m_pCountdown,m_pTimer->getTime()+3000,false);
    }
  }
  else m_pCountdown->setVisible(false);
}

void CGame::showGameOver() {
  m_pGameOver->setVisible(true);
}

void CGame::showFastestLap(const wchar_t *sText) {
  m_pFastestLap->setText(sText);
  m_pFastestLap->setVisible(true);
  addElementToToggle(m_pFastestLap,m_pTimer->getTime()+5000,false);
}

void CGame::showRaceInfo(array<CRaceTime *> pInfo, CLapTime *pFastestLap, u32 iPlayers, const wchar_t **sNames) {
  wchar_t s[0xFF];
  u32 iHiScoreTime[4];
  m_pGameOver->setVisible(false);

  CHiScoreList *pList=new CHiScoreList(m_pDevice,stringc(stringw(m_pLogic->getHiScoreFile())).c_str());
  printf("info of race with %i players\n",iPlayers);

  dimension2di dim=dimension2di(180*iPlayers,550);
  position2di pos  =position2di(m_cScreenSize.Width/2-dim.Width/2,m_cScreenSize.Height/2-dim.Height/2),
              start=position2di(5,5);

  IGUITab *pTab=m_pGuienv->addTab(rect<s32>(pos,dim));
  pTab->setDrawBackground(true);
  pTab->setBackgroundColor(SColor(192,96,96,96));

  for (u32 i=0; i<iPlayers; i++)
    if (pInfo[i]->getFastestLap()!=NULL) {
      iHiScoreTime[i]=pInfo[i]->getFastestLap()->getTime();
      pList->insert(iHiScoreTime[i],sNames[i]);
    }
    else iHiScoreTime[i]=0;

  for (u32 i=0; i<iPlayers; i++) {
    pos=start+position2di(180*i,0);
    dim=dimension2di(170,45);

    IGUIStaticText *plr=m_pGuienv->addStaticText(sNames[i],rect<s32>(pos,dim),false,true,pTab);
    plr->setOverrideFont(m_pBigFont);
    plr->setBackgroundColor(SColor(128,16,16,16));
    plr->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    plr->setDrawBackground(true);
    plr->setOverrideColor(SColor(255,255,118,70));

    pos+=position2di(0,55);
    dim=dimension2di(170,18);

    CRaceTime *pRace=pInfo[i];
    for (u32 j=0; j<pRace->getCurrentLapNo(); j++) {
      CLapTime *pLap=pRace->getLap(j);
      if (pLap && pLap->getTime()) {
        printf("\tLap %i: %.2f",j,((f32)pLap->getTime())/1000);
        swprintf(s,0xFF,L"Lap %i: %.2f",j+1,((f32)pLap->getTime())/1000);
        IGUIStaticText *lap=m_pGuienv->addStaticText(s,rect<s32>(pos,dim),false,true,pTab);
        lap->setBackgroundColor(SColor(128,16,16,16));
        lap->setDrawBackground(true);
        lap->setOverrideColor(SColor(255,255,118,70));

        pos.Y+=20;
        if (pLap==pRace->getFastestLap()) {
          printf(" *");
          lap->setBackgroundColor(SColor(128,96,96,192));
        }

        if (pLap==pFastestLap) {
          printf("#");
          lap->setBackgroundColor(SColor(128,192,96,96));
        }
        printf("\n");
      }
    }

    u32 iPos=pList->getPositionOf(iHiScoreTime[i],sNames[i]);
    printf("position in HiScore table: %i\n",iPos);
    printf("pos: %i, %i\n",pos.X,pos.Y);
    wchar_t s[0xFF];
    if (iPos>0)
      swprintf(s,0xFF,L"HiScore Position: %u",iPos);
    else
      swprintf(s,0xFF,L"Not in HiScore List");

    IGUIStaticText *pHiScore=m_pGuienv->addStaticText(s,rect<s32>(pos,dim),false,true,pTab,-1,true);
    pHiScore->setBackgroundColor(SColor(128,128,192,128));

    //Show the racetime or D.N.F. if the race was not finished
    if (pInfo[i]->getTime()!=0)
      swprintf(s,0xFF,L"Racetime: %.2f",((f32)pInfo[i]->getTime())/1000);
    else
      swprintf(s,0xFF,L"Did not finish");

    IGUIStaticText *pRaceTime=m_pGuienv->addStaticText(s,rect<s32>(pos+position2di(0,25),dim),false,true,pTab);
    pRaceTime->setBackgroundColor(SColor(128,16,16,16));
    pRaceTime->setDrawBackground(true);
    pRaceTime->setOverrideColor(SColor(255,255,118,70));

    pos.Y+=50;

    u32 iFinishPos=pInfo[i]->getFinishedPos();
    swprintf(s,0xFF,L"%s",iFinishPos==0?L"1st":iFinishPos==1?L"2nd":iFinishPos==2?L"3rd":iFinishPos==3?L"4th":L"d.n.f.");
    IGUIStaticText *pPos=m_pGuienv->addStaticText(s,rect<s32>(pos,dimension2di(170,40)),false,true,pTab);
    pPos->setOverrideFont(m_pBigFont);
    pPos->setBackgroundColor(SColor(128,16,16,16));
    pPos->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    pPos->setDrawBackground(true);
    pPos->setOverrideColor(SColor(255,255,118,70));

    printf("\t\tpos: %i, %i\n",pos.X,pos.Y);

    pos.X+=180;
    pos.Y=0;
  }

  pList->save();
}

void CGame::addElementToToggle(IGUIElement *pElement, u32 iTime, bool bVisible) {
  list<GUIElementToToggle *>::Iterator it;
  for (it=m_lElementsToToggle.begin(); it!=m_lElementsToToggle.end(); it++)
    if ((*it)->m_pElement==pElement) {
      (*it)->m_iRemoveTime=iTime;
      return;
    }

  GUIElementToToggle *pRemove=new GUIElementToToggle(pElement,iTime,bVisible);
  m_lElementsToToggle.push_back(pRemove);
}

void CGame::setGameLogic(IGameLogic *pLogic) {
  printf("CGame::setGameLogic\n");
  m_pLogic=pLogic;
}

void CGame::setInitError(const wchar_t *sText) {
  m_bInitError=sText!=NULL;
  if (m_bInitError) {
    wcscpy(m_sInitErrorText,sText);
    rect<s32> theRect=rect<s32>(m_cScreenSize.Width/2-200,m_cScreenSize.Height/2-50,m_cScreenSize.Width/2+200,m_cScreenSize.Height/2+50);
    IGUIStaticText *pError=m_pGuienv->addStaticText(sText,theRect,true,true,NULL,-1,true);
    pError->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pLogic->setGameState(eGameStateFinished);
  }
}

void CGame::initReplayRecord() {
  c8 sName[0xFF];
  stringc sFile=stringc(m_sLevelName.c_str()),sCwd;
  time_t cTime=time(NULL);
  strftime(sName,0xFF,"data/replay/%Y-%m-%d-%H-%M-%S.replay",localtime(&cTime));

  IFileSystem *pFs=m_pDevice->getFileSystem();
  sCwd=pFs->getWorkingDirectory();
  if (sFile.subString(0,sCwd.size())==sCwd)
    sFile=sFile.subString(sCwd.size()+1,sFile.size());

  m_pRecorder=new CMarbles2Recorder(sName,sFile.c_str(),"marbles2",m_pDevice);
}

CMarbles2Recorder *CGame::getRecorder() {
  return m_pRecorder;
}

void CGame::progress(u32 iDone, u32 iTotal) {
  printf("shadow progress: %u of %u done (%.2f percent)\n",iDone,iTotal,100.0f*((f32)iDone)/((f32)iTotal));
  wchar_t s[0xFF];
  swprintf(s,0xFF,L"Initializing: %.2f %%",100.0f*((f32)iDone)/((f32)iTotal));
  m_pLoadingProgress->setText(s);
  m_pPrgBar->setProgress((u32)(100.0f*((f32)iDone)/((f32)iTotal)));
  m_pDriver->beginScene(true,true,SColor(128,64,64,64));
  m_pGuienv->drawAll();
  m_pDriver->endScene();
  m_pLogic->update();
}

void CGame::setQuitGame(bool b) {
  m_bQuitGame=b;
}

void CGame::setWaitingImageVisibility(bool b) {
  m_pLoadingBackground->setVisible(b);
  if (!b) m_pLoadingBackground->remove();
}

void CGame::setNetInfoText(const wchar_t *sPing, const wchar_t *sSent, const wchar_t *sRecv, const wchar_t *sTraffic) {
  m_pNetInfoPing->setText(sPing);
  m_pNetInfoSent->setText(sSent);
  m_pNetInfoRecv->setText(sRecv);
  m_pNetInfoTrfi->setText(sTraffic);
}

void CGame::restartGhost() {
  if (m_pGhostPlayer) {
    printf("starting ghost player!\n");
    m_pGhostPlayer->startGhost(true);
  }
}

void CGame::stopGhost() {
  if (m_pGhostPlayer) m_pGhostPlayer->stopGhost(false);
}

void CGame::setGhostFile(const c8 *sFile) {
  if (sFile[0]=='\0') return;
  m_pGhostPlayer=new CGhostPlayer(m_pDevice,m_pSndEngine);
  m_pGhostPlayer->loadGhostFile(sFile);
}
