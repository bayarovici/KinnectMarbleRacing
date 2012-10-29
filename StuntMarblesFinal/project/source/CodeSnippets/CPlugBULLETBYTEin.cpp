  #include <CPlugBULLETBYTEin.h>
  #include <CManagedShadow.h>
  #include <CPreviewNode.h>
  #include <CReplayCam.h>
  #include <CRespawnNode.h>
  #include <CRenderToTextureNode.h>

CPlugBULLETBYTEin::CPlugBULLETBYTEin(ISceneManager *pManager) : ISceneNodeFactory() {
  m_pManager=pManager;
}

ISceneNode *CPlugBULLETBYTEin::addSceneNode(ESCENE_NODE_TYPE type, ISceneNode *parent) {
  if (type==MANAGED_SHADOW_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
    return new CManagedShadow(parent,m_pManager);
  }

  if (type==PREVIEW_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
    return new CPreview(parent,m_pManager);
  }

	if (type==PREVIEW_POINT_ID) {
		if (!parent) parent=m_pManager->getRootSceneNode();
		return new CPreviewPoint(parent,m_pManager);
	}

  if (type==REPLAY_CAM_NODE_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
    return new CReplayCam(parent,m_pManager);
  }

  if (type==RESPAWN_NODE_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
    return new CRespawnNode(parent,m_pManager);
  }

  if (type==RTT_NODE_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
	return new CRenderToTextureNode(parent,m_pManager);
  }

  return NULL;
}

ISceneNode *CPlugBULLETBYTEin::addSceneNode (const c8 *typeName, ISceneNode *parent) {
  if (!strcmp(typeName,"CManagedShadow")) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CManagedShadow(parent,m_pManager);
  }

  if (!strcmp(typeName,PREVIEW_NAME)) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CPreview(parent,m_pManager);
		#ifdef _IRREDIT_PLUGIN
		  ((CPreview *)pRet)->setLogger(m_pLogger);
		#endif
  }

	if (!strcmp(typeName,PREVIEW_POINT_NAME)) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CPreviewPoint(parent,m_pManager);
	}

  if (!strcmp(typeName,REPLAY_CAM_NODE_NAME)) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CReplayCam(parent,m_pManager);
  }

  if (!strcmp(typeName,RESPAWN_NODE_NAME)) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CRespawnNode(parent,m_pManager);
  }

  if (!strcmp(typeName,RTT_NODE_NAME)) {
	  if (!parent) parent=m_pManager->getRootSceneNode();
    return new CRenderToTextureNode(parent,m_pManager);
  }

  return NULL;
}

u32 CPlugBULLETBYTEin::getCreatableSceneNodeTypeCount() const {
  return 6;
}

ESCENE_NODE_TYPE CPlugBULLETBYTEin::getCreateableSceneNodeType(u32 idx) const {
  if (idx==0) return (ESCENE_NODE_TYPE)MANAGED_SHADOW_ID;
  if (idx==1) return (ESCENE_NODE_TYPE)PREVIEW_ID;
	if (idx==2) return (ESCENE_NODE_TYPE)PREVIEW_POINT_ID;
  if (idx==3) return (ESCENE_NODE_TYPE)REPLAY_CAM_NODE_ID;
  if (idx==4) return (ESCENE_NODE_TYPE)RESPAWN_NODE_ID;
  if (idx==5) return (ESCENE_NODE_TYPE)RTT_NODE_ID;

	return ESNT_UNKNOWN;
}

const c8 *CPlugBULLETBYTEin::getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const {
  switch (type) {
    case MANAGED_SHADOW_ID: return MANAGED_SHADOW_NAME;
    case PREVIEW_ID: return PREVIEW_NAME;
    case PREVIEW_POINT_ID: return PREVIEW_POINT_NAME;
    case REPLAY_CAM_NODE_ID: return REPLAY_CAM_NODE_NAME;
    case RESPAWN_NODE_ID: return RESPAWN_NODE_NAME;
	case RTT_NODE_ID: return RTT_NODE_NAME;
    default: return NULL;
  }
}

const c8 *CPlugBULLETBYTEin::getCreateableSceneNodeTypeName(u32 idx) const {
  if (idx==0) return MANAGED_SHADOW_NAME;
  if (idx==1) return PREVIEW_NAME;
	if (idx==2) return PREVIEW_POINT_NAME;
  if (idx==3) return REPLAY_CAM_NODE_NAME;
  if (idx==4) return RESPAWN_NODE_NAME;
  if (idx==5) return RTT_NODE_NAME;

	return NULL;
}

CPlugBULLETBYTEin::~CPlugBULLETBYTEin() {
}

