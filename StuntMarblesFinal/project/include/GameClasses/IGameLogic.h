#ifndef _I_GAME_LOGIC
  #define _I_GAME_LOGIC

  #include <irrlicht.h>
  #include <irrOde.h>

enum eGameLogicState {
  eGameStateStarting=0,
  eGameStateReady1,
  eGameStateReady2,
  eGameStateCountdown1,
  eGameStateCountdown2,
  eGameStateGo,
  eGameStatePlay,
  eGameStateGameOver1,
  eGameStateGameOver2,
  eGameStateLapList,
  eGameStateFinished,
  eGameStateWaitingPlayers,
  eGameStateInitGame
};

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CGame;
class CIrrCC;
class CRaceInfo;
class CIrrOdeNetAdapter;

/**
 * @class IGameLogic
 * This is an interface that needs to be implemented to get game logic working. The idea behind this is to implement
 * it once for the "normal" playing (aka "single player" or "splitscreen") and make another implementation for network gaming
 * @author Christian Keimel / dustbin::games
 */
class IGameLogic {
  public:
    virtual ~IGameLogic() { }

    virtual void setDevice(IrrlichtDevice *pDevice)=0;

		/**
		 * Set the triangle selector. This is important for the automatic camera
		 * @param pSelector the new triangle selector
		 */
    virtual void setTriangleSelector(IMetaTriangleSelector *pSelector)=0;

		/**
		 * Set the game object. The game object is responsible for displaying GUI stuff of the race. GUI elements of the players
		 * are displayed somewhere else.
		 * @param pGame the CGame object
		 */
    virtual void setGame(CGame *pGame)=0;

		/**
		 * Set the timer
		 * @param pTimer the timer
		 */
    virtual void setTimer(ITimer *pTimer)=0;

		/**
		 * respawn a marble
		 * @param pMarble the marble to respawn
		 * @param pCam the camera of the marble
		 * @param idx the indes of the respawn node to use
		 */
    virtual void respawn(CIrrOdeBody *pMarble, ICameraSceneNode *pCam, u32 idx)=0;

		/**
		 * Init the game logic
		 */
    virtual void init()=0;

		/**
		 * Set race info
		 * @param pInfo the race info
		 */
    virtual void setRaceInfo(CRaceInfo *pInfo)=0;

		/**
		 * get race info
		 * @return the race info
		 */
    virtual CRaceInfo *getRaceInfo()=0;

		/**
		 * get the current game state
		 * @return the current game state
		 */
    virtual u32 getGameState()=0;

		/**
		 * set a new game state
		 * @param iState the new game state
		 */
    virtual void setGameState(u32 iState)=0;

		/**
		 * get the hiScore file
		 * @return the hiScore file
		 */
    virtual const wchar_t *getHiScoreFile()=0;

		/**
		 * This method need to be called once per frame
		 */
    virtual void update()=0;

    /**
     * Set the net adapter. This is needed for net game servers.
     * @param pAdapter the net adapter
     */
    virtual void setNetAdapter(CIrrOdeNetAdapter *pAdapter)=0;

    virtual void quitState()=0;

    virtual void resetPlayerObject()=0;

    virtual void saveGhost() { }

    virtual void recordGhost(bool b) { }

    virtual void setNetbook(bool b) { }
};

#endif
