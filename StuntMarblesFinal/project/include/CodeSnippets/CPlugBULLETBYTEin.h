#ifndef _C_PLUG_BULLETBYTE_IN
  #define _C_PLUG_BULLETBYTE_IN

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

/**
 * @class CManagedShadowFactory
 * This class implements Irrlicht's scene node factory for managed shadows
 * @see CManagedShadow
 * @see CShadowManager
 * @author Christian Keimel / dustbin::games
 */
class CPlugBULLETBYTEin : public ISceneNodeFactory {
  protected:
    ISceneManager *m_pManager;

  public:
    CPlugBULLETBYTEin(ISceneManager *pManager);
    virtual ~CPlugBULLETBYTEin();
    virtual ISceneNode *addSceneNode (ESCENE_NODE_TYPE type, ISceneNode *parent=0);
    virtual ISceneNode *addSceneNode (const c8 *typeName, ISceneNode *parent=0);
    virtual u32 getCreatableSceneNodeTypeCount() const;
    virtual ESCENE_NODE_TYPE getCreateableSceneNodeType (u32 idx) const;
    virtual const c8 *getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const;
    virtual const c8 *getCreateableSceneNodeTypeName(u32 idx) const;
};

#endif

