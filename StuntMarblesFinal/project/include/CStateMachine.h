#ifndef _C_STATE_MACHINE
  #define _C_STATE_MACHINE

  #include <irrlicht.h>
  #include <IState.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CMenu;
class CGame;
class CLevel;
class CReplay;
class COptions;
class CHiScore;
class CCredits;
class CNetGame;
class CTournament;
class CReplaySelect;
class CTournamentRace;

/**
 * This is the state machine that takes care of managing all the game's states
 * @author Christian Keimel / dustbin::games
 */
class CStateMachine {
  protected:
    array<IState *> m_aStates;						/**< a list of all states */

    IState *m_pActive;										/**< the active state */

    IrrlichtDevice  *m_pDevice;
    IVideoDriver    *m_pDriver;
    ISceneManager   *m_pSmgr,							/**< the main scene manager */
                    *m_pPreviewManager;		/**< the scene manager for level preview */
    IGUIEnvironment *m_pGuienv;
    IGUIFont        *m_pFont;							/**< the new standard font */

    bool m_bGraphicsChanged,			/**< this flag must be "true" if the graphics mode needs re-initialization, e.g. to chane it */
			   m_bDrawScene;						/**< should the state machine draw the scene, or will the active state take care of that? */

    ISoundEngine *m_pSndEngine;		/**< the sound engine */
    u32 m_iFps;

    CMenu           *m_pMenu;					/**< the "main menu" state */
    CLevel          *m_pLevl;					/**< the "select level" state */
    COptions        *m_pOpts;					/**< the "options" state */
    CHiScore        *m_pHiSc;					/**< the "hiScore" state */
    CCredits        *m_pCred;					/**< the "credits" state */
    CGame           *m_pGame;					/**< the "game" state */
    CTournament     *m_pTournament;		/**< the "tournament" state */
    CTournamentRace *m_pTrnRace;			/**< the "tournament race" state */
    CReplaySelect   *m_pReplaySel;		/**< the "select replay" state */
    CReplay         *m_pReplay;				/**< the "play replay" state */
    CNetGame        *m_pNetGame;      /**< the "netgame" state */

    void addState(IState *pState);

  public:
    CStateMachine();
    virtual ~CStateMachine() { }

		/**
		 * Initialize all states
		 * @param pDevice the Irrlicht device to be used
		 */
    virtual void initStates(IrrlichtDevice *pDevice);

		/**
		 * clear all states
		 */
    virtual void clearStates();

		/**
		 * set the m_bGraphicsChanged flag to "true"
		 * @see m_bGraphicsChanged
		 */
    void graphicsChanged();

		/**
		 * change the m_bDrawScene flag
		 * @see m_bDrawScene
		 */
		void setDrawScene(bool b);

		/**
		 * retrieve the sound engine
		 * @return the sound engine
		 */
		ISoundEngine *getSoundEngine();

	  /**
		 * start and run the state machine
		 * @return unimportant
		 */
    u32 run();

		/**
		 * get the aspect ratio
		 * @return the aspect ratio
		 */
    f32 getAspectRatio();

		/**
		 * get the "preview" scene manager
		 * @return the preview scene manager
		 */
    ISceneManager *getPreviewSceneManager();

    /**
     * get the framerate
     * @return the framerate
     */
    u32 getFps();
};
#endif
