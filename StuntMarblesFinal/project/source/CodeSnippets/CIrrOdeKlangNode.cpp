  #include <irrlicht.h>

  #include <CIrrOdeKlangNode.h>
  #include <CIrrOdeKlangManager.h>

CIrrOdeKlangNodeFactory::CIrrOdeKlangNodeFactory(ISceneManager *pManager) : ISceneNodeFactory() {
  m_pManager=pManager;
}

ISceneNode *CIrrOdeKlangNodeFactory::addSceneNode(ESCENE_NODE_TYPE type, ISceneNode *parent) {
  ISceneNode *pRet=NULL;
  if (type==IRR_ODE_KLANG_NODE_ID) {
    if (!parent) parent=m_pManager->getRootSceneNode();
    pRet=new CIrrOdeKlangNode(parent,m_pManager);
  }
  return pRet;
}

ISceneNode *CIrrOdeKlangNodeFactory::addSceneNode (const c8 *typeName, ISceneNode *parent) {
  if (!strcmp(typeName,IRR_ODE_KLANG_NODE_NAME)) {
		if (!parent) parent=m_pManager->getRootSceneNode();
    return new CIrrOdeKlangNode(parent,m_pManager);
  }
  return NULL;
}

u32 CIrrOdeKlangNodeFactory::getCreatableSceneNodeTypeCount() const {
  return 1;
}

ESCENE_NODE_TYPE CIrrOdeKlangNodeFactory::getCreateableSceneNodeType(u32 idx) const {
	return idx==0?(ESCENE_NODE_TYPE)IRR_ODE_KLANG_NODE_ID:ESNT_UNKNOWN;
}

const c8 *CIrrOdeKlangNodeFactory::getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const {
	return (ESCENE_NODE_TYPE)type==IRR_ODE_KLANG_NODE_ID?IRR_ODE_KLANG_NODE_NAME:NULL;
}

const c8 *CIrrOdeKlangNodeFactory::getCreateableSceneNodeTypeName(u32 idx) const {
	return idx==0?IRR_ODE_KLANG_NODE_NAME:NULL;
}

CIrrOdeKlangNodeFactory::~CIrrOdeKlangNodeFactory() {
}

//********************** CIrrOdeKlangNode implementation **********************

CIrrOdeKlangNode::CIrrOdeKlangNode(ISceneNode *pParent, ISceneManager *pMgr, s32 iId, const vector3df pos, const vector3df rot, const vector3df scale) : ISceneNode(pParent,pMgr,iId,pos,rot,scale) {
	m_fMinDistance=0.0f;
	m_fMaxDistance=0.0f;
	m_fMinVel=0.0f;
	m_fMaxVel=0.0f;
	m_fMinForce=0.0f;
	m_fMaxForce=0.0f;
	m_fVolume=0.0f;

	m_bLinearVelSound =false;
	m_bAngularVelSound=false;
	m_bCollisionSound =false;
	m_bTouching=false;
	//m_bTouched=false;
	m_bPlay=false;

	m_vOldVelocity=vector3df(0.0f,0.0f,0.0f);
	m_vBuffer=vector3df(0.0f,0.0f,0.0f);

	m_sFileName=stringw("");

  m_pBody=NULL;
  #ifndef _IRR_ODE_KLANG_PLUGIN
    ISceneNode *theParent=getParent();
    if (theParent && theParent->getType()==IRR_ODE_BODY_ID) m_pBody=(CIrrOdeBody *)theParent;

	  m_pSound=NULL;
	  m_pSound2=NULL;
		m_pSndEngine=NULL;
	#endif
}

CIrrOdeKlangNode::~CIrrOdeKlangNode() {
  #ifndef _IRR_ODE_KLANG_PLUGIN
    if (m_pSound ) { m_pSound ->setIsPaused(true); m_pSound ->drop(); }
    if (m_pSound2) { m_pSound2->setIsPaused(true); m_pSound2->drop(); }
  #endif
}

ESCENE_NODE_TYPE CIrrOdeKlangNode::getType() const {
  return (ESCENE_NODE_TYPE)IRR_ODE_KLANG_NODE_ID;
}

void CIrrOdeKlangNode::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const {
  ISceneNode::serializeAttributes(out,options);

	out->addString("FileName",m_sFileName.c_str());

	out->addFloat("minDistance",m_fMinDistance);
	out->addFloat("maxDistance",m_fMaxDistance);

	out->addBool("LinearVelSound" ,m_bLinearVelSound );
	out->addBool("AngularVelSound",m_bAngularVelSound);
	out->addBool("CollisionSound" ,m_bCollisionSound );
	out->addBool("AngularDamping" ,m_bAngDamping     );

	if (m_bLinearVelSound || m_bAngularVelSound) {
		out->addFloat("MinVelocity",m_fMinVel);
		out->addFloat("MaxVelocity",m_fMaxVel);
		out->addBool("Touching",m_bTouching);
	}
	else
	  if (m_bCollisionSound) {
		out->addFloat("MinForce",m_fMinForce);
		out->addFloat("MaxForce",m_fMaxForce);
	  }
	  else
		if (m_bAngDamping) {
			out->addFloat("MinDamping",m_fMinDamping);
			out->addFloat("MaxDamping",m_fMaxDamping);
			out->addFloat("MinVelocity",m_fMinVel);
			out->addFloat("MaxVelocity",m_fMaxVel);
			out->addBool("Touching",m_bTouching);
		}
}

void CIrrOdeKlangNode::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options) {
  ISceneNode::deserializeAttributes(in,options);

	m_sFileName=in->getAttributeAsString("FileName");

	m_fMinDistance=in->getAttributeAsFloat("minDistance");
	m_fMaxDistance=in->getAttributeAsFloat("maxDistance");

	m_bLinearVelSound =in->getAttributeAsBool("LinearVelSound");
	m_bAngularVelSound=in->getAttributeAsBool("AngularVelSound");
	m_bCollisionSound =in->getAttributeAsBool("CollisionSound");
	m_bAngDamping     =in->getAttributeAsBool("AngularDamping");

	if (m_bLinearVelSound || m_bAngularVelSound) {
		m_fMinVel=in->getAttributeAsFloat("MinVelocity");
		m_fMaxVel=in->getAttributeAsFloat("MaxVelocity");
		m_bTouching=in->getAttributeAsBool("Touching");
	}
	else
	  if (m_bCollisionSound) {
		m_fMinForce=in->getAttributeAsFloat("MinForce");
		m_fMaxForce=in->getAttributeAsFloat("MaxForce");
	  }
	  else
		if (m_bAngDamping) {
			m_fMinDamping=in->getAttributeAsFloat("MinDamping");
			m_fMaxDamping=in->getAttributeAsFloat("MaxDamping");
			m_fMinVel    =in->getAttributeAsFloat("MinVelocity");
			m_fMaxVel    =in->getAttributeAsFloat("MaxVelocity");
			m_bTouching  =in->getAttributeAsFloat("Touching");
		}

  #ifndef _IRR_ODE_KLANG_PLUGIN
    if (getParent()->getType()==IRR_ODE_BODY_ID) {
      m_pBody=(CIrrOdeBody *)getParent();
      CIrrOdeKlangManager::getSharedInstance()->addOdeKlangNode(this);
    }
	#endif
}

ISceneNode *CIrrOdeKlangNode::clone(ISceneNode* newParent, ISceneManager* newManager) {
	CIrrOdeKlangNode *pRet=new CIrrOdeKlangNode(newParent?newParent:getParent(),newManager?newManager:m_pSceneManager);

	pRet->setMinDistance(m_fMinDistance);
	pRet->setMaxDistance(m_fMaxDistance);
	pRet->setCollisionSound(m_bCollisionSound);
	pRet->setLinearVelSound(m_bLinearVelSound);
	pRet->setAngularVelSound(m_bAngularVelSound);
	pRet->setMinVelocity(m_fMinVel);
	pRet->setMaxVelocity(m_fMaxVel);
	pRet->setFileName(m_sFileName);
	pRet->setMinForce(m_fMinForce);
	pRet->setMaxForce(m_fMaxForce);
	pRet->setAngularDamping(m_bAngDamping);
	pRet->setMinDamping(m_fMinDamping);
	pRet->setMaxDamping(m_fMaxDamping);

  return pRet;
}

f32 CIrrOdeKlangNode::getMinDistance() { return m_fMinDistance; }
f32 CIrrOdeKlangNode::getMaxDistance() { return m_fMaxDistance; }
f32 CIrrOdeKlangNode::getMinVelocity() { return m_fMinVel; }
f32 CIrrOdeKlangNode::getMaxVelocity() { return m_fMaxVel; }

f32 CIrrOdeKlangNode::getMinForce() { return m_fMinForce; }
f32 CIrrOdeKlangNode::getMaxForce() { return m_fMaxForce; }

bool CIrrOdeKlangNode::isCollisionSound () { return m_bCollisionSound ; }
bool CIrrOdeKlangNode::isLinearVelSound () { return m_bLinearVelSound ; }
bool CIrrOdeKlangNode::isAngularVelSound() { return m_bAngularVelSound; }
bool CIrrOdeKlangNode::isAngularDampingSound() { return m_bAngDamping; }

void CIrrOdeKlangNode::setMinDistance(f32 f) { m_fMinDistance=f; }
void CIrrOdeKlangNode::setMaxDistance(f32 f) { m_fMaxDistance=f; }
void CIrrOdeKlangNode::setMinVelocity(f32 f) { m_fMinVel=f; }
void CIrrOdeKlangNode::setMaxVelocity(f32 f) { m_fMaxVel=f; }

void CIrrOdeKlangNode::setMinForce(f32 f) { m_fMinForce=f; }
void CIrrOdeKlangNode::setMaxForce(f32 f) { m_fMaxForce=f; }

void CIrrOdeKlangNode::setMinDamping(f32 f) { m_fMinDamping=f; }
void CIrrOdeKlangNode::setMaxDamping(f32 f) { m_fMaxDamping=f; }

void CIrrOdeKlangNode::setCollisionSound (bool b) { m_bCollisionSound =b; }
void CIrrOdeKlangNode::setLinearVelSound (bool b) { m_bLinearVelSound =b; }
void CIrrOdeKlangNode::setAngularVelSound(bool b) { m_bAngularVelSound=b; }
void CIrrOdeKlangNode::setAngularDamping(bool b) { m_bAngDamping=b; }

CIrrOdeBody *CIrrOdeKlangNode::getBody() { return m_pBody; }

#ifndef _IRR_ODE_KLANG_PLUGIN
	void CIrrOdeKlangNode::setSoundEngine(ISoundEngine *pSndEngine) {
		m_pSndEngine=pSndEngine;
	}
#endif

void CIrrOdeKlangNode::setTouching(bool b) {
	m_bTouching=b;
}

bool CIrrOdeKlangNode::isTouching() {
	return m_bTouching;
}

stringc CIrrOdeKlangNode::getFileName() { return m_sFileName; }

void CIrrOdeKlangNode::setFileName(stringc sFileName) { m_sFileName=sFileName; }

void CIrrOdeKlangNode::play() {
  m_bPlay=true;
}

void CIrrOdeKlangNode::update(ICameraSceneNode *pCam1, ICameraSceneNode *pCam2) {
  #ifndef _IRR_ODE_KLANG_PLUGIN
    m_vOldVelocity=m_vBuffer;
    if (!m_pSndEngine || !m_pBody) return;
    m_vBuffer=m_pBody->getLinearVelocity();

    if (m_bCollisionSound) {
      if (m_pBody->getTouched()!=NULL) {
        f32 v=(m_vOldVelocity-m_pBody->getLinearVelocity()).getLength();
        if (v>m_fMinForce) {
          v-=m_fMinForce;
          v/=m_fMaxForce-m_fMinForce;
          if (v>1.0f) v=1.0f;

          vector3df vPos=m_pBody->getAbsolutePosition();

          if (pCam2==NULL) {
            if (m_pSndEngine) {
              m_pSound=m_pSndEngine->play3D(m_sFileName.c_str(),vec3df(vPos.X,vPos.Y,vPos.Z),false,true);
              m_pSound->setMinDistance(m_fMinDistance);
              m_pSound->setMaxDistance(m_fMaxDistance);
              m_pSound->setVolume(v);
              m_pSound->setIsPaused(false);
            }
          }
          else {
            f32 d1=(pCam1->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength(),
                d2=(pCam2->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength();

            if (d1<m_fMaxDistance) {
              if (m_pSndEngine) {
				#ifndef _IRR_ODE_KLANG_PLUGIN
                  m_pSound=m_pSndEngine->play2D(m_sFileName.c_str(),false,true);
                  f32 vol=1.0f-((d1-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance));
                  vol*=v;
                  if (vol>1.0f) vol=1.0f;
                  m_pSound->setVolume(vol);
                  m_pSound->setPan(0.8f);
                  m_pSound->setIsPaused(false);
                #endif
              }
            }

            if (d2<m_fMaxDistance) {
              if (m_pSndEngine) {
				#ifndef _IRR_ODE_KLANG_PLUGIN
                  m_pSound2=m_pSndEngine->play2D(m_sFileName.c_str(),false,true);
                  f32 vol=1.0f-((d2-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance));
                  vol*=v;
                  if (vol>1.0f) vol=1.0f;
                  m_pSound2->setVolume(vol);
                  m_pSound2->setPan(-0.8f);
                  m_pSound2->setIsPaused(false);
                #endif
              }
            }
          }
        }
        m_bPlay=false;
      }
    }
    else
      if (m_bAngularVelSound || m_bLinearVelSound) {
        if (!m_bTouching || m_pBody->getTouched()!=NULL) {
          vector3df vVel=m_pBody->getLinearVelocity();
          f32 v=m_bLinearVelSound?m_pBody->getLinearVelocity().getLength():m_pBody->getAngularVelocity().getLength();

          if (v>m_fMinVel) {
            if (v>m_fMaxVel) v=m_fMaxVel;
            v-=m_fMinVel;
            v/=(m_fMaxVel-m_fMinVel);
            vector3df vPos=m_pBody->getAbsolutePosition();

            if (pCam2==NULL) {
              if (!m_pSound) {
                if (m_pSndEngine) {
                  m_pSound=m_pSndEngine->play3D(m_sFileName.c_str(),vec3df(vPos.X,vPos.Y,vPos.Z),true,true);
                  m_pSound->setMinDistance(m_fMinDistance);
                  m_pSound->setMaxDistance(m_fMaxDistance);
                  m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
                  m_pSound->setVelocity(vec3df(vVel.X,vVel.Y,vVel.Z));
                  m_pSound->setVolume(v);
                  m_pSound->setIsPaused(false);
                }
              }
              else {
                if (m_pSndEngine) {
                  m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
                  m_pSound->setVelocity(vec3df(vVel.X,vVel.Y,vVel.Z));
                  m_pSound->setVolume(v);
                }
              }
            }
            else {
              f32 d1=(pCam1->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength(),
                  d2=(pCam2->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength();

              if (d1<m_fMaxDistance) {
                if (!m_pSound) m_pSound=m_pSndEngine->play2D(m_sFileName.c_str(),true,true);
                f32 vol=1.0f-((d1-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance));
                vol*=v;
                if (vol>1.0f) vol=1.0f;
                m_pSound->setVolume(vol);
                m_pSound->setPan(0.8f);
                m_pSound->setIsPaused(false);
              }

              if (d2<m_fMaxDistance) {
                if (!m_pSound2) m_pSound2=m_pSndEngine->play2D(m_sFileName.c_str(),true,true);
                f32 vol=1.0f-((d2-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance));
                vol*=v;
                if (vol>1.0f) vol=1.0f;
                m_pSound2->setVolume(vol);
                m_pSound2->setPan(-0.8f);
                m_pSound2->setIsPaused(false);
              }
            }
          }
          else {
            if (m_pSound ) m_pSound ->setVolume(0.0f);
            if (m_pSound2) m_pSound2->setVolume(0.0f);
          }
        }
        else {
          if (m_pSound ) m_pSound ->setVolume(0.0f);
          if (m_pSound2) m_pSound2->setVolume(0.0f);
        }
      }
      else {
        if (!m_bTouching || m_pBody->getTouched()!=NULL) {
          vector3df vPos=m_pBody->getAbsolutePosition(),
                    vVel=m_pBody->getLinearVelocity();
          if (pCam2==NULL) {
            if (m_pSndEngine) {
              if (!m_pSound) {
                m_pSound=m_pSndEngine->play3D(m_sFileName.c_str(),vec3df(vPos.X,vPos.Y,vPos.Z),true,true);
                m_pSound->setMinDistance(m_fMinDistance);
                m_pSound->setMaxDistance(m_fMaxDistance);
                m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
                m_pSound->setVelocity(vec3df(vVel.X,vVel.Y,vVel.Z));
                m_pSound->setVolume(m_fVolume);
                m_pSound->setIsPaused(false);
              }
              else {
                m_pSound->setVolume((m_bTouching || m_pBody->getTouched()==NULL)?m_fVolume:0.0f);
                m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
                //m_pSound->setVelocity(vec3df(vVel.X,vVel.Y,vVel.Z));
              }
            }
          }
          else {
            f32 d1=(pCam1->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength(),
                d2=(pCam2->getAbsolutePosition()-m_pBody->getAbsolutePosition()).getLength();

            if (d1<m_fMaxDistance) {
              if (m_pSndEngine) {
                if (!m_pSound) {
                  m_pSound=m_pSndEngine->play2D(m_sFileName.c_str(),true,true);
                  m_pSound->setVolume(0.0f);
                }
                f32 vol=1.0f-((d1-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance)),v=m_fVolume*vol;
                if (v>1.0f) v=1.0f;
                m_pSound->setVolume(v);
                m_pSound->setPan(0.8f);
                m_pSound->setIsPaused(false);
              }
            }

            if (d2<m_fMaxDistance) {
              if (m_pSndEngine) {
                if (!m_pSound2) {
                  m_pSound2=m_pSndEngine->play2D(m_sFileName.c_str(),true,true);
                  m_pSound2->setVolume(0.0f);
                }
                f32 vol=1.0f-((d2-m_fMinDistance)/(m_fMaxDistance-m_fMinDistance)),v=m_fVolume*vol;
                if (v>1.0f) v=1.0f;
                m_pSound2->setVolume(v);
                m_pSound2->setPan(-0.8f);
                m_pSound2->setIsPaused(false);
              }
            }
          }
        }
        else {
          if (m_pSound ) m_pSound ->setVolume(0.0f);
          if (m_pSound2) m_pSound2->setVolume(0.0f);
        }
      }

    if (m_bAngDamping) {
      vector3df v1=m_pBody->getLinearVelocity();

      f32 fFact1=m_pBody->getNodeAngularDamping();
      if (fFact1>m_fMinDamping) {
        irr::ode::CIrrOdeGeom *g=m_pBody->getTouched();
        /*if (g) {
          irr::ode::CIrrOdeBody *b=g->getBody();
          if (b) {
            v1-=b->getLinearVelocity();
          }
        }*/
        if (g) {
          fFact1-=m_fMinDamping;
          fFact1/=m_fMaxDamping-m_fMinDamping;
          f32 fFact2=v1.getLength();
          if (fFact2>m_fMaxVel-m_fMinVel) fFact2=m_fMaxVel-m_fMinVel;
          fFact2/=m_fMaxVel-m_fMinVel;
          f32 fVol=fFact1*fFact2;

          vector3df vPos=m_pBody->getAbsolutePosition(),
                    vVel=m_pBody->getLinearVelocity();

          if (pCam2==NULL) {
            if (m_pSndEngine) {
              if (!m_pSound) {
                m_pSound=m_pSndEngine->play3D(m_sFileName.c_str(),vec3df(vPos.X,vPos.Y,vPos.Z),true,true);
                m_pSound->setMinDistance(m_fMinDistance);
                m_pSound->setMaxDistance(m_fMaxDistance);
                m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
                m_pSound->setVelocity(vec3df(vVel.X,vVel.Y,vVel.Z));
                m_pSound->setVolume(fVol);
              }
              else {
                m_pSound->setVolume(fVol);
                m_pSound->setPosition(vec3df(vPos.X,vPos.Y,vPos.Z));
              }
            }
          }
        }
      }
    }
	#endif
}

void CIrrOdeKlangNode::setVolume(f32 f) {
  m_fVolume=f;
}

void CIrrOdeKlangNode::setIsPaused(bool b) {
  #ifndef _IRR_ODE_KLANG_PLUGIN
    if (m_pSound ) m_pSound ->setIsPaused(b);
    if (m_pSound2) m_pSound2->setIsPaused(b);
  #endif
}

void CIrrOdeKlangNode::stop() {
  #ifndef _IRR_ODE_KLANG_PLUGIN
    if (m_pSound ) m_pSound ->stop();
    if (m_pSound2) m_pSound2->stop();
  #endif
}
