#ifndef _C_IRR_ODE_REPLAYER
  #define _C_IRR_ODE_REPLAYER

  #include <CSerializer.h>
  #include <CShadowManager.h>
  #include <irrlicht.h>
  #include <irrklang.h>
  #include <event/IIrrOdeEvent.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;
using namespace irrklang;

/**
 * The player's states
 * @author Christian Keimel / dustbin::games
 */
enum ERePlayerStates {
  _E_INIT,			/**< player is initializing */
  _E_PLAYING,		/**< player is playing */
  _E_STOPPED,   /**< player is stopped */
  _E_FINISHED		/**< player finished playing */
};

/**
 * This interface can be implemented to handle game specific messages
 * @author Christian Keimel / dustbin::games
 */
class IMessageHandler {
  public:
	  /**
		 * This method is called if a game specific message is found in a replay.
		 * @param pMessage the message to be handled
		 */
    virtual void handleMessage(IIrrOdeEvent *pMessage)=0;
};

/**
 * This class is designed to replay files recorded with the IrrOdeRecorder
 * @author Christian Keimel / dustbin::games
 */
class CIrrOdeRePlayer : public CManagedShadowProgress {
  protected:
    ITimer *m_pTimer;						/**< the timer */
    IrrlichtDevice *m_pDevice;	/**< the irrlicht device */
    stringc m_sAppName,					/**< the name from which the currently played recording was recorded */
            m_sSceneFile;				/**< the scene file of the current replay */

    list<IIrrOdeEvent *> m_lMessages;					/**< this list holds all messages of a replay */
    list<IIrrOdeEvent *>::Iterator m_itMsg;		/**< iterator for the messages */
    u32 m_iLastTime,											    /**< the last frame's timestamp */
        m_iCurTime,												    /**< the current frame's timestamp */
        m_iState,													    /**< the state of the replayer */
        m_iStepSize;                          /**< the world's stepsize */
    bool m_bValidReplay,									    /**< is the loaded replay a valid IrrOdeRecorder file? */
         m_bFollowCam,                        /**< is the camera in follow cam mode? */
         m_bPaused;                           /**< is the replay paused? */

    ISoundEngine *m_pSndEngine;						/**< the sound engine */
    CSerializer m_cSerializer;					  /**< the serializer for the messages */
    IGUIStaticText *m_pLoadingProgress;		/**< the static text for showing the shadow generation progress */

    void deleteAllMessages();		/**< this method clears the list of messages */

  public:
    CIrrOdeRePlayer(IrrlichtDevice *pDevice);
    virtual ~CIrrOdeRePlayer();

		/**
		 * Load a replay. The working directory name must be the same as in the recorded replay, i.e. of the
		 * program that did the recording. Use isValidReplay to check if the loaded replay is a valid replay.
		 * @param sFile filename of the replay to load
		 * @see isValidReplay
		 */
    void loadReplay(const c8 *sFile);

		/**
		 * Initialize the replayer
		 */
    void init();

		/**
		 * Call this method every frame. The playing is done here
		 * @return true if a simulation step was done
		 */
    bool update();

		/**
		 * With this method you can scan a loaded replay. All messages of the replay will be passed to pHandler
		 * @param pHandler the receiver of the replay's messages
		 */
    void scanReplay(IMessageHandler *pHandler);

		/**
		 * find out if the replay is valid
		 * @return "true" if the replay is valid, "false" otherwise
		 */
    bool isValidReplay();

		/**
		 * get the name of the application that did the recording
		 * @return name of the application that did the recording
		 */
    const char *getAppName();

		/**
		 * this method can be overloaded by a game-specific player. It is called when the replay is started
		 */
    virtual void startReplay() { }

		/**
		 * This method can be overloaded by a game-specific player. It is called on every simulation step
		 */
    virtual void step(u32 iStepNo) { }

		/**
		 * This method can be overloaded by a game-specific player. It is called when the replay has finished
		 */
    virtual void replayFinished() { }

		/**
		 * This is the method that is called by CShadowManager after each initialized shadow
		 * @param iDone number of the shadows that have been created
		 * @param iTotal total number of shadows in the scene
		 */
    virtual void progress(u32 iDone, u32 iTotal);

		/**
		 * Use this method to set the shadow progress text element. Will be updated with a percent value of the shadow creation
		 * @param pProgress the static text element to be updated
		 */
    void setLoadingProgressElement(IGUIStaticText *pProgress);

    void start();
    void stop();
    bool isStarted();

    void setIsPaused(bool b) { m_bPaused=b; }
    bool isPaused() { return m_bPaused; }
};

#endif
