#ifndef _C_MANAGED_SHADOW
  #define _C_MANAGED_SHADOW

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CShadowManager;

const int MANAGED_SHADOW_ID=MAKE_IRR_ID('m','g','s','d');
const c8 MANAGED_SHADOW_NAME[0xFF]="CManagedShadow";

/**
 * @class CManagedShadow
 * This class manages Irrlicht shadows. You can attach instances of this class as child of any animated mesh scene node using a plugin
 * for IrrEdit. Depending on the level of shadow they are shown or hidden.
 * @author Christian Keimel / dustbin::games
 */
class CManagedShadow : public ISceneNode {
  private:
    IShadowVolumeSceneNode *m_pShadow;	/**< the shadow */
		ISceneManager *m_pSceneManager;			/**< the scene manager */
    u32 m_iLevel;												/**< this shadow's level */

  public:
    CManagedShadow(ISceneNode *pParent, ISceneManager *pMgr, s32 iId=-1, const vector3df pos=vector3df(0,0,0), const vector3df rot=vector3df(0,0,0), const vector3df scale=vector3df(1,1,1));

    virtual void render() { }		/**< nothing to render */

    virtual const aabbox3df &getBoundingBox() const {
      static aabbox3df aBox;
      return aBox;
    }
		
		/**
		 * Set the level of this shadow. A shadow is shown if it's level is smaller or equal to the level of the shadow manager
		 * @param iLevel the new level of the shadow
		 * @see CShadowManager
		 */
    void setLevel(u32 iLevel);
		
		/**
		 * Get the level of this shadow
		 * @return the level of this shadow
		 */
    u32 getLevel();

    virtual ESCENE_NODE_TYPE getType() const;
		
		/**
		 * Update the shadow depending on the new level, i.e. create or delete the shadow if necessary
		 * @param theLevel the new shadow level
		 */
    void update(u32 theLevel);

    virtual void serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const;
    virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

		virtual ISceneNode *clone(ISceneNode* newParent=0, ISceneManager* newManager=0);
};

#endif
