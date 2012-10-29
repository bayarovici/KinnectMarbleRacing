#ifndef _I_STATE
  #define _I_STATE

  #include <irrlicht.h>
  #include <irrKlang.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace irrklang;

class CStateMachine;

/**
 * This is the interface for the game states
 * @author Christian Keimel / dustbin::games
 */
class IState {
  protected:
    IrrlichtDevice  *m_pDevice;		/**< the Irrlicht device */
    IVideoDriver    *m_pDriver;		/**< the Irrlicht video driver */
    ISceneManager   *m_pSmgr;			/**< the Irrlicht scene manager */
    IGUIEnvironment *m_pGuienv;		/**< the Irrlicht GUI environment */

    CStateMachine *m_pStateMachine;		/**< the state machine this state belongs to */

    u32 m_iNum;		/**< the index of this state */

  public:
    IState(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) {
      m_pDevice=pDevice;
      m_pDriver=m_pDevice->getVideoDriver();
      m_pSmgr  =m_pDevice->getSceneManager();
      m_pGuienv=m_pDevice->getGUIEnvironment();

      m_pStateMachine=pStateMachine;
    }
    virtual ~IState() { }

		/**
		 * This method is called by the state machine on state activation. Must be implemented in subclass
		 * @param pPrevious the previously active state
		 */
    virtual void activate(IState *pPrevious)=0;

		/**
		 * This method is called by the state machine on state deactivation. Must be implemented in subclass
		 * @param pNext the next active state
		 */
    virtual void deactivate(IState *pNext)=0;

		/**
		 * This method is called by the state manager on each from.
		 * @return "0" is no state change is wanted, "index of new state-1" to switch to another state, out of bounds index to quit program
		 */
    virtual u32 update()=0;

		/**
		 * Sets the index of this state. Called by state manager
		 * @param iNum the index of this state
		 */
    void setNum(u32 iNum) { m_iNum=iNum; }

		/**
		 * get the index of this state
		 * @return the index of this state
		 */
    u32 getNum() { return m_iNum; }
};
#endif
