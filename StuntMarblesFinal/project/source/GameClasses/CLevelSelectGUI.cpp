  #include <GameClasses/CLevelSelectGUI.h>
  #include <IrrOde.h>
	#include <CIrrOdeKlangManager.h>
	#include <CShadowManager.h>

CLevelSelectGUI::CLevelSelectGUI(IrrlichtDevice *pDevice, ISoundEngine *pSndEngine, ISceneManager *pPreviewManager, bool bNetbook) {
  m_pDevice=pDevice;
  m_pGuienv=m_pDevice->getGUIEnvironment();
  m_pDriver=m_pDevice->getVideoDriver();
  m_pSmgr=m_pDevice->getSceneManager();

  m_pSndEngine=pSndEngine;
  m_bOdeInitialized=false;
  m_bNetbook=bNetbook;

  m_pPreviewManager=pPreviewManager;

  dimension2du screenSize=m_pDriver->getScreenSize();
  s32 iLeftX=screenSize.Width/2-400,iRightX=screenSize.Width/2+400;
  m_pTab=m_pGuienv->addTab(rect<s32>(iLeftX,0,iRightX,600));
  m_pLevels=m_pGuienv->addListBox(rect<s32>(10,30,340,190),m_pTab,-1,true);
  m_cViewPort=rect<s32>(iLeftX+350,10,iLeftX+790,190);
  IGUIStaticText *pLvl=m_pGuienv->addStaticText(L"Select Level",rect<s32>(10,10,340,27),true,false,m_pTab,-1,true);
  pLvl->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

  m_pLevelList=new CLevelList(m_pDevice);
  for (u32 i=0; i<m_pLevelList->m_aLevels.size(); i++)
    m_pLevels->addItem(m_pLevelList->m_aLevels[i]->m_sName.c_str());

  m_iHiScoreItem=-1;
  if (m_pLevels->getItemCount()>0) m_pLevels->setSelected(0);

	IImage *img=m_pDriver->createImageFromFile("data/load_preview.png");
	if (img) {
		ITexture *pTex=m_pDriver->addTexture("load_preview",img);
		if (pTex) {
		  position2di pos=m_cViewPort.getCenter()-position2di(pTex->getOriginalSize().Width/2,pTex->getOriginalSize().Height/2);
		  m_pPreview=m_pGuienv->addImage(pTex,pos);
		  if (m_pPreview) m_pPreview->setVisible(false);
		}
    else m_pPreview=NULL;
    img->drop();
	}
	else m_pPreview=NULL;

	img=m_pDriver->createImageFromFile("data/no_preview.png");
	if (img) {
	  ITexture *pTex=m_pDriver->addTexture("no_preview",img);
	  if (pTex) {
	    position2di pos=m_cViewPort.getCenter()-position2di(pTex->getOriginalSize().Width/2,pTex->getOriginalSize().Height/2);
	    m_pNoPreview=m_pGuienv->addImage(pTex,pos);
	    if (m_pNoPreview) m_pNoPreview->setVisible(false);
	  }
	  else m_pNoPreview=NULL;
	  img->drop();
	}
	else m_pNoPreview=NULL;

  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();
	m_pPreviewAnimator=NULL;
}

CLevelSelectGUI::~CLevelSelectGUI() {
  delete m_pLevelList;
  m_pLevels=NULL;
  if (m_pPreviewAnimator) m_pPreviewAnimator->drop();
  if (m_bOdeInitialized) CIrrOdeManager::getSharedInstance()->closeODE();
}

IGUITab *CLevelSelectGUI::getTab() {
  return m_pTab;
}

bool CLevelSelectGUI::update() {
  u32 iTime=m_pTimer->getTime();
  if (m_pPreviewAnimator) m_pPreviewAnimator->animateNode(m_pPreviewManager->getActiveCamera(),iTime);

  if (m_pLevels && m_iHiScoreItem!=m_pLevels->getSelected()) {
    if (m_pSndEngine && m_iHiScoreItem!=-2) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
    CIrrOdeManager *odeManager=CIrrOdeManager::getSharedInstance();
    m_iHiScoreItem=m_pLevels->getSelected();

    IFileSystem *pFs=m_pDevice->getFileSystem();
    bool bNetBookLevel=m_bNetbook && pFs->existFile(path(m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sNetbookFile));
    const wchar_t *sFile=bNetBookLevel?m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sNetbookFile.c_str():m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sFile.c_str();

    m_pPreviewManager->clear();
    m_pPreview->setVisible(true);
    m_pNoPreview->setVisible(false);
    if (m_bOdeInitialized) odeManager->closeODE();

    m_pDriver->beginScene(true,true,SColor(0,200,200,200));
    m_pSmgr->drawAll();
    this->drawAll();
    m_pGuienv->drawAll();
    m_pDriver->endScene();

    CIrrOdeKlangManager::getSharedInstance()->removeAllNodes();
    CShadowManager::getSharedInstance()->clear();
    m_pPreviewManager->getMeshCache()->clear();
    m_pPreviewManager->loadScene(stringc(sFile).c_str());

    ISceneNode *pSkybox=m_pPreviewManager->getSceneNodeFromType(ESNT_SKY_BOX);
    if (pSkybox)
      for (u32 i=0; i<pSkybox->getMaterialCount(); i++)
        for (u32 j=0; j<MATERIAL_MAX_TEXTURES; j++) {
          pSkybox->getMaterial(i).TextureLayer[j].LODBias=-128;
          if (pSkybox->getMaterial(i).TextureLayer[j].Texture!=NULL && j!=0) {
            pSkybox->getMaterial(i).TextureLayer[j].Texture=pSkybox->getMaterial(i).TextureLayer[0].Texture;
          }
        }

    ICameraSceneNode *pCam=m_pPreviewManager->addCameraSceneNode(NULL,vector3df(0.0f,0.0f,0.0f),vector3df(1.0f,0.0f,0.0f));
    m_pPreviewAnimator=new CFollowPathAnimator(m_pDevice,"preview",150,500,pCam,m_pPreviewManager);
    pCam->addAnimator(m_pPreviewAnimator);
    m_pPreviewAnimator->setCamParameter(pCam);
    m_pPreviewManager->setActiveCamera(pCam);
    pCam->setAspectRatio((f32)((f32)m_cViewPort.getWidth()/(f32)m_cViewPort.getHeight()));

    odeManager->initODE();
    odeManager->initPhysics();
    m_bOdeInitialized=true;
    m_pPreview->setVisible(false);
    m_pPreviewAnimator->start();

    m_pNoPreview->setVisible(!m_pPreviewAnimator->animationRunning());

    return true;
  }
  return false;
}

const wchar_t *CLevelSelectGUI::getHiScoreFile() {
  return m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sHiScoreFile.c_str();
}

const wchar_t *CLevelSelectGUI::getLevelName() {
  return m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sName.c_str();
}

const wchar_t *CLevelSelectGUI::getLevelFile() {
  return m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sFile.c_str();
}

const wchar_t *CLevelSelectGUI::getLevelInfo() {
  return m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sInfo.c_str();
}

const wchar_t *CLevelSelectGUI::getLevelNotebookFile() {
  return m_pLevelList->m_aLevels[m_iHiScoreItem]->m_sNetbookFile.c_str();
}

void CLevelSelectGUI::drawAll() {
	CIrrOdeManager::getSharedInstance()->step();
  rect<s32> oldViewPort=m_pDriver->getViewPort();
  m_pDriver->setViewPort(m_cViewPort);
  m_pDriver->setRenderTarget(NULL);
  m_pPreviewManager->drawAll();
  m_pDriver->setViewPort(oldViewPort);
  m_pGuienv->drawAll();
}

bool CLevelSelectGUI::isMultiplayer() {
  ISceneNode *pl2=m_pPreviewManager->getSceneNodeFromName("player2");
  return pl2!=NULL;
}

void CLevelSelectGUI::setSelectedLevel(s32 idx) {
  if (idx<(s32)m_pLevels->getItemCount()) m_pLevels->setSelected(idx);
}

s32 CLevelSelectGUI::getSelectedLevel() {
  return m_pLevels->getSelected();
}
