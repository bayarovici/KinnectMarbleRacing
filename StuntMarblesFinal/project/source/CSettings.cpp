  #include <CSettings.h>
  #define _USE_MATH_DEFINES
  #include <math.h>
  #undef _USE_MATH_DEFINES
	#include <CShadowManager.h>

/**
 * The constructor
 * @param scene to be loaded and displayed in the background. Can be "NULL" if no scene should be shown
 * @param the XML file to read and store the settings
 * @param title of the window
 * @param background color of the window
 * @param Irrlicht device. Should be "NULL" if a window is to be created
 */
CSettings::CSettings(const c8 *sScene, const c8 *sSettingsFile, const wchar_t *sTitle, SColor cBackground, IrrlichtDevice *pDevice) {
  //Create a software device
	if (!pDevice) {
    m_pDevice=createDevice(EDT_SOFTWARE,dimension2d<u32>(230,200), 16, false, false, false, 0);
		m_pDevice->getFileSystem()->addZipFileArchive("data/menu.dat",false,false);
	}
  else m_pDevice=pDevice;

  if (sTitle) m_pDevice->setWindowCaption(sTitle);

  if (sSettingsFile) strcpy(m_sSettingsFile,sSettingsFile); else m_sSettingsFile[0]='\0';
  if (sScene       ) strcpy(m_sScene       ,sScene       ); else m_sScene       [0]='\0';
  if (sTitle) wcscpy(m_sTitle,sTitle); else m_sTitle[0]=0;

  m_iResolution=0;
  m_iShadows   =0;
  m_iDriver    =EDT_NULL;
  m_bFullscreen=true;

  m_iMinX=0;
  m_iMinY=0;

  //init the Irrlicht classes
  m_pGuiEnv=m_pDevice->getGUIEnvironment();
  m_pSceneManager=m_pDevice->getSceneManager();
  m_pDriver=m_pDevice->getVideoDriver();
  m_cBackground=cBackground;

  m_bSettingsChanged=false;
  m_iClose=0;

  load();
  if (!pDevice) {
    //Now add an event receiver
    m_pDevice->setEventReceiver(this);
    //createGUI(m_pGuiEnv,NULL,position2di(5,80),true);
    m_bDropDevice=true;
  }
  else m_bDropDevice=false;
}

void CSettings::createGUI(IGUIEnvironment *gui, IGUIElement *pParent, position2di pos, bool bCancel) {
  dimension2du dim=gui->getSkin()->getFont()->getDimension(L"graphics driver");
  s32 iHeight=dim.Height,iWidth=dim.Width,yPos=pos.Y+1,xPos=pos.X;
  //add the static textfields
  if (!pParent) m_pGuiEnv->addStaticText(m_sTitle,rect<s32>(5,5,125,18),true,true,0,-1,true);
  m_pGuiEnv->addStaticText(L"graphics driver",rect<s32>(xPos,yPos,xPos+iWidth+16,yPos+iHeight+2),false,true,pParent,-1,true); yPos+=2*iHeight;
  m_pGuiEnv->addStaticText(L"resolution"     ,rect<s32>(xPos,yPos,xPos+iWidth+16,yPos+iHeight+2),false,true,pParent,-1,true); yPos+=2*iHeight;
  m_pGuiEnv->addStaticText(L"fullscreen"     ,rect<s32>(xPos,yPos,xPos+iWidth+16,yPos+iHeight+2),false,true,pParent,-1,true); yPos+=2*iHeight;
  m_pGuiEnv->addStaticText(L"shadows"        ,rect<s32>(xPos,yPos,xPos+iWidth+16,yPos+iHeight+2),false,true,pParent,-1,true); yPos+=2*iHeight;

  //add the "driver", "resolution" and "shadows" comboboxes
  yPos=pos.Y;
  m_pDrivers   =m_pGuiEnv->addComboBox(rect<s32>(xPos+iWidth+20,yPos,xPos+3*iWidth+20,yPos+iHeight+4),pParent,3); yPos+=2*iHeight;
  m_pResolution=m_pGuiEnv->addComboBox(rect<s32>(xPos+iWidth+20,yPos,xPos+3*iWidth+20,yPos+iHeight+4),pParent,4); yPos+=2*iHeight;

  //add the "fullscreen"
  m_pFullscreen=m_pGuiEnv->addCheckBox(false,rect<s32>(xPos+iWidth+20,yPos,xPos+iWidth+20+2*iHeight,yPos+iHeight+4),pParent,5); yPos+=2*iHeight;
  m_pShadows   =m_pGuiEnv->addComboBox(rect<s32>(xPos+iWidth+20,yPos,xPos+3*iWidth+20,yPos+iHeight+4),pParent,6); yPos+=4*iHeight;

	m_pShadows->addItem(L"0 - shadows off");
	for (u32 i=1; i<4; i++) {
		wchar_t s[0xFF];
		swprintf(s,0xFF,L"%i",i);
		m_pShadows->addItem(s);
	}
	m_pShadows->addItem(L"4 - all shadows");

  dimension2du dimBtn=gui->getSkin()->getFont()->getDimension(L"Start");
  position2d<u32> posBtn=position2d<u32>(xPos+iWidth,yPos);

  //add the "OK" and "Cancel" buttons
  m_pOk=m_pGuiEnv->addButton(rect<s32>(posBtn.X+20,yPos,posBtn.X+35+dimBtn.Width,yPos+iHeight+5),pParent,1,L"OK");
  if (bCancel)
    m_pCancel=m_pGuiEnv->addButton(rect<s32>(posBtn.X+40+dimBtn.Width,yPos,posBtn.X+55+2*dimBtn.Width,yPos+iHeight+5),pParent,2,L"Cancel");

  //If no driver information was found in the settings file we initialize the driver list with all available drivers
  if (m_aDrvs.size()==0) {
    _DRV *drv;
    drv=new _DRV(); drv->iDriver=EDT_SOFTWARE     ; wcscpy(drv->sName,L"Software"     ); m_aDrvs.push_back(drv);
    drv=new _DRV(); drv->iDriver=EDT_BURNINGSVIDEO; wcscpy(drv->sName,L"BurningsVideo"); m_aDrvs.push_back(drv);
    drv=new _DRV(); drv->iDriver=EDT_DIRECT3D8    ; wcscpy(drv->sName,L"Direct3D8"    ); m_aDrvs.push_back(drv);
    drv=new _DRV(); drv->iDriver=EDT_DIRECT3D9    ; wcscpy(drv->sName,L"Direct3D9"    ); m_aDrvs.push_back(drv);
    drv=new _DRV(); drv->iDriver=EDT_OPENGL       ; wcscpy(drv->sName,L"OpenGL"       ); m_aDrvs.push_back(drv);
  }

  //If driver information were found we just add the drivers from the XML file
  for (u32 i=0; i<m_aDrvs.size(); i++) m_pDrivers->addItem(m_aDrvs[i]->sName);

  //we add all video modes with a bit depth of at least 16 to the "resolution" combobox
  m_pVModes=m_pDevice->getVideoModeList();
  for (s32 i=0; i<m_pVModes->getVideoModeCount(); i++) {
    dimension2d<u32> dim=m_pVModes->getVideoModeResolution(i);
    if (m_pVModes->getVideoModeDepth(i)>=16 && dim.Width>=m_iMinX && dim.Height>=m_iMinY) {
      dimension2du res=m_pVModes->getVideoModeResolution(i);
      _VRES *vRes=new _VRES();
      vRes->iWidth =res.Width ;
      vRes->iHeight=res.Height;
      vRes->iBpp   =m_pVModes->getVideoModeDepth(i);
      m_aVModes.push_back(vRes);
    }
  }

  //now we fill the "resolution" combobox with the drivers we added to the array
  for (u32 i=0; i<m_aVModes.size(); i++) {
    wchar_t s[0xFF];
    _VRES *res=m_aVModes[i];
    swprintf(s,0xFE,L"%ix%i %i bpp",res->iWidth,res->iHeight,res->iBpp);
    m_pResolution->addItem(s);
  }

  m_pCam=NULL;

  //we load the scene if a scene is wanted
  if (m_sScene[0]) {
    m_pSceneManager->loadScene(m_sScene);
    m_pCam=reinterpret_cast<ICameraSceneNode *>(m_pSceneManager->getSceneNodeFromName("theCamera"));
    if (m_pDevice->getSceneManager()->getSceneNodeFromName("skybox"))
      m_pDevice->getSceneManager()->getSceneNodeFromName("skybox")->remove();
  }

  updateGui();
  m_pDevice->getCursorControl()->setVisible(true);
}

/**
 * The destructor saves the settings file if the settings were changed
 */
CSettings::~CSettings() {
  //Settings changed and "OK" button clicked?
  if (m_iClose==1) save();
  for (u32 i=0; i<m_aVModes.size(); i++) delete m_aVModes[i];
  if (m_pDevice && m_bDropDevice) {
    m_pDevice->closeDevice();
    m_pDevice->drop();
  }
}

void CSettings::save() {

  //Write settings to the settings file
  IXMLWriter *pXml=m_pDevice->getFileSystem()->createXMLWriter(m_sSettingsFile);

  wchar_t res[0xFF],drv[0xFF],dummy[0xFF];
  if (pXml) {
		wchar_t ws[0xFF];

    swprintf(res,0xFE,L"%i",m_iResolution);
    swprintf(drv,0xFE,L"%i",m_iDriver);

    array<stringw> aResN; aResN.push_back(stringw(L"value"));
    array<stringw> aResV; aResV.push_back(stringw(res));
    if (m_iMinX>0) { aResN.push_back("minx"); swprintf(dummy,0xFF,L"%i",m_iMinX); aResV.push_back(dummy); }
    if (m_iMinY>0) { aResN.push_back("miny"); swprintf(dummy,0xFF,L"%i",m_iMinY); aResV.push_back(dummy); }
    array<stringw> aDrvN; aDrvN.push_back(stringw(L"value"));
    array<stringw> aDrvV; aDrvV.push_back(stringw(drv));
    array<stringw> aFlsN; aFlsN.push_back(stringw(L"value"));
    array<stringw> aFlsV; aFlsV.push_back(stringw(m_bFullscreen?L"1":L"0"));
    array<stringw> aShdN; aShdN.push_back(stringw(L"value"));
		swprintf(ws,0xFF,L"%i",m_iShadows);
    array<stringw> aShdV; aShdV.push_back(ws);

    pXml->writeXMLHeader();
    pXml->writeElement(L"settings"); pXml->writeLineBreak();
    for (u32 i=0; i<m_aDrvs.size(); i++) {
      wchar_t s[0xFF];
      swprintf(s,0xFE,L"%i",m_aDrvs[i]->iDriver);
      pXml->writeElement(L"videodriver",true,L"name",m_aDrvs[i]->sName,L"id",s);
      pXml->writeLineBreak();
    }
    pXml->writeElement(L"resolution",true,aResN,aResV); pXml->writeLineBreak();
    pXml->writeElement(L"fullscreen",true,aFlsN,aFlsV); pXml->writeLineBreak();
    pXml->writeElement(L"shadows"   ,true,aShdN,aShdV); pXml->writeLineBreak();
    pXml->writeElement(L"driver"    ,true,aDrvN,aDrvV); pXml->writeLineBreak();

    pXml->writeElement(L"set",true); pXml->writeLineBreak();

    pXml->writeClosingTag(L"settings");
    pXml->drop();
  }
}

void CSettings::updateGui() {
  //we init the GUI items with the values read from the settings XML file
  for (u32 i=0; i<m_aDrvs.size(); i++) if (m_aDrvs[i]->iDriver==m_iDriver) m_pDrivers->setSelected(i);

  m_pResolution->setSelected(m_iResolution);
	m_pShadows->setSelected   (m_iShadows   );

  m_pFullscreen->setChecked(m_bFullscreen);
}

void CSettings::load() {
  //read the settings from the settings file
  IXMLReaderUTF8 *pXml=m_pDevice->getFileSystem()->createXMLReaderUTF8(m_sSettingsFile);
  m_bRunFirst=true;

	if (pXml) {
		while (pXml->read()) {
			//This is special: the video drivers to choose from need to be in the XML file
			if (!strcmp(pXml->getNodeName(),"videodriver")) {
				_DRV *drv=new _DRV();
				wcscpy(drv->sName,stringw(pXml->getAttributeValue("name")).c_str());
				drv->iDriver=(E_DRIVER_TYPE)atoi(pXml->getAttributeValue("id"  ));
				m_aDrvs.push_back(drv);
			}

			if (!strcmp(pXml->getNodeName(),"resolution")) {
				m_iResolution=atoi(pXml->getAttributeValue("value"));
				if (pXml->getAttributeValue("minx")) m_iMinX=atoi(pXml->getAttributeValue("minx"));
				if (pXml->getAttributeValue("miny")) m_iMinY=atoi(pXml->getAttributeValue("miny"));
			}
			if (!strcmp(pXml->getNodeName(),"fullscreen")) m_bFullscreen=atoi(pXml->getAttributeValue("value"))!=0;
			if (!strcmp(pXml->getNodeName(),"shadows"   )) m_iShadows   =atoi(pXml->getAttributeValue("value"));
			if (!strcmp(pXml->getNodeName(),"driver"    )) m_iDriver    =(E_DRIVER_TYPE)atoi(pXml->getAttributeValue("value"));
			if (!strcmp(pXml->getNodeName(),"set"       )) m_bRunFirst  =false;
		}

		pXml->drop();
	}

	CShadowManager::getSharedInstance()->setLevel(m_iShadows);
}

/**
 * Run the dialog
 */
u32 CSettings::run() {
  while (m_pDevice->run() && !m_iClose) {
    m_pDriver->beginScene(true,true,m_cBackground);
    if (m_pCam) m_pCam->setTarget(vector3df(0.0f,0.0f,0.0f));

    m_pSceneManager->drawAll();
    m_pGuiEnv->drawAll();

    m_pDriver->endScene();
  }

	if (m_iClose==1) {
		CShadowManager::getSharedInstance()->setLevel(m_iShadows);
	}

  return m_iClose?m_iClose:2;
}


u32 CSettings::getClose() {
  u32 iRet=m_iClose;
  m_iClose=0;
  return iRet;
}

/**
 * Event receiver for the Irrlicht event pipeline
 */
bool CSettings::OnEvent(const SEvent &event) {
  bool bRet=false;

  if (event.EventType==EET_GUI_EVENT) {
    if (event.GUIEvent.EventType==EGET_BUTTON_CLICKED) {
      m_iClose=event.GUIEvent.Caller->getID();
      bRet=true;
    }

    if (event.GUIEvent.EventType==EGET_COMBO_BOX_CHANGED) {
      m_bSettingsChanged=true;
      switch (event.GUIEvent.Caller->getID()) {
        case 3: m_iDriver=m_aDrvs[m_pDrivers->getSelected()]->iDriver; bRet=true; break;
        case 4: m_iResolution=m_pResolution->getSelected();bRet=true; break;
				case 6: m_iShadows   =m_pShadows->getSelected(); bRet=true; break;
      }
    }

    if (event.GUIEvent.EventType==EGET_CHECKBOX_CHANGED) {
      m_bSettingsChanged=true;
      switch (event.GUIEvent.Caller->getID()) {
        case 5: m_bFullscreen=m_pFullscreen->isChecked(); bRet=true; break;
      }
    }
  }

  if (event.EventType==EET_KEY_INPUT_EVENT) {
    if (!event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_RETURN:
          m_iClose=1;
          bRet=true;
          break;

        case KEY_ESCAPE:
          m_iClose=2;
          bRet=true;
          break;

        default:
          break;
      }
    }
  }

  return bRet;
}

/**
 * This function drops the software device created to display the window and creates a new device
 * with the settings of the user
 */
IrrlichtDevice *CSettings::createDeviceFromSettings() {
  if (m_pDevice && m_bDropDevice) {
    m_pDevice->closeDevice();
    m_pDevice->run();
    m_pDevice->drop();
    m_pDevice=NULL;
    m_bDropDevice=false;
  }
  if (m_iResolution<m_aVModes.size()) {
    _VRES *res=m_aVModes[m_iResolution];
    m_pDevice=createDevice(m_iDriver,dimension2du(res->iWidth,res->iHeight),res->iBpp,m_bFullscreen,m_iShadows!=0,false,0);
    m_pDevice->run();
  }
  return m_pDevice;
}

u32 CSettings::getShadowLevel() {
	return m_iShadows;
}

void CSettings::addValidDriver(const wchar_t *sName, E_DRIVER_TYPE iDriver) {
  _DRV *drv;
  drv=new _DRV();
  drv->iDriver=iDriver;
  wcscpy(drv->sName,sName);
  m_aDrvs.push_back(drv);
}

void CSettings::setMinResolution(dimension2di iRes) {
  m_iMinX=iRes.Width;
  m_iMinY=iRes.Height;
}
