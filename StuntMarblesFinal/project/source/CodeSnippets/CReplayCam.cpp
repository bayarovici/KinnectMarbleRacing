  #include <irrlicht.h>
  #include <CReplayCam.h>

	#ifdef _IRREDIT_PLUGIN
		#include <windows.h>
	#endif

CReplayCam::CReplayCam(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_pSceneManager=pMgr;
	m_fFactor=1.0f;
  #ifdef _IRREDIT_PLUGIN
	  m_pVideoDriver=pMgr->getVideoDriver();
  	m_cMat.setFlag(EMF_LIGHTING,false);
	  GetModuleFileName(NULL,m_sResources,1024);
	  if (strstr(m_sResources,"irrEdit.exe")) *strstr(m_sResources,"irrEdit.exe")='\0';
  	strcat(m_sResources,"resources\\");
	  c8 sFileName[1024];
	  sprintf(sFileName,"%sreplayCam.3ds",m_sResources);
    m_pMesh=pMgr->getMesh(sFileName);

		if (m_pMesh) {
			c8 sFileName[1024];
			sprintf(sFileName,"%sreplayCam.jpg",m_sResources);
			m_cMat.setTexture(0,m_pVideoDriver->getTexture(sFileName));
    }
  #endif
}

ESCENE_NODE_TYPE CReplayCam::getType() const {
  return (ESCENE_NODE_TYPE)REPLAY_CAM_NODE_ID;
}

void CReplayCam::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);
	out->addFloat("Factor",m_fFactor);
}

void CReplayCam::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);
	m_fFactor=in->getAttributeAsFloat("Factor");
}

ISceneNode *CReplayCam::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CReplayCam *pRet=new CReplayCam(newParent?newParent:getParent(),newManager?newManager:m_pSceneManager);
	pRet->setPosition(getPosition());
	pRet->setRotation(getRotation());
  return pRet;
}

void CReplayCam::render() {
  #ifdef _IRREDIT_PLUGIN
    if (m_pMesh && m_pMesh->getMesh(0) && m_pMesh->getMesh(0)->getMeshBuffer(0)) {
			matrix4 tmpMatrix;
			tmpMatrix.setTranslation(AbsoluteTransformation.getTranslation());
      m_pVideoDriver->setMaterial(m_cMat);
      m_pVideoDriver->setTransform(video::ETS_WORLD,tmpMatrix);
      m_pVideoDriver->drawMeshBuffer(m_pMesh->getMesh(0)->getMeshBuffer(0));
    }
  #endif
}

u32 CReplayCam::getMaterialCount() {
  return 1;
}

SMaterial &CReplayCam::getMaterial(u32 iIdx) {
  return m_cMat;
}

void CReplayCam::OnRegisterSceneNode() {
	#ifdef _IRREDIT_PLUGIN
		if (IsVisible) SceneManager->registerNodeForRendering(this);
	#endif
  ISceneNode::OnRegisterSceneNode();
}
