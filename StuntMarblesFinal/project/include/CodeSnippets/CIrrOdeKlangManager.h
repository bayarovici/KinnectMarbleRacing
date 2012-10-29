#ifndef _C_IRR_ODE_KLANG_MANAGER
  #define _C_IRR_ODE_KLANG_MANAGER

  #include <irrlicht.h>
  #include <IrrOde.h>
	#ifndef _IRR_ODE_KLANG_PLUGIN
		#include <irrklang.h>
	#endif

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;
#ifndef _IRR_ODE_KLANG_PLUGIN
  using namespace irrklang;
#endif

class CIrrOdeKlangNode;

/**
 * @class CIrrOdeKlangManager
 * This class manages all IrrOdeKlangNodes found in a scene
 * @see CIrrOdeKlangNode
 * @author Christian Keimel / dustbin::games
 */
class CIrrOdeKlangManager : public IIrrOdeEventListener {
  protected:
		#ifndef _IRR_ODE_KLANG_PLUGIN
		  ISoundEngine *m_pSndEngine;
		#endif

		vector3df m_vVel[2];

		list<CIrrOdeKlangNode *> m_lCollisionNodes,		/**< nodes that do collision sound */
                             m_lLinVelNodes,			/**< nodes that do linear velocity sound */
                             m_lAngVelNodes,			/**< nodes that do angular velocity sounds */
                             m_lOtherNodes;				/**< nodes that do other sounds */

    ICameraSceneNode *m_pListener[2];		/**< The listeners. As Stunt Marble Racers has a 2 player splitscreen mode there is an array of listeners here */
    CIrrOdeKlangManager();

  public:
	  /**
		 * Get the shared singleton instance of the klang manager
		 * @return the shared singleton instance
		 */
    static CIrrOdeKlangManager *getSharedInstance();

    void activate();		/**< activate the manager */
    void deactivate();	/**< deactivate the manager and stop all sounds */

    virtual ~CIrrOdeKlangManager() { }

		/**
		 * add an IrrOdeKlangNode. This method is called by CIrrOdeKlangNode's constructor
		 * @param pNode the node to be added
		 */
		void addOdeKlangNode(CIrrOdeKlangNode *pNode);

		/**
		 * remove all nodes
		 */
		void removeAllNodes();

		/**
		 * set a listener
		 * @param pListener the listener to set
		 * @param iIdx the position in the listener array (i.e. "0" or "1");
		 */
		void setListener(ICameraSceneNode *pListener, u32 iIdx);

		/**
		 * Set the velocity of a listener
		 * @param vVel the velocity vector of the listener
		 * @param iIdx the index (0,1) of the listener
		 */
		void setListenerVelocity(vector3df vVel, u32 iIdx);

		#ifndef _IRR_ODE_KLANG_PLUGIN
		  /**
			 * Set the sound engine
			 * @param pSndEngine the sound engine
			 */
		  void setSoundEngine(ISoundEngine *pSndEngine);
		#endif

		/**
		 * IrrODE event receiver
		 * @param pEvent the event to be handled
		 */
    virtual bool onEvent(IIrrOdeEvent *pEvent);

		/**
		 * IrrODE event chack
		 * @param pEvent the event to be handled
		 * @return "true" if this class handles such events, "false" otherwise
		 */
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);
};

#endif


