  #include <irrlicht.h>
  #include <CRenderToTextureNode.h>
	#include <CPlugBULLETBYTEin.h>
	#include <CRenderToTextureManager.h>

	#ifdef _IRREDIT_PLUGIN
		#include <windows.h>
	#endif

CRenderToTextureNode::CRenderToTextureNode(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_iMaterial=0;
	m_iTexture=0;
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
			sprintf(sFileName,"%sRttNode.png",m_sResources);
			m_cMat.setTexture(0,m_pVideoDriver->getTexture(sFileName));
    }
  #endif

  CRenderToTextureManager::getSharedInstance()->addRenderToTextureNode(this);
}

ESCENE_NODE_TYPE CRenderToTextureNode::getType() const {
  return (ESCENE_NODE_TYPE)RTT_NODE_ID;
}

void CRenderToTextureNode::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);
  out->addInt("Material",m_iMaterial);
  out->addInt("Texture",m_iTexture);
}

void CRenderToTextureNode::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);
  m_iMaterial=in->getAttributeAsInt("Material");
  m_iTexture=in->getAttributeAsInt("Texture");
}

ISceneNode *CRenderToTextureNode::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CRenderToTextureNode *pRet=new CRenderToTextureNode(newParent,newManager?newManager:m_pSceneManager);
  return pRet;
}

void CRenderToTextureNode::render() {
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
				vector3df start=(*it)->getAbsolutePosition(),absStart=start;
				it++;

				for (it=childList.begin(); it!=childList.end(); it++) {
					vector3df end=(*it)->getAbsolutePosition();
					m_pVideoDriver->draw3DLine(start,end,SColor(0xFF,0xFF,0,0));
					start=end;
				}

				m_pVideoDriver->draw3DLine(start,absStart,SColor(0xFF,0xFF,0,0));
			}
    }
  #endif
}

u32 CRenderToTextureNode::getMaterialCount() {
  return 1;
}

SMaterial &CRenderToTextureNode::getMaterial(u32 iIdx) {
  return m_cMat;
}

void CRenderToTextureNode::OnRegisterSceneNode() {
	#ifdef _IRREDIT_PLUGIN
		if (IsVisible) SceneManager->registerNodeForRendering(this);
	#endif
  ISceneNode::OnRegisterSceneNode();
}

void CRenderToTextureNode::setPosition(const core::vector3df &newpos) {
	ISceneNode::setPosition(vector3df(0.0f,0.0f,0.0f));
}

