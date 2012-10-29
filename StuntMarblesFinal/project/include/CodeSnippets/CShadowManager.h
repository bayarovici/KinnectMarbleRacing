#ifndef _C_SHADOW_MANAGER
  #define _C_SHADOW_MANAGER

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CManagedShadow;

/**
 * @class CManagedShadowProgress
 * This interface can be implemented in order to show the progress of shadow creation to the user
 * @author Christian Keimel / dustbin::games
 */
class CManagedShadowProgress {
  public:
	  /**
		 * This callback is called after each created shadow
		 * @param iDone number of the shadows that were created
		 * @param iTotal total number of shadows in the scene
		 */
    virtual void progress(u32 iDone, u32 iTotal)=0;
};

/**
 * @class CShadowManager
 * This is the manager that takes care of all managed shadows in the scene.
 * @see CManagedShadow
 * @author Christian Keimel / dustbin::games
 */
class CShadowManager {
  protected:
    list <CManagedShadow *> m_lShadows;		/**< list of all managed shadows in the scene */
    u32 m_iLevel;													/**< the current shadow level */
    CManagedShadowProgress *m_pProgress;	/**< a callback that shows updates of the shadow creation */

    CShadowManager();		/**< protected constructor */

  public:
	  /**
		 * Use this method to get the shared singleton instance of the shadow manager
		 * @return the shared singled instance of the shadow manager
		 */
    static CShadowManager *getSharedInstance();

		/**
		 * This method is called by the constructor of CManagedShadow to add the shadow to the list
		 * @param pNew the managed shadow to be added
		 */
    void addShadow(CManagedShadow *pNew);

		/**
		 * Set the shadow level. All shadows with a level <= the new level are shown, all others are hidden
		 * @param iLevel the new shadow level
		 */
    void setLevel(u32 iLevel);

		/**
		 * This method updates all shadows. Takes quite a while on first call to create all shadows.
		 */
		void update();

		/**
		 * Get the current shadow level
		 * @return the current shadow level
		 */
		u32 getLevel();

		/**
		 * Set the progress callback that is called after creation of each shadow
		 * @param pProgress the progress callback
		 */
		void setProgressCallback(CManagedShadowProgress *pProgress);

		/**
		 * clear the list of shadows
		 */
		void clear();
};

#endif


