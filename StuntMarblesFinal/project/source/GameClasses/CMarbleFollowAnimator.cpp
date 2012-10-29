#include <GameClasses/CMarbleFollowAnimator.h>

CMarbleFollowCameraAnimator::CMarbleFollowCameraAnimator(ICameraSceneNode *pNode, ISceneManager *pSmgr) {
  m_pNode=pNode;
  m_pSmgr=pSmgr;
  m_iMode=_CAM_FREE_MODE;
  m_pBody=NULL;
  m_fAngleH=0.0f;
  m_fAngleV=15.0f;
  m_fCamDist=10.0f;
  m_bCamVMove=false;
  m_bCamHMove=false;
  m_bActive=true;
  m_iFreeCamTime=0;
  m_iTime=0;
  m_iStepTime=0;

  vector3df pos=pNode->getPosition();
  vector3df rot=pos.getHorizontalAngle();
  m_fAngleH=rot.Y;
  pNode->setParent(m_pSmgr->getRootSceneNode());
  m_pCollisionMngr=NULL;
}

void CMarbleFollowCameraAnimator::setMode(u32 iMode) {
  m_iMode=iMode;
}

void CMarbleFollowCameraAnimator::setAngleV(f32 f) { m_fAngleV=f; }
void CMarbleFollowCameraAnimator::setAngleH(f32 f) { m_fAngleH=f; }

void CMarbleFollowCameraAnimator::camLeft(f32 f) {
  m_fAngleH+=f;
  m_iFreeCamTime=m_iTime;
}

void CMarbleFollowCameraAnimator::camRight(f32 f) {
  m_fAngleH-=f;
  m_iFreeCamTime=m_iTime;
}

void CMarbleFollowCameraAnimator::setSoundEngine(ISoundEngine *pSndEngine) {
  m_pSndEngine=pSndEngine;
}

f32 CMarbleFollowCameraAnimator::getAngleH() { return m_fAngleH; }

void CMarbleFollowCameraAnimator::setIsActive(bool b) {
  m_bActive=b;
}

void CMarbleFollowCameraAnimator::setSelector(IMetaTriangleSelector *pSelector) {
  m_pSelector=pSelector;
}

void CMarbleFollowCameraAnimator::setCollisionManager(ISceneCollisionManager *pCollisionMngr) {
  m_pCollisionMngr=pCollisionMngr;
}

void CMarbleFollowCameraAnimator::setBody(CIrrOdeBody *pBody) {
  m_pBody=pBody;
}

void CMarbleFollowCameraAnimator::setPan(f32 fPan) {
  m_fSndPan=fPan;
}

void CMarbleFollowCameraAnimator::animateNode (ISceneNode *node, u32 timeMs) {
  m_iTime=timeMs;
}

void CMarbleFollowCameraAnimator::camStep() {
  if (!m_pBody) return;
  vector3df v=-m_pBody->getLinearVelocity();

  if (v.getLength()<10.0f || m_iTime-m_iFreeCamTime<2500) {
    if (m_iMode==_CAM_RACE_MODE) {
      m_iMode=_CAM_FREE_MODE;
      if (m_pSndEngine) {
        ISound *snd=m_pSndEngine->play2D("data/sounds/camchange.ogg",false,true);
        snd->setPan(m_fSndPan);
        snd->setIsPaused(false);
        snd->drop();
      }
    }

    while (m_fAngleH>360.0f) m_fAngleH-=360.0f;
    while (m_fAngleH<  0.0f) m_fAngleH+=360.0f;

    if (m_bCamVMove) {
      f32 fAngleMove=(v.getLength()-20.0f)/15.0f+0.1f;

      if (fAngleMove>0.5f) fAngleMove=0.25f;
      if (fAngleMove<0.1f) fAngleMove=0.10f;

      if (m_fAngleV<15.0f) {
        m_fAngleV+=0.25f;
        if (m_fAngleV>=15.0f) {
          m_fAngleV=15.0f;
          m_bCamVMove=false;
        }
      }
      else
        if (m_fAngleV>=15.0f) {
          m_fAngleV-=0.25f;
          if (m_fAngleV<=15.0f) {
            m_fAngleV=15.0f;
            m_bCamVMove=false;
          }
        }
    }
    else {
      if (m_fAngleV>17.5f || m_fAngleV<12.5f)
        m_bCamVMove=true;
      else
        m_fAngleV=15.0f;
    }
  }
  else {
    f32 fAngleMove=(v.getLength()-10.0f)/15.0f+0.1f;
    if (fAngleMove>0.5f) fAngleMove=0.5f;
    if (m_iMode!=_CAM_RACE_MODE) {
      m_iMode=_CAM_RACE_MODE;
      if (m_pSndEngine) {
        ISound *snd=m_pSndEngine->play2D("data/sounds/camchange.ogg",false,true);
        snd->setPan(m_fSndPan);
        snd->setIsPaused(false);
        snd->drop();
      }
    }
    f32 fCamAngle=v.getHorizontalAngle().Y,
        fCamV=(f32)(asin(v.Y/v.getLength())/GRAD_PI2)+15.0f;

    while (fCamAngle>m_fAngleH+180.0f) fCamAngle-=360.0f;
    while (fCamAngle<m_fAngleH-180.0f) fCamAngle+=360.0f;

    if (m_fAngleH<fCamAngle) {
      m_fAngleH+=fAngleMove;
      if (fCamAngle<m_fAngleH) m_fAngleH=fCamAngle;
    }

    if (m_fAngleH>fCamAngle) {
      m_fAngleH-=fAngleMove;
      if (fCamAngle>m_fAngleH) m_fAngleH=fCamAngle;
    }

    if (m_bCamVMove) {
      f32 fMoveV=(fCamV-m_fAngleV)/10;

      if (fMoveV> 0.5f) fMoveV= 0.5f;
      if (fMoveV<-0.5f) fMoveV=-0.5f;

      if (m_fAngleV<fCamV) {
        m_fAngleV+=fMoveV;
        if (m_fAngleV>fCamV) {
          m_fAngleV=fCamV;
          m_bCamVMove=false;
        }
      }

      if (m_fAngleV>fCamV) {
        m_fAngleV+=fMoveV;
        if (m_fAngleV<fCamV) {
          m_fAngleV=fCamV;
          m_bCamVMove=false;
        }
      }
    }
    else
      if (m_fAngleV<fCamV-3.0f || m_fAngleV>fCamV+3.0f)
        m_bCamVMove=true;
  }

  f32 dist=(v.getLength()/3)+5;
  if (m_fCamDist>dist) {
    m_fCamDist-=0.1f;
    if (m_fCamDist<dist) m_fCamDist=dist;
  }
  if (m_fCamDist<dist) {
    m_fCamDist+=0.1f;
    if (m_fCamDist>dist) m_fCamDist=dist;
  }

  if (m_fAngleV> 80.0f) m_fAngleV= 80.0f;
  if (m_fAngleV<-80.0f) m_fAngleV=-80.0f;

  if (m_fCamDist<10.0f) m_fCamDist=10.0f;
  if (m_fCamDist>25.0f) m_fCamDist=25.0f;

  f32 fDist=m_fCamDist;

  f32 fCamX=(irr::f32)(fDist*sin(m_fAngleH*GRAD_PI2)*cos(m_fAngleV*GRAD_PI2)),
      fCamY=(irr::f32)(fDist*sin(m_fAngleV*GRAD_PI2)),
      fCamZ=(irr::f32)(fDist*cos(m_fAngleH*GRAD_PI2)*cos(m_fAngleV*GRAD_PI2));

  m_vCamPos=m_pBody->getPosition()+vector3df(fCamX,fCamY,fCamZ);
  vector3df marblePos=m_pBody->getPosition(),colPoint;
  triangle3df tri;
  const line3d<f32> cLine=line3d<f32>(marblePos,m_vCamPos);

  const ISceneNode *pNode=NULL;
  if (m_pCollisionMngr && m_pCollisionMngr->getCollisionPoint(cLine,m_pSelector,colPoint,tri,pNode)) {
    vector3df len=colPoint-marblePos;
    colPoint-=(colPoint-marblePos).normalize();
    m_vCamPos=colPoint+vector3df(0.0f,len.getLength()<5.0f?5-len.getLength():0.0f,0.0f);
  }

  m_pNode->setPosition(m_vCamPos);
  m_pNode->setTarget(marblePos);
}

ISceneNodeAnimator *CMarbleFollowCameraAnimator::createClone (ISceneNode *node, ISceneManager *newManager) {
  CMarbleFollowCameraAnimator *pNew=new CMarbleFollowCameraAnimator((ICameraSceneNode *)node,newManager?newManager:m_pSmgr);
  return pNew;
}

ESCENE_NODE_ANIMATOR_TYPE CMarbleFollowCameraAnimator::getType () const {
  return ESNAT_UNKNOWN ;
}

bool CMarbleFollowCameraAnimator::isEventReceiverEnabled () const {
  return false;
}

CMarbleFollowCameraAnimator::~CMarbleFollowCameraAnimator() {
}

bool CMarbleFollowCameraAnimator::isRaceCam() {
  return m_iMode==_CAM_RACE_MODE;
}
