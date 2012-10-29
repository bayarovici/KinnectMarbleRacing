#ifndef _C_MARBLE_FOLLOW_ANIMATOR
  #define _C_MARBLE_FOLLOW_ANIMATOR

  #include <irrlicht.h>
  #include <irrklang.h>
  #include <IrrOde.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;
using namespace irrklang;

  #define _CAM_RACE_MODE 1
  #define _CAM_FREE_MODE 2

class CMarbleFollowCameraAnimator : public ISceneNodeAnimator {
  private:
    ICameraSceneNode *m_pNode;
    ISceneManager *m_pSmgr;
    u32 m_iMode;
    f32 m_fAngleH,m_fAngleV,m_fCamDist,m_fSndPan;
    ISceneCollisionManager *m_pCollisionMngr;
    bool m_bCamVMove,m_bCamHMove,m_bActive;
    u32 m_iFreeCamTime;
    IMetaTriangleSelector *m_pSelector;
    ISoundEngine *m_pSndEngine;
    vector3df m_vCamPos;

    CIrrOdeBody *m_pBody;
    u32 m_iTime,m_iStepTime;

  public:
    CMarbleFollowCameraAnimator(ICameraSceneNode *pNode, ISceneManager *pSmgr);

    void setMode(u32 iMode);

    void setAngleV(f32 f);
    void setAngleH(f32 f);

    void camLeft(f32 f);

    void camRight(f32 f);

    void setSoundEngine(ISoundEngine *pSndEngine);

    f32 getAngleH();

    void setIsActive(bool b);

    void setSelector(IMetaTriangleSelector *pSelector);

    void setCollisionManager(ISceneCollisionManager *pCollisionMngr);

    void setBody(CIrrOdeBody *pBody);

    void setPan(f32 fPan);

    virtual void 	animateNode(ISceneNode *node, u32 timeMs);

    void camStep();

    virtual ISceneNodeAnimator *createClone(ISceneNode *node, ISceneManager *newManager=0);

    virtual ESCENE_NODE_ANIMATOR_TYPE getType() const;

    virtual bool isEventReceiverEnabled() const;

    virtual ~CMarbleFollowCameraAnimator();

    bool isRaceCam();
};

#endif
