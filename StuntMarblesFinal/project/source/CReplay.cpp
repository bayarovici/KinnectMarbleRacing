  #include <CReplay.h>
  #include <CReplaySelect.h>
  #include <CStateMachine.h>
  #include <CMarbles2Replayer.h>
  #include <CIrrOdeKlangManager.h>
  #include <CReplayCam.h>
  #include <irrOde.h>

  #include <GameClasses/CMarbleFollowAnimator.h>

using namespace ode;

enum eCamMode {
  eCamModeFocus,
  eCamModeFree,
  eCamModeFollow,
  eCamModeBlimp
};

CReplay::CReplay(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_pRePlayer=new CMarbles2Replayer(pDevice);
  m_pSelect=NULL;
  m_pCollisionMngr=NULL;
}

CReplay::~CReplay() {
  delete m_pRePlayer;
}

void CReplay::updateCamInfo() {
  if (m_lCams.getSize()==0) {
    if (m_iCamMode==eCamModeFocus) m_iCamMode=eCamModeFree;
  }

  wchar_t s[0xFF];

  switch(m_iCamMode) {
    case eCamModeFocus:
      m_pSmgr->setActiveCamera(m_pCam);
      m_pActiveCam=m_pCam;
      CIrrOdeKlangManager::getSharedInstance()->setListener((ICameraSceneNode *)m_pActiveCam,0);
      swprintf(s,0xFF,L"Player %i",m_iFocused+1);
      break;

    case eCamModeFree:
      m_pSmgr->setActiveCamera(m_pFreeCam);
      m_pActiveCam=m_pFreeCam;
      CIrrOdeKlangManager::getSharedInstance()->setListener((ICameraSceneNode *)m_pActiveCam,0);
      wcscpy(s,L"Free Cam");
      break;

    case eCamModeFollow:
      m_pSmgr->setActiveCamera(m_aFollowCams[m_iFocused]);
      m_pActiveCam=m_aFollowCams[m_iFocused];
      CIrrOdeKlangManager::getSharedInstance()->setListener((ICameraSceneNode *)m_pActiveCam,0);
      wcscpy(s,L"Follow Cam");
      break;

    case eCamModeBlimp:
      m_pSmgr->setActiveCamera(m_aBlimpCams[m_iFocused]);
      m_pActiveCam=m_aBlimpCams[m_iFocused];
      CIrrOdeKlangManager::getSharedInstance()->setListener((ICameraSceneNode *)m_pActiveCam,0);
      wcscpy(s,L"Blimp Cam");
      break;
  }

  m_pCamInfo->setText(s);
}

void CReplay::findCam(ISceneNode *pParent) {
  list<ISceneNode *> pChildren=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  for (it=pChildren.begin(); it!=pChildren.end(); it++) {
    if ((*it)->getType()==REPLAY_CAM_NODE_ID) {
      m_lCams.push_back((CReplayCam *)(*it));
    }
    findCam(*it);
  }
}

void CReplay::activate(IState *pPrevious) {
  printf("\n\nCReplay::activate\n\n\n");
  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_lCams.clear();
  m_iMoveX=0;
  m_iMoveY=0;
  m_iMoveZ=0;
  m_iRet=0;
  m_iAngleV=0;
  m_iAngleH=0;
  m_iCamMode=eCamModeFocus;
  if (m_pSelect==NULL)
    m_iRet=9;
  else {
    dimension2du cScreenSize=m_pDevice->getVideoDriver()->getScreenSize();

    m_pLoadingBackground=m_pGuienv->addImage(rect<s32>(0,0,cScreenSize.Width,cScreenSize.Height));
    m_pLoadingBackground->setScaleImage(true);
    m_pLoadingBackground->setImage(m_pDriver->getTexture("data/load_screen.png"));
    m_pLoadingBackground->setUseAlphaChannel(false);

    s32 cx=cScreenSize.Width/2,cy=cScreenSize.Height/2;
    m_pLoadingProgress=m_pGuienv->addStaticText(L"",rect<s32>(cx-150,cy-20,cx+150,cy+20),true,true,NULL,-1,true);
    m_pLoadingProgress->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pLoadingProgress->setOverrideFont(m_pGuienv->getFont("data/font2.xml"));
    m_pLoadingProgress->setBackgroundColor(SColor(128,16,16,16));
    m_pLoadingProgress->setOverrideColor(SColor(255,255,118,70));
    m_pRePlayer->setLoadingProgressElement(m_pLoadingProgress);

    m_pPaused=m_pGuienv->addStaticText(L"Paused",rect<s32>(cx-150,2*cy-50,cx+150,2*cy-10),true,true,NULL,-1,true);
    m_pPaused->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pPaused->setOverrideFont(m_pGuienv->getFont("data/font2.xml"));
    m_pPaused->setBackgroundColor(SColor(128,16,16,16));
    m_pPaused->setOverrideColor(SColor(255,255,118,70));
    m_pPaused->setVisible(false);

    u32 dimx=m_pDevice->getVideoDriver()->getScreenSize().Width;

    m_pCamInfo=m_pGuienv->addStaticText(L"Player 1",rect<s32>(dimx/2-100,35,dimx/2+100,55),true,true,NULL,-1,true);
    m_pCamInfo->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pCamInfo->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

    printf("loading replay \"%s\" ... \n",stringc(m_pSelect->getReplayName()).c_str());
    m_pRePlayer->loadReplay(stringc(m_pSelect->getReplayName()).c_str());
    if (m_pRePlayer->isValidReplay()) {
      m_pRePlayer->createGUI();
      m_pRePlayer->init();

      findCam(m_pSmgr->getRootSceneNode());

      m_pSelector=m_pSmgr->createMetaTriangleSelector();
      createTriangleSelectors(m_pSmgr->getRootSceneNode());

      ISceneNode *plr=NULL;
      u32 i=1;
      do {
        c8 s[0xFF];
        sprintf(s,"player%i",i++);
        plr=m_pSmgr->getSceneNodeFromName(s);
        if (plr) {
          printf("%s found\n",s);
          m_aPlayers.push_back(plr);

          ICameraSceneNode *pCam=(ICameraSceneNode *)findCamera(plr);

          m_pCollisionMngr=m_pDevice->getSceneManager()->getSceneCollisionManager();

          if (pCam) {
            pCam->setTarget(plr->getAbsolutePosition());

            CMarbleFollowCameraAnimator *pAnimator=new CMarbleFollowCameraAnimator(pCam,m_pSmgr);
            pAnimator->setSoundEngine(NULL);
            pAnimator->setCollisionManager(m_pDevice->getSceneManager()->getSceneCollisionManager());
            pAnimator->setBody((CIrrOdeBody *)plr);
            pAnimator->setSelector(m_pSelector);
            pCam->addAnimator(pAnimator);

            m_aAnimators.push_back(pAnimator);
            m_aFollowCams.push_back(pCam);
          }
          else printf("no camera found for player %i\n",i);

          ICameraSceneNode *pBlimp=m_pSmgr->addCameraSceneNode();
          pBlimp->setUpVector(vector3df(0.0f,0.0f,1.0f));
          m_aBlimpCams.push_back(pBlimp);
          m_aBlimpDist.push_back(0.0f);
        }
      }
      while (plr!=NULL);

      m_pFreeCam=m_pSmgr->addCameraSceneNodeFPS(NULL,50.0f,0.1f);

      if (m_lCams.getSize()>0) {
        m_pCam=m_pSmgr->addCameraSceneNode();
        m_pActiveCam=*(m_lCams.begin());
        m_pCam->setPosition(m_pActiveCam->getPosition());
        m_pSmgr->setActiveCamera(m_pCam);
      }
      else {
        m_pActiveCam=NULL;
        m_pSmgr->setActiveCamera(m_pFreeCam);
        m_iCamMode=eCamModeFree;
        m_pCam=m_pFreeCam;
      }

      CIrrOdeKlangManager::getSharedInstance()->setListener(m_pCam,0);
      m_pDevice->getCursorControl()->setVisible(false);
    }
    else {
      printf("invalid level!\n");
    }
    m_pDevice->setEventReceiver(this);
  }

  CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(m_pRePlayer);

  m_pLoadingBackground->setVisible(false);
  m_pLoadingProgress->setVisible(false);
  m_iFocused=0;
  m_pTimer=m_pDevice->getTimer();
  m_iLastTime=m_pTimer->getTime();
  m_iThisTime=m_iLastTime;
}

void CReplay::deactivate(IState *pNext) {
  //OK, here we just have to delete our arrays of references. The referenced objects
  //will be deleted by the scenemanager in "m_pSmgr->clear()"!
  m_aAnimators.clear();
  m_aFollowCams.clear();
  m_aBlimpCams.clear();
  m_aBlimpDist.clear();

  CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(m_pRePlayer);

  m_pSelector->removeAllTriangleSelectors();
  m_pSelector->drop();
  CIrrOdeKlangManager::getSharedInstance()->deactivate();
  m_pSmgr->clear();
  m_pGuienv->clear();
  m_lCams.clear();
  m_aPlayers.clear();
  CIrrOdeManager::getSharedInstance()->closeODE();
  m_pRePlayer->setLoadingProgressElement(NULL);
}

u32 CReplay::update() {
  if (!m_pRePlayer->isStarted()) m_pRePlayer->start();
  if (m_pRePlayer->update())
    for (u32 i=0; i<m_aAnimators.size(); i++) {
      m_aAnimators[i]->camStep();
    }

  for (u32 i=0; i<m_aBlimpCams.size(); i++) {
    f32 v=20.0f+((CIrrOdeBody *)m_aPlayers[i])->getLinearVelocity().getLength();

    if (m_aBlimpDist[i]!=0){
      f32 diff=v-m_aBlimpDist[i];
      v=m_aBlimpDist[i]+(diff/10.0f);
    }
    m_aBlimpDist[i]=v;

    vector3df vDist=vector3df(0.0f,v,0.0f);

    m_aBlimpCams[i]->setPosition(m_aPlayers[i]->getPosition()+vDist);
    m_aBlimpCams[i]->setTarget(m_aPlayers[i]->getPosition());
    if (v>0.1f) {
      vector3df vUp=((CIrrOdeBody *)m_aPlayers[i])->getLinearVelocity().normalize(),
                vOldUp=m_aBlimpCams[i]->getUpVector(),
                vNewUp=(vUp-vOldUp)/150.0f;

      m_aBlimpCams[i]->setUpVector((vOldUp+vNewUp).normalize());
    }
  }

  m_iLastTime=m_iThisTime;
  m_iThisTime=m_pTimer->getTime();

  if (m_lCams.getSize()>0 && m_aPlayers.size() && m_iCamMode!=eCamModeFree) {
    ISceneNode *pFocus=m_aPlayers[m_iFocused];
    f32 fNearDist=1000000;

    list<CReplayCam *>::Iterator it;
    for (it=m_lCams.begin(); it!=m_lCams.end(); it++) {
      vector3df dist=(*it)->getPosition()-pFocus->getPosition();
      if (dist.getLength()*(*it)->getFactor()<fNearDist) {
        m_pActiveCam=*it;
        fNearDist=dist.getLength()*(*it)->getFactor();
      }
    }

    m_pCam->setPosition(m_pActiveCam->getPosition());
    m_pCam->setTarget(pFocus->getPosition());

    m_pFreeCam->setPosition(m_iCamMode==eCamModeFocus?m_pActiveCam->getPosition():m_aFollowCams[m_iFocused]->getPosition());
    m_pFreeCam->setTarget(m_iCamMode==eCamModeFocus?pFocus->getPosition():m_aFollowCams[m_iFocused]->getTarget());
  }

  return m_iRet;
}

bool CReplay::OnEvent(const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_KEY_INPUT_EVENT) {
    if (!event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_ESCAPE:
          m_iRet=9;
          bRet=true;
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuback.ogg");
          break;

         case KEY_KEY_1: if (m_aPlayers.size()>=1) { m_iFocused=0; updateCamInfo(); } break;
         case KEY_KEY_2: if (m_aPlayers.size()>=2) { m_iFocused=1; updateCamInfo(); } break;
         case KEY_KEY_3: if (m_aPlayers.size()>=3) { m_iFocused=2; updateCamInfo(); } break;
         case KEY_KEY_4: if (m_aPlayers.size()>=4) { m_iFocused=3; updateCamInfo(); } break;

         case KEY_UP   : m_iMoveZ=0; break;
         case KEY_DOWN : m_iMoveZ=0; break;
         case KEY_LEFT : m_iMoveX=0; break;
         case KEY_RIGHT: m_iMoveX=0; break;

         case KEY_KEY_F: m_iCamMode=eCamModeFree; updateCamInfo();break;
         case KEY_KEY_M: m_iCamMode=eCamModeFollow; updateCamInfo(); break;
         case KEY_KEY_B: m_iCamMode=eCamModeBlimp; updateCamInfo(); break;
         case KEY_KEY_S: m_iCamMode=eCamModeFocus; updateCamInfo(); break;

         case KEY_TAB:
           switch (m_iCamMode) {
             case eCamModeFocus:
               m_iCamMode=eCamModeFree;
               updateCamInfo();
               break;

             case eCamModeFree:
               m_iCamMode=eCamModeFollow;
               updateCamInfo();
               break;

             case eCamModeFollow:
               m_iCamMode=eCamModeFocus;
               updateCamInfo();
               break;
           }
           break;

          case KEY_SPACE:
            m_pRePlayer->setIsPaused(!m_pRePlayer->isPaused());
            m_pPaused->setVisible(m_pRePlayer->isPaused());
            break;

        default:
          break;
      }
    }

    if (event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
         case KEY_UP   : m_iMoveZ= 1; break;
         case KEY_DOWN : m_iMoveZ=-1; break;
         case KEY_LEFT : m_iMoveX=-1; break;
         case KEY_RIGHT: m_iMoveX= 1; break;
         default: break;
      }
    }
  }

  if (event.EventType==EET_MOUSE_INPUT_EVENT ) {
    m_iMoveY+=(s32)(2.0f*event.MouseInput.Wheel);
  }

  return bRet;
}

void CReplay::setReplaySelect(CReplaySelect *pSelect) {
  m_pSelect=pSelect;
}

void CReplay::createTriangleSelectors(ISceneNode *pParent) {
  list<ISceneNode *> pChildren=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  if (!strcmp(pParent->getName(),"player1") || !strcmp(pParent->getName(),"player2") ||
      !strcmp(pParent->getName(),"player3") || !strcmp(pParent->getName(),"player4")) {
    printf("no triangle selector for \"%s\"\n",pParent->getName());
    return;
  }

  for (it=pChildren.begin(); it!=pChildren.end(); it++) {
    ISceneNode *pChild=*it;

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

ISceneNode *CReplay::findCamera(ISceneNode *pParent) {
  if (pParent->getType()==ESNT_CAMERA) return pParent;

  list<ISceneNode *> childList=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  for (it=childList.begin(); it!=childList.end(); it++) {
    ISceneNode *pRet=findCamera(*it);
    if (pRet) return pRet;
  }

  return NULL;
}
