  #include <IrrOdeNet/CIrrOdeRePlayer.h>
  #include <IrrOdeNet/CEventIdentify.h>
  #include <IrrOdeNet/CIrrOdeMessageFactory.h>

  #include <irrOde.h>
  #include <CMessage.h>
  #include <CShadowManager.h>
  #include <CIrrOdeKlangManager.h>
  #include <CRenderToTextureManager.h>

using namespace ode;

CIrrOdeRePlayer::CIrrOdeRePlayer(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
  m_pTimer=pDevice->getTimer();
  m_pSndEngine=irrklang::createIrrKlangDevice();
  m_iState=_E_INIT;
  m_bValidReplay=false;
  m_pLoadingProgress=NULL;
  m_bFollowCam=false;
  m_bPaused=false;
}

CIrrOdeRePlayer::~CIrrOdeRePlayer() {
  deleteAllMessages();
}

void CIrrOdeRePlayer::loadReplay(const c8 *sFile) {
  deleteAllMessages();
  IFileSystem *pFs=m_pDevice->getFileSystem();
  IReadFile *f=pFs->createAndOpenFile(path(sFile));
  m_bValidReplay=false;

  if (f==NULL) {
    printf("can't open \"%s\"\n",sFile);
    return;
  }

  CIrrOdeMessageFactory *pFactory=CIrrOdeMessageFactory::getSharedMessageFactory();

  while (f->getPos()<f->getSize()) {
    c8 pBuffer[4096];
    u32 iMessageSize;

    f->read(&iMessageSize,sizeof(u32));
    f->read(pBuffer,iMessageSize);

    m_cSerializer.setBuffer(pBuffer,iMessageSize);
    IIrrOdeEvent *evt=pFactory->createMessage(&m_cSerializer);

    if (evt) {
      m_lMessages.push_back(evt);
      if (!m_bValidReplay) {
        if (evt->getType()==eMessageIdentify) {
          CEventIdentify *p=(CEventIdentify *)evt;
          m_bValidReplay=!strcmp(p->getIdentify(),_IRR_ODE_IDENTIFY_STRING) && !strcmp(p->getAppName(),"marbles2");
        }
      }

      if (evt->getType()==irr::ode::eIrrOdeEventLoadScene) {
        irr::ode::CIrrOdeEventLoadScene *p=(irr::ode::CIrrOdeEventLoadScene *)evt;
        m_sSceneFile=stringc(p->getScene());
        printf("\n\t\tscene file=\"%s\"\n\n",m_sSceneFile.c_str());
      }
    }
  }

  f->drop();

  if (!m_bValidReplay) {
    printf("ERROR: \"%s\" is not a valid replay!\n",sFile);
    deleteAllMessages();
  }

  m_itMsg=m_lMessages.begin();
  m_iState=_E_STOPPED;
}

void CIrrOdeRePlayer::init() {
  m_iLastTime=m_pTimer->getTime();
  m_iCurTime=m_iLastTime;
  CIrrOdeManager::getSharedInstance()->closeODE();
  CRenderToTextureManager::getSharedInstance()->clearLists();
  CIrrOdeKlangManager::getSharedInstance()->setSoundEngine(m_pSndEngine);
  CIrrOdeKlangManager::getSharedInstance()->activate();
  if (m_sSceneFile.size()>0) {
    CShadowManager::getSharedInstance()->clear();
    m_pDevice->getSceneManager()->loadScene(m_sSceneFile.c_str());

    ISceneNode *pSkybox=m_pDevice->getSceneManager()->getSceneNodeFromType(ESNT_SKY_BOX);
    if (pSkybox)
      for (u32 i=0; i<pSkybox->getMaterialCount(); i++) {
        for (u32 j=0; j<MATERIAL_MAX_TEXTURES; j++) {
          pSkybox->getMaterial(i).TextureLayer[j].LODBias=-128;
          if (pSkybox->getMaterial(i).TextureLayer[j].Texture!=NULL && j!=0) {
            pSkybox->getMaterial(i).TextureLayer[j].Texture=pSkybox->getMaterial(i).TextureLayer[0].Texture;
          }
        }
      }

    CShadowManager::getSharedInstance()->setProgressCallback(this);
    CShadowManager::getSharedInstance()->update();
    CShadowManager::getSharedInstance()->setProgressCallback(NULL);
    CIrrOdeManager::getSharedInstance()->initODE();
    CIrrOdeManager::getSharedInstance()->initPhysics();

    ISceneManager *pSmgr=m_pDevice->getSceneManager();
    CRenderToTextureManager::getSharedInstance()->addTarget(pSmgr->getSceneNodeFromName("player1"));
    CRenderToTextureManager::getSharedInstance()->setActive(true);

    core::list<CIrrOdeWorld *> lWorlds=CIrrOdeManager::getSharedInstance()->getWorlds();

    if (lWorlds.getSize()>0)
      m_iStepSize=(u32)((*lWorlds.begin())->getStepSize()*1000.0f);
    else
      m_iStepSize=16;

    m_pLoadingProgress->setVisible(false);
  }
  startReplay();
}

bool CIrrOdeRePlayer::update() {
  bool bRet=false;
  static int iFrame=0;
  m_iCurTime=m_pTimer->getTime();
  if (m_iState!=_E_PLAYING) {
    m_iLastTime=m_iCurTime;
    return bRet;
  }

  while (m_iCurTime-m_iLastTime>m_iStepSize && m_iState==_E_PLAYING && m_itMsg!=m_lMessages.end()) {
    bRet=true;
    m_iLastTime+=m_iStepSize;

    IIrrOdeEvent *m=*m_itMsg;
    u16 iMsgCode=m->getType();

    while (!m_bPaused && iMsgCode!=irr::ode::eIrrOdeEventStep && m_iState==_E_PLAYING) {
      CIrrOdeManager::getSharedInstance()->getQueue()->postEvent(m,false);

      if (iMsgCode==irr::ode::eIrrOdeEventBodyRemoved) {
        CIrrOdeEventBodyRemoved *p=(CIrrOdeEventBodyRemoved *)m;
        ISceneNode *pNode=m_pDevice->getSceneManager()->getSceneNodeFromId(p->getBodyId());
        if (pNode) pNode->setVisible(false);
      }

      if (m_itMsg==m_lMessages.end()) {
        m_iState=_E_FINISHED;
        CRenderToTextureManager::getSharedInstance()->setActive(false);
        replayFinished();
      }
      else {
        m_itMsg++;
        if (m_itMsg==m_lMessages.end()) {
          m_iState=_E_FINISHED;
          CRenderToTextureManager::getSharedInstance()->setActive(false);
          replayFinished();
        }
        else {
          m=*m_itMsg;
          iMsgCode=m->getType();
        }
      }
    }

    CIrrOdeEventStep *pStep=new CIrrOdeEventStep ((u32)0);
    pStep->setFrameNo(iFrame);
    ICameraSceneNode *pCam=m_pDevice->getSceneManager()->getActiveCamera();
    CIrrOdeManager::getSharedInstance()->getQueue()->postEvent(pStep,false);
    m_pDevice->getSceneManager()->setActiveCamera(pCam);

    if (m_iState==_E_PLAYING) {
      if ((*m_itMsg)->getType()==eIrrOdeEventStep) {
        CIrrOdeEventStep *p=(CIrrOdeEventStep *)*m_itMsg;
        step(p->getStepNo());
        m_itMsg++;
        if (m_itMsg==m_lMessages.end()) {
          m_iState=_E_FINISHED;
          CRenderToTextureManager::getSharedInstance()->setActive(false);
          replayFinished();
        }
      }
    }
    else
      if (m_iState==_E_FINISHED) CIrrOdeKlangManager::getSharedInstance()->deactivate();
  }
  iFrame++;
  return bRet;
}

const char *CIrrOdeRePlayer::getAppName() {
  return m_sAppName.c_str();
}

void CIrrOdeRePlayer::deleteAllMessages() {
  while (m_lMessages.getSize()>0) {
    list<IIrrOdeEvent *>::Iterator it=m_lMessages.begin();
    IIrrOdeEvent *m=*it;
    m_lMessages.erase(it);
    delete m;
  }
}

bool CIrrOdeRePlayer::isValidReplay() {
  return m_bValidReplay;

}

void CIrrOdeRePlayer::scanReplay(IMessageHandler *pHandler) {
  list<IIrrOdeEvent *>::Iterator it;
  for (it=m_lMessages.begin(); it!=m_lMessages.end(); it++) {
    IIrrOdeEvent *m=*it;
    pHandler->handleMessage(m);
  }
}

void CIrrOdeRePlayer::progress(u32 iDone, u32 iTotal) {
  if (m_pLoadingProgress==NULL) return;
  printf("shadow progress: %u of %u done (%.2f percent)\n",iDone,iTotal,100.0f*((f32)iDone)/((f32)iTotal));
  wchar_t s[0xFF];
  swprintf(s,0xFF,L"Initializing: %.2f %%",100.0f*((f32)iDone)/((f32)iTotal));
  m_pLoadingProgress->setText(s);
  m_pDevice->getVideoDriver()->beginScene(true,true,SColor(128,64,64,64));
  m_pDevice->getGUIEnvironment()->drawAll();
  m_pDevice->getVideoDriver()->endScene();
}

void CIrrOdeRePlayer::setLoadingProgressElement(IGUIStaticText *pProgress) {
  m_pLoadingProgress=pProgress;
}

void CIrrOdeRePlayer::start() {
  m_iState=_E_PLAYING;
}

void CIrrOdeRePlayer::stop() {
  m_iState=_E_STOPPED;
}

bool CIrrOdeRePlayer::isStarted() {
  return m_iState==_E_PLAYING;
}
