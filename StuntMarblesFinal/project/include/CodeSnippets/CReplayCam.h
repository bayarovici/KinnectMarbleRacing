#ifndef _C_REPLAY_CAM_NODE
  #define _C_REPLAY_CAM_NODE

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

const int REPLAY_CAM_NODE_ID=MAKE_IRR_ID('c','r','p','c');
const c8 REPLAY_CAM_NODE_NAME[0xFF]="ReplayCam";

class CReplayCam : public ISceneNode {
  private:
		ISceneManager *m_pSceneManager;
		f32 m_fFactor;
    SMaterial m_cMat;             /**< the material of the node */
    #ifdef _IRREDIT_PLUGIN
      IAnimatedMesh *m_pMesh;				/**< the mesh to be renderer in IrrEdit when compiled as plugin */
  	  c8 m_sResources[1024];				/**< the path name of the resources when copiled as plugin */
			IVideoDriver *m_pVideoDriver; /**< the video driver */
    #endif
  public:
    CReplayCam(ISceneNode *pParent, ISceneManager *pMgr, s32 iId=-1, const vector3df pos=vector3df(0,0,0), const vector3df rot=vector3df(0,0,0), const vector3df scale=vector3df(1,1,1));

    virtual void render();

    f32 getFactor() { return m_fFactor; }

    virtual const aabbox3df &getBoundingBox() const {
      static aabbox3df aBox;
      return aBox;
    }

    virtual ESCENE_NODE_TYPE getType() const;

    virtual void serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const;
    virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

		virtual ISceneNode *clone(ISceneNode* newParent=0, ISceneManager* newManager=0);
    virtual u32 getMaterialCount();
    virtual SMaterial &getMaterial(u32 iIdx);
		virtual void OnRegisterSceneNode();
};

#endif
