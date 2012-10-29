  #include <GameClasses/CPlayer.h>
  #include <CGame.h>

  #include <CIrrOdeKlangManager.h>
  #include <CIrrOdeKlangNode.h>
  #include <GameClasses/CMarbleFollowAnimator.h>
  #include <GameClasses/IGameLogic.h>
  #include <CRenderToTextureManager.h>
  #include <IIrrOdeController.h>

  #include <IrrODE.h>

CPlayer::CPlayer(u32 iNum, u32 iLocalNum, IrrlichtDevice *pDevice, f32 fCamAspectRatio, CGame *pGame, f32 fPan, bool bNetClient) {
  init(pDevice,pGame);
  m_iNum=iNum;
  m_fPan=fPan;
  m_bNetClient=bNetClient;

  c8 sNodeName[0xFF];
  sprintf(sNodeName,"player%i",iNum);

  m_pCamMode =NULL;
  m_pLap     =NULL;
  m_pFinished=NULL;
  m_pSpeed   =NULL;
  m_pInfo    =NULL;
  m_pCpTime  =NULL;
  m_pName    =NULL;
  m_pCpInfo  =NULL;
  m_pLapTab  =NULL;

  m_bCamStepped=false;

  ISceneNode *pNode=pDevice->getSceneManager()->getSceneNodeFromName(sNodeName);

  if (pNode==NULL) {
    m_bInitOK=false;
    printf("Player %i node (%s) not found!\n",iNum,sNodeName);
  }
  else {
    m_bInitOK=true;
    m_pMarble=(CIrrOdeBody *)pNode;
    ISceneNode *pCam=findCamera(pNode);
    if (pCam==NULL)
      printf("No Camera scene node found for player %i\n",iNum);
    else {
      m_pCam=(ICameraSceneNode *)pCam;
      m_pCam->setAspectRatio(fCamAspectRatio);
      m_pCam->setTarget(m_pMarble->getAbsolutePosition());

      m_pAnimator=new CMarbleFollowCameraAnimator(m_pCam,m_pDevice->getSceneManager());
      m_pAnimator->setBody(m_pMarble);
      m_pAnimator->setSelector(pGame->getTriangleSelector());
      m_pAnimator->setCollisionManager(m_pDevice->getSceneManager()->getSceneCollisionManager());
      m_pAnimator->setSoundEngine(m_pSndEngine);
      m_pCam->addAnimator(m_pAnimator);
      printf("\n\t\tiLocalNum: %i\n\n",iLocalNum);
      CIrrOdeKlangManager::getSharedInstance()->setListener(m_pCam,iLocalNum-1);
    }
    CRenderToTextureManager::getSharedInstance()->addTarget(pNode);
  }

	for (u32 i=0; i<10; i++) {
		c8 sName[0xFF];
		sprintf(sName,"data/numbers/%i.png",i);
		ITexture *tex=m_pDevice->getVideoDriver()->getTexture(sName);
		m_aNumbers.push_back(tex);
	}

	ITexture *tex=m_pDevice->getVideoDriver()->getTexture("data/numbers/slash.png"); m_aNumbers.push_back(tex);
	          tex=m_pDevice->getVideoDriver()->getTexture("data/numbers/space.png"); m_aNumbers.push_back(tex);
}

CPlayer::CPlayer(ISceneNode *pNode, IrrlichtDevice *pDevice, f32 fCamAspectRatio, CGame *pGame) {
  init(pDevice,pGame);
  m_bInitOK=pNode!=NULL;
  m_pMarble=(CIrrOdeBody *)pNode;

  ISceneNode *pCam=findCamera(pNode);
  if (pCam==NULL)
    printf("No Camera scene node found\n");
  else
    if (m_bInitOK) {
      m_pCam=(ICameraSceneNode *)pCam;
      m_pCam->setAspectRatio(fCamAspectRatio);
      m_pCam->setTarget(m_pMarble->getAbsolutePosition());

      m_pAnimator=new CMarbleFollowCameraAnimator(m_pCam,pDevice->getSceneManager());
      m_pAnimator->setBody(m_pMarble);
      m_pAnimator->setSelector(pGame->getTriangleSelector());
      m_pAnimator->setCollisionManager(m_pDevice->getSceneManager()->getSceneCollisionManager());
      m_pAnimator->setSoundEngine(m_pSndEngine);
      //m_pCam->addAnimator(m_pAnimator);
      CIrrOdeKlangManager::getSharedInstance()->setListener(m_pCam,0);
    }
    else printf("marble node is NULL\n");

  CRenderToTextureManager::getSharedInstance()->addTarget(pNode);
}

void CPlayer::init(IrrlichtDevice *pDevice, CGame *pGame) {
  m_pDevice=pDevice;
  m_iNum=1;
  m_pMarble=NULL;
  m_bActive=false;
  m_bFinished=false;
  m_pGame=pGame;
  m_pCamFree=NULL;
  m_pCamRace=NULL;
  m_iRespawnTime=0;

  m_pConfigCtrl=pGame->getCC();
  m_pTimer=NULL;
  m_pController=NULL;

  m_pBigFont=m_pDevice->getGUIEnvironment()->getFont("data/font2.xml");
  m_fPan=0.0f;
  m_pSndEngine=pGame->getSoundEngine();
}

CPlayer::~CPlayer() {
  delete m_pAnimator;
  m_aCpState.clear();
  m_aLapNo.clear();
}

ISceneNode *CPlayer::findCamera(ISceneNode *pParent) {
  if (pParent->getType()==ESNT_CAMERA) {
    return pParent;
  }

  list<ISceneNode *> childList=pParent->getChildren();
  list<ISceneNode *>::Iterator it;

  ISceneNode *pRet=NULL;
  for (it=childList.begin(); it!=childList.end(); it++) {
    pRet=findCamera(*it);
    if (pRet) break;
  }
  return pRet;
}

ICameraSceneNode *CPlayer::getCamera() {
  return m_pCam;
}

void CPlayer::step(bool bDoCamStep) {
  f32 f;

  if (!m_bFinished) {
    f=m_pGame->getControl(m_bNetClient?1:m_iNum,C_CL); if (f) m_pAnimator->camLeft (2*f);
    f=m_pGame->getControl(m_bNetClient?1:m_iNum,C_CR); if (f) m_pAnimator->camRight(2*f);

    bool bRaceCamOld=m_pAnimator->isRaceCam();
    if (bDoCamStep) m_pAnimator->camStep();
    m_bCamStepped=false;
    bool bRaceCamNew=m_pAnimator->isRaceCam();

    if (bRaceCamOld!=bRaceCamNew) {
      if (bRaceCamNew) {
        if (m_pCamRace && m_pCamMode) m_pCamMode->setImage(m_pCamRace);
      }
      else {
        if (m_pCamFree && m_pCamMode) m_pCamMode->setImage(m_pCamFree);
      }
    }
  }

  if (m_bActive) {
    f32 acc=250*(m_pGame->getControl(m_bNetClient?1:m_iNum,C_FW)),
        str=250*(m_pGame->getControl(m_bNetClient?1:m_iNum,C_LF));

    if (acc>=25.0f || acc<=-25.0f || str>=25.0f || str<=-25.0f) {
      f32 a=m_pAnimator->getAngleH();
      vector3df push=vector3df(0.0f,a,0.0f).rotationToDirection(-vector3df(acc,0.0f,str));
      m_pController->addTorque(m_pMarble,push);
    }

    f=0.005f;
    f32 f2=m_pGame->getControl(m_bNetClient?1:m_iNum,C_BR);
    f32 f3=f+(0.6f*(f2-f));
    if (f3!=m_fDamping) {
      m_pController->setAngularDamping(m_pMarble,f3);
      m_fDamping=f3;
    }

    f32 fRespawn=m_pGame->getControl(m_bNetClient?1:m_iNum,C_RS);
    if (fRespawn)
      if (m_iRespawnTime==0)
        m_iRespawnTime=120;
      else {
        m_iRespawnTime--;
        if (!m_iRespawnTime) {
          ISound *snd=m_pSndEngine->play2D("data/sounds/respawn.ogg",false,true);
          snd->setPan(m_fPan);
          snd->setIsPaused(false);
          snd->drop();
          m_pGame->getGameLogic()->respawn(m_pMarble,m_pCam,m_iNum-1);
        }
      }
    else
      m_iRespawnTime=0;
  }
}

void CPlayer::toggleCheckpoint(u32 iNum, bool bPlaySound) {
  if (iNum<m_aCpState.size()) {
    if (bPlaySound && m_pSndEngine) {
      ISound *snd=m_pSndEngine->play2D("data/sounds/checkpoint.ogg",false,true);
      snd->setPan(m_fPan);
      snd->setIsPaused(false);
      snd->drop();
    }

    m_aCpState[iNum]->setImage(m_pCpOk);
  }
}

void CPlayer::resetCheckpoints() {
  if (m_pSndEngine) {
    ISound *snd=m_pSndEngine->play2D("data/sounds/lap.ogg",false,true);
    snd->setPan(m_fPan);
    snd->setIsPaused(false);
    snd->drop();
  }

  for (u32 i=0; i<m_aCpState.size(); i++) m_aCpState[i]->setImage(m_pCpNok);
}

void CPlayer::showCheckpointInfo(const wchar_t *sText, u32 iTime, bool bFastest, bool bLastLap) {
  if (m_pLapTab) {
    m_pCpTime->setText(sText);
    m_pCpInfo->setBackgroundColor(bFastest?SColor(128,128,128,192):SColor(128,192,192,192));
    m_pCpInfo->setVisible(true);
    m_pLapTab->setVisible(false);
    m_pGame->addElementToToggle(m_pCpInfo,m_pTimer->getTime()+2000,false);
    m_pGame->addElementToToggle(m_pLapTab,m_pTimer->getTime()+2000,!bLastLap);
  }
}

void CPlayer::setInfoText(const wchar_t *sText) {
  if (m_pInfo) m_pInfo->setText(sText);
}

void CPlayer::setLapNo(u32 iLap, u32 iMaxLap) {
  if (m_pLapTab) {
    c8 sLaps[0xFF];
    sprintf(sLaps,"%i/%i\n",iLap,iMaxLap);
    for (u32 i=0; i<strlen(sLaps) && i<m_aLapNo.size(); i++) {
      if (sLaps[i]>='0' && sLaps[i]<='9')
        m_aLapNo[i]->setImage(m_aNumbers[sLaps[i]-'0']);
      else
        if (sLaps[i]=='/') m_aLapNo[i]->setImage(m_aNumbers[10]);
    }
    m_pLapTab->setVisible(true);
  }
}

void CPlayer::setActive(bool b) {
  m_bActive=b;
}

bool CPlayer::isActive() {
  return m_bActive;
}

void CPlayer::createPlayerGUI(bool bHSplit, const wchar_t *sName, u32 iCheckpoints) {
  dimension2du cScreenSize=m_pDevice->getVideoDriver()->getScreenSize();
  position2di pos;

  IVideoDriver *pDriver=m_pDevice->getVideoDriver();
  IGUIEnvironment *pGuienv=m_pDevice->getGUIEnvironment();

  if (m_iNum==1 || m_bNetClient) {
    pos.X=37;
    pos.Y=(bHSplit || m_bNetClient)?cScreenSize.Height-42:cScreenSize.Height/2-42;
  }
  else {
    pos.X=bHSplit?cScreenSize.Width-237:37;
    pos.Y=cScreenSize.Height-42;
  }

  m_pName=pGuienv->addStaticText(sName,rect<s32>(pos.X,pos.Y,pos.X+200,pos.Y+32));
  m_pName->setDrawBackground(true);
  m_pName->setOverrideColor(SColor(255,255,118,70));
  m_pName->setOverrideFont(m_pBigFont);
  m_pName->setBackgroundColor(SColor(128,16,16,16));
  m_pName->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

  if (bHSplit)
    pos=position2di(m_iNum==1 || m_bNetClient?5:cScreenSize.Width-37,cScreenSize.Height-42);
  else
    pos=position2di(5,m_iNum==1 && !m_bNetClient?cScreenSize.Height/2-42:cScreenSize.Height-42);

  m_pCamMode=pGuienv->addImage(pDriver->getTexture("data/freecam.png"),pos);
  m_pCamFree=pDriver->getTexture("data/freecam.png");
  m_pCamRace=pDriver->getTexture("data/racecam.png");
  m_pCpOk =pDriver->getTexture("data/checkpoint_ok.png" );
  m_pCpNok=pDriver->getTexture("data/checkpoint_nok.png");

  if (bHSplit || m_bNetClient)
    pos=position2di(m_iNum==1 || m_bNetClient?5:cScreenSize.Width-265,12);
  else
    pos=position2di(5,m_iNum==1 || m_bNetClient?12:cScreenSize.Height/2+17);

  ITexture *img=pDriver->getTexture("data/checkpoint.png");
  m_pSpeed=pGuienv->addImage(img,pos);

  m_pInfo=pGuienv->addStaticText(L"Hello World!",rect<s32>(pos.X+15,pos.Y+7,pos.X+245,pos.Y+30));

  pos=position2di(m_iNum==1 || m_bNetClient?5:bHSplit?cScreenSize.Width-22:5,bHSplit?56:m_iNum==1 || m_bNetClient?56:cScreenSize.Height/2+61);
  for (u32 i=0; i<iCheckpoints; i++) {
    IGUIImage *pCp=pGuienv->addImage(m_pCpNok,pos);
    pos.Y+=22;
    m_aCpState.push_back(pCp);
  }

  if (bHSplit || m_bNetClient)
    pos=position2di(m_iNum==1 || m_bNetClient?39:cScreenSize.Width-255,56);
  else
    pos=position2di(35,m_iNum==1 || m_bNetClient?56:cScreenSize.Height/2+61);

  m_pCpInfo=pGuienv->addTab(rect<s32>(pos.X,pos.Y,pos.X+215,pos.Y+60));
  m_pCpInfo->setBackgroundColor(SColor(128,192,192,192));
  m_pCpInfo->setDrawBackground(true);
  m_pCpInfo->setToolTipText(L"Checkpoint Info");
  pGuienv->addImage(pDriver->getTexture("data/cp_done.png"),position2di(5,5),true,m_pCpInfo);
  m_pCpTime=pGuienv->addStaticText(L"Cp_time",rect<s32>(5,40,205,60),false,true,m_pCpInfo);
  m_pCpInfo->setVisible(false);

  pos.Y+=10;
  if (m_iNum!=1 && !m_bNetClient && bHSplit) {
    pos.X=cScreenSize.Width-190;
  }

  pos=position2di(m_iNum==1 || m_bNetClient?30:bHSplit?cScreenSize.Width-240:50,bHSplit?56:m_iNum==1 || m_bNetClient?56:cScreenSize.Height/2+61);

  m_pLapTab=pGuienv->addTab(rect<s32>(pos.X,pos.Y,pos.X+200,pos.Y+55));
  m_pLapTab->setVisible(false);
  m_pLapTab->setToolTipText(L"Lap Info");
  pos=position2di(5,5);
  ITexture *tex=pDriver->getTexture("data/lap.png");
  m_pLap=pGuienv->addImage(pDriver->getTexture("data/lap.png"),pos,true,m_pLapTab);
  pos.X+=tex->getSize().Width+5;

  for (u32 j=0; j<5; j++) {
    IGUIImage *pImg=pGuienv->addImage(m_aNumbers[11],pos,true,m_pLapTab);
    m_aLapNo.push_back(pImg);
    pos.X+=26;
  }

  pos.X=(!bHSplit || m_iNum==1 || m_bNetClient)? 30:cScreenSize.Width   -136;
  pos.Y=( bHSplit || m_iNum==1 || m_bNetClient)?130:cScreenSize.Height/2+140;

  m_pFinished=pGuienv->addImage(pDriver->getTexture("data/finished.png"),pos);
  m_pFinished->setVisible(false);
  m_pFinished->setToolTipText(L"Finished");
}

void CPlayer::updateGUI(u32 iTime) {
  if (m_bActive) {
    wchar_t s[0xFF];
    if (m_iRespawnTime)
      swprintf(s,0xFF,L"Respawn in %.2f sec",((f32)m_iRespawnTime)*0.016f);
    else
      swprintf(s,0xFF,L"Time: %.2f  Speed: %.2f",((f32)iTime)/1000,m_pMarble->getNodeLinearVelocity().getLength());

    if (m_pInfo) m_pInfo->setText(s);
  }
}

void CPlayer::setFinished(bool b) {
  if (m_pSndEngine) {
    ISound *snd=m_pSndEngine->play2D("data/sounds/gameover.ogg",false,true);
    snd->setPan(m_fPan);
    snd->setIsPaused(false);
    snd->drop();
  }
  m_bFinished=b;
  if (m_pFinished) {
    m_pFinished->setVisible(b);
    if (b) m_pGame->addElementToToggle(m_pFinished,m_pTimer->getTime()+3000,false);
  }
}

bool CPlayer::hasFinished() {
  return m_bFinished;
}

vector3df CPlayer::getPosition() {
  return m_pMarble->getAbsolutePosition();
}

void CPlayer::setTimer(ITimer *pTimer) {
  m_pTimer=pTimer;
}

void CPlayer::clearGUI() {
  if (m_pCamMode ) m_pCamMode ->setVisible(false);
  if (m_pLap     ) m_pLap     ->setVisible(false);
  if (m_pFinished) m_pFinished->setVisible(false);
  if (m_pSpeed   ) m_pSpeed   ->setVisible(false);
  if (m_pInfo    ) m_pInfo    ->setVisible(false);
  if (m_pCpTime  ) m_pCpTime  ->setVisible(false);
  if (m_pCpInfo  ) m_pCpInfo  ->setVisible(false);
  if (m_pLapTab  ) m_pLapTab  ->setVisible(false);
  if (m_pName    ) m_pName    ->setVisible(false);

  for (u32 i=0; i<m_aCpState.size(); i++) if (m_aCpState[i]) m_aCpState[i]->setVisible(false);
}

ISceneNode *CPlayer::getMarble() {
  return m_pMarble;
}

bool CPlayer::initialized() {
  return m_bInitOK;
}

void CPlayer::setController(IIrrOdeController *pController) {
  m_pController=pController;
}

IIrrOdeController *CPlayer::getController() {
  return m_pController;
}

void CPlayer::camStep() {
  if (!m_bFinished) {
    bool bRaceCamOld=m_pAnimator->isRaceCam();
    m_pAnimator->camStep();
    bool bRaceCamNew=m_pAnimator->isRaceCam();

    if (bRaceCamOld!=bRaceCamNew) {
      if (bRaceCamNew) {
        if (m_pCamRace && m_pCamMode) m_pCamMode->setImage(m_pCamRace);
      }
      else {
        if (m_pCamFree && m_pCamMode) m_pCamMode->setImage(m_pCamFree);
      }
    }
    m_bCamStepped=true;
  }
}
