#ifndef _C_PREVIEW_NODE
  #define _C_PREVIEW_NODE

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

const int PREVIEW_ID=MAKE_IRR_ID('p','r','v','p');
const c8 PREVIEW_NAME[0xFF]="Preview";

const int PREVIEW_POINT_ID=MAKE_IRR_ID('p','r','p','t');
const c8 PREVIEW_POINT_NAME[0xFF]="PreviewPoint";

/**
 * @class CPreview
 * This node is the parent of the preview path.
 * @see CPreviewPoint
 * @see CFollowPathAnimator
 * @see CPreviewNodeFactory
 * @author Christian Keimel / dustbin::games
 */
class CPreview : public ISceneNode {
  private:
		ISceneManager *m_pSceneManager;
    SMaterial m_cMat;             /**< the material of the node */

    #ifdef _IRREDIT_PLUGIN
      IAnimatedMesh *m_pMesh;				/**< the mesh to be renderer in IrrEdit when compiled as plugin */
  	  c8 m_sResources[1024];				/**< the path name of the resources when copiled as plugin */
			IVideoDriver *m_pVideoDriver; /**< the video driver */
			ILogger *m_pLogger;
    #endif
  public:
    CPreview(ISceneNode *pParent, ISceneManager *pMgr, s32 iId=-1, const vector3df pos=vector3df(0,0,0), const vector3df rot=vector3df(0,0,0), const vector3df scale=vector3df(1,1,1));

    virtual void render();

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

		virtual void setPosition(const core::vector3df &newpos);
    #ifdef _IRREDIT_PLUGIN
		  void setLogger(ILogger *pLogger);
		#endif
};

/**
 * @class CPreviewPoint
 * This class is for the waypoints of the level preview in "Stunt Marble Racers". With this class the preview path can be set using IrrEdit
 * @see CPreview
 * @see CFollowPathAnimator
 * @see CPreviewNodeFactory
 * @author Christian Keimel / dustbin::games
 */
class CPreviewPoint : public ISceneNode {
  private:
		ISceneManager *m_pSceneManager;
    SMaterial m_cMat;             /**< the material of the node */

    #ifdef _IRREDIT_PLUGIN
      IAnimatedMesh *m_pMesh;				/**< the mesh to be renderer in IrrEdit when compiled as plugin */
  	  c8 m_sResources[1024];				/**< the path name of the resources when copiled as plugin */
			IVideoDriver *m_pVideoDriver; /**< the video driver */
    #endif
  public:
    CPreviewPoint(ISceneNode *pParent, ISceneManager *pMgr, s32 iId=-1, const vector3df pos=vector3df(0,0,0), const vector3df rot=vector3df(0,0,0), const vector3df scale=vector3df(1,1,1));
		virtual ~CPreviewPoint();

    virtual void render();

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
