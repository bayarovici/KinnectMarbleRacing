  #include <irrlicht.h>
  #include <CRespawnNode.h>

	#ifdef _IRREDIT_PLUGIN
		#include <windows.h>
	#endif

CRespawnNode::CRespawnNode(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_pSceneManager=pMgr;
	m_iNum=0;
	m_vLookAt=vector3df(1.0f,0.0f,0.0f);
  #ifdef _IRREDIT_PLUGIN
	  m_pVideoDriver=pMgr->getVideoDriver();
  	m_cMat.setFlag(EMF_LIGHTING,false);
	  GetModuleFileName(NULL,m_sResources,1024);
	  if (strstr(m_sResources,"irrEdit.exe")) *strstr(m_sResources,"irrEdit.exe")='\0';
  	strcat(m_sResources,"resources\\");
	  c8 sFileName[1024];
	  sprintf(sFileName,"%srespawn_node.3ds",m_sResources);
    m_pMesh=pMgr->getMesh(sFileName);

		if (m_pMesh) {
			c8 sFileName[1024];
			sprintf(sFileName,"%sRespawnNode.png",m_sResources);
			m_cMat.setTexture(0,m_pVideoDriver->getTexture(sFileName));
    }
  #endif
}

ESCENE_NODE_TYPE CRespawnNode::getType() const {
  return (ESCENE_NODE_TYPE)RESPAWN_NODE_ID;
}

void CRespawnNode::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);

	out->addInt("Num",m_iNum);
	out->addVector3d("LookAt",m_vLookAt);
}

void CRespawnNode::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);

	m_iNum=in->getAttributeAsInt("Num");
	m_vLookAt=in->getAttributeAsVector3d("LookAt");
}

ISceneNode *CRespawnNode::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CRespawnNode *pRet=new CRespawnNode(newParent,newManager?newManager:m_pSceneManager);
  return pRet;
}

void CRespawnNode::render() {
  #ifdef _IRREDIT_PLUGIN
    if (m_pMesh && m_pMesh->getMesh(0) && m_pMesh->getMesh(0)->getMeshBuffer(0)) {
			matrix4 tmpMatrix;
			tmpMatrix.setTranslation(AbsoluteTransformation.getTranslation());
      m_pVideoDriver->setMaterial(m_cMat);
      m_pVideoDriver->setTransform(video::ETS_WORLD,tmpMatrix);
      m_pVideoDriver->drawMeshBuffer(m_pMesh->getMesh(0)->getMeshBuffer(0));
			m_pVideoDriver->draw3DLine(vector3df(0.0f,0.0f,0.0f),10.0f*m_vLookAt,SColor(0xFF,0xFF,0,0));
    }
  #endif
}

u32 CRespawnNode::getMaterialCount() {
  return 1;
}

SMaterial &CRespawnNode::getMaterial(u32 iIdx) {
  return m_cMat;
}

void CRespawnNode::OnRegisterSceneNode() {
	#ifdef _IRREDIT_PLUGIN
		if (IsVisible) SceneManager->registerNodeForRendering(this);
	#endif
  ISceneNode::OnRegisterSceneNode();
}

vector3df CRespawnNode::getLookAt() {
  return m_vLookAt;
}

u32 CRespawnNode::getNum() {
	return m_iNum;
}
