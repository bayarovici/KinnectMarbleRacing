#ifndef _C_RENDER_TO_TEXTURE_MANAGER
  #define _C_RENDER_TO_TEXTURE_MANAGER

  #include <event/IIrrOdeEventListener.h>
  #include <CReplayCam.h>
  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CRenderToTextureNode;

class CRenderToTextureManager : public irr::ode::IIrrOdeEventListener {
  protected:
    ISceneManager *m_pSmgr;
    IVideoDriver *m_pDriver;

    ITexture *m_pVideoTexture;
    ICameraSceneNode *m_pVideoCam;
    list<CReplayCam *> m_lCams;
    list<CRenderToTextureNode *> m_lNodes;
    list<ISceneNode *> m_lTargets;
    list<ISceneNode *>::Iterator m_itTarget;
    u32 m_iStepCnt,
        m_iStepsToRender,
        m_iTargetSwitch,
        m_iLastFrameNo,
        m_iTextureSize;

    bool m_bActive,
         m_bSwitch;

    void findCam(ISceneNode *pParent);

    CRenderToTextureManager();
    virtual ~CRenderToTextureManager();

  public:
    static CRenderToTextureManager *getSharedInstance();

    void setSceneManager(ISceneManager *pSmgr);

    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);

    void addRenderToTextureNode(CRenderToTextureNode *pNode);
    void addTarget(ISceneNode *pTarget);

    void setActive(bool b);
    void setStepsToRender(u32 i);
    void setTextureSize(u32 i) { m_iTextureSize=i; }
    void setGlobalSwitch(bool b) { m_bSwitch=b; }
    void clearLists();
};

#endif
