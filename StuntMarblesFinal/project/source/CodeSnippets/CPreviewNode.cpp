  #include <irrlicht.h>
  #include <CPreviewNode.h>
	#include <CPlugBULLETBYTEin.h>

	#ifdef _IRREDIT_PLUGIN
		#include <windows.h>
	#endif

CPreview::CPreview(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_pSceneManager=pMgr;
	setPosition(vector3df(0.0f,0.0f,0.0f));
  #ifdef _IRREDIT_PLUGIN
	  m_pVideoDriver=pMgr->getVideoDriver();
  	m_cMat.setFlag(EMF_LIGHTING,false);
	  GetModuleFileName(NULL,m_sResources,1024);
	  if (strstr(m_sResources,"irrEdit.exe")) *strstr(m_sResources,"irrEdit.exe")='\0';
  	strcat(m_sResources,"resources\\");
	  c8 sFileName[1024];
	  sprintf(sFileName,"%sPreviewNode.3ds",m_sResources);
    m_pMesh=pMgr->getMesh(sFileName);

		if (m_pMesh) {
			c8 sFileName[1024];
			sprintf(sFileName,"%sPreviewNode.png",m_sResources);
			m_cMat.setTexture(0,m_pVideoDriver->getTexture(sFileName));
    }
  #endif
}

ESCENE_NODE_TYPE CPreview::getType() const {
  return (ESCENE_NODE_TYPE)PREVIEW_ID;
}

void CPreview::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);
}

void CPreview::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);
}

ISceneNode *CPreview::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CPreview *pRet=new CPreview(newParent,newManager?newManager:m_pSceneManager);
  return pRet;
}

void CPreview::render() {
  #ifdef _IRREDIT_PLUGIN
    if (m_pMesh && m_pMesh->getMesh(0) && m_pMesh->getMesh(0)->getMeshBuffer(0)) {
			matrix4 tmpMatrix;
			tmpMatrix.setTranslation(AbsoluteTransformation.getTranslation());
      m_pVideoDriver->setMaterial(m_cMat);
      m_pVideoDriver->setTransform(video::ETS_WORLD,tmpMatrix);
      m_pVideoDriver->drawMeshBuffer(m_pMesh->getMesh(0)->getMeshBuffer(0));

			list<ISceneNode *> childList;
			list<ISceneNode *>::Iterator it=childList.begin();

			wchar_t s[0xFF];

			if (it!=childList.end()) {
				swprintf(s,0xFF,L"Child Nodes!");
				if (m_pLogger) m_pLogger->log(s);
				vector3df start=(*it)->getAbsolutePosition(),absStart=start;
				it++;

				for (it=childList.begin(); it!=childList.end(); it++) {
					vector3df end=(*it)->getAbsolutePosition();
					m_pVideoDriver->draw3DLine(start,end,SColor(0xFF,0xFF,0,0));
					start=end;
				}

				m_pVideoDriver->draw3DLine(start,absStart,SColor(0xFF,0xFF,0,0));
			}
			else
				if (m_pLogger) {
					static int si=0;
					swprintf(s,0xFF,L"No Child Nodes %i",si++);
					m_pLogger->log(s);
				}
    }
		m_pLogger=NULL;
  #endif
}

u32 CPreview::getMaterialCount() {
  return 1;
}

SMaterial &CPreview::getMaterial(u32 iIdx) {
  return m_cMat;
}

void CPreview::OnRegisterSceneNode() {
	#ifdef _IRREDIT_PLUGIN
		if (IsVisible) SceneManager->registerNodeForRendering(this);
	#endif
  ISceneNode::OnRegisterSceneNode();
}

void CPreview::setPosition(const core::vector3df &newpos) {
	ISceneNode::setPosition(vector3df(0.0f,0.0f,0.0f));
}

#ifdef _IRREDIT_PLUGIN
	void CPreview::setLogger(ILogger *pLogger) {
		m_pLogger=pLogger;
	}
#endif

CPreviewPoint::~CPreviewPoint() {
}

CPreviewPoint::CPreviewPoint(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_pSceneManager=pMgr;
	setPosition(vector3df(0.0f,0.0f,0.0f));
  #ifdef _IRREDIT_PLUGIN
	  m_pVideoDriver=pMgr->getVideoDriver();
  	m_cMat.setFlag(EMF_LIGHTING,false);
	  GetModuleFileName(NULL,m_sResources,1024);
	  if (strstr(m_sResources,"irrEdit.exe")) *strstr(m_sResources,"irrEdit.exe")='\0';
  	strcat(m_sResources,"resources\\");
	  c8 sFileName[1024];
	  sprintf(sFileName,"%sPreviewNode.3ds",m_sResources);
    m_pMesh=pMgr->getMesh(sFileName);

		if (m_pMesh) {
			c8 sFileName[1024];
			sprintf(sFileName,"%sPreviewPoint.png",m_sResources);
			m_cMat.setTexture(0,m_pVideoDriver->getTexture(sFileName));
    }
  #endif
}

ESCENE_NODE_TYPE CPreviewPoint::getType() const {
  return (ESCENE_NODE_TYPE)PREVIEW_POINT_ID;
}

void CPreviewPoint::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);
}

void CPreviewPoint::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);
}

ISceneNode *CPreviewPoint::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CPreviewPoint *pRet=new CPreviewPoint(newParent?newParent:getParent(),newManager?newManager:m_pSceneManager);
	pRet->cloneMembers(this,newManager);
  return pRet;
}

void CPreviewPoint::render() {
  #ifdef _IRREDIT_PLUGIN
    if (m_pMesh && m_pMesh->getMesh(0) && m_pMesh->getMesh(0)->getMeshBuffer(0)) {
			matrix4 tmpMatrix;
			tmpMatrix.setTranslation(AbsoluteTransformation.getTranslation());
      m_pVideoDriver->setMaterial(m_cMat);
      m_pVideoDriver->setTransform(video::ETS_WORLD,tmpMatrix);
      m_pVideoDriver->drawMeshBuffer(m_pMesh->getMesh(0)->getMeshBuffer(0));
    }

		if (getParent()) {
			matrix4 tmpMatrix;
			tmpMatrix.setTranslation(vector3df(0.0f,0.0f,0.0f));
      m_pVideoDriver->setTransform(video::ETS_WORLD,tmpMatrix);

			list<ISceneNode *> pOthers=getParent()->getChildren();
			list<ISceneNode *>::Iterator it;

			for (it=pOthers.begin(); it!=pOthers.end(); it++) {
				if (*it==this) {
					it++;
					if (it==pOthers.end()) it=pOthers.begin();
					m_pVideoDriver->draw3DLine(getAbsolutePosition(),(*it)->getAbsolutePosition(),SColor(0xFF,0xFF,0,0));
					return;
				}
			}
		}
  #endif
}

u32 CPreviewPoint::getMaterialCount() {
  return 1;
}

SMaterial &CPreviewPoint::getMaterial(u32 iIdx) {
  return m_cMat;
}

void CPreviewPoint::OnRegisterSceneNode() {
	#ifdef _IRREDIT_PLUGIN
		if (IsVisible) SceneManager->registerNodeForRendering(this);
	#endif
  ISceneNode::OnRegisterSceneNode();
}
