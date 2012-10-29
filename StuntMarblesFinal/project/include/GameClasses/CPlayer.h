#ifndef _C_PLAYER
  #define _C_PLAYER

  #include <irrlicht.h>
  #include <irrklang.h>
  #include <irrOde.h>
#include <KinectJoystickGUI.h>

#include <KinectFramework.h>
using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;
using namespace irrklang;

class CIrrCC;
class CGame;
class CGameLogic;
class CIrrOdeKlangNode;
class IIrrOdeController;
class CMarbleFollowCameraAnimator;

/**
 * @class CPlayer
 * This class handles all player-specific stuff. It controls the marble, the camera and the player specific GUI elements
 * @author Christian Keimel / dustbin::games
 */
class CPlayer {
  private:
    IrrlichtDevice *m_pDevice;
    ISoundEngine *m_pSndEngine;

    IIrrOdeController *m_pController;

    ICameraSceneNode *m_pCam;				/**< the player's camera */
    IGUIFont *m_pBigFont;						/**< the big font, e.g. for playername */
    IGUIImage *m_pCamMode,					/**< the "camera mode" GUI element */
              *m_pLap,							/**< the "lap no." GUI element */
              *m_pFinished,					/**< the "finished" GUI element */
              *m_pSpeed;						/**< the "speed" GUI element */
    ITimer *m_pTimer;
    IGUIStaticText *m_pInfo,				/**< the info text GUI element */
                   *m_pCpTime,			/**< the checkpoint time text GUI element */
                   *m_pName;				/**< the player name GUI element */
    IGUITab *m_pCpInfo,							/**< the checkpoint info GUI tab */
            *m_pLapTab;							/**< the lap info GUI tab */

    ITexture *m_pCamFree,		/**< the "free camera" image */
             *m_pCamRace,		/**< the "race camera" image */
             *m_pCpOk,			/**< the "checkpoint OK" image */
             *m_pCpNok;			/**< the "checkpoint not OK" image */

    array<IGUIImage *> m_aCpState,		/**< GUI elements to show checkpoint states */
                       m_aLapNo;			/**< GUI elements to show lap number */
    array<ITexture *> m_aNumbers;			/**< images with the numbers for the lap */

    CIrrOdeBody *m_pMarble;		/**< the player's marble */
    CIrrCC *m_pConfigCtrl;		/**< the player's control set */
    CGame *m_pGame;						/**< the Game */

    u32 m_iNum,						/**< the player's index */
        m_iInfoTime,			/**< the timestamp when the info tab was shown */
        m_iRespawnTime;		/**< countdown to respawn */
    f32 m_fPan,						/**< panorama of sound for 2 player splitscreen mode */
        m_fDamping;       /**< the angular damping the player is applying */

    bool m_bActive,			/**< is the player active? */
         m_bFinished,		/**< has the player finished the race? */
         m_bInitOK,			/**< was the initialization OK? */
         m_bNetClient,  /**< is this player a net client? */
         m_bCamStepped; /**< a net client steps cam whenver the marble is moved. No need to do this on the normal step */

    ISceneNode *findCamera(ISceneNode *pParent);		/**< find the player's camera scene node */
    CMarbleFollowCameraAnimator *m_pAnimator;				/**< the camera animator */
	CKinectJoystickGUI* m_kinectJoystick;

  public:
    CPlayer(u32 iNum, u32 iLocalNum, IrrlichtDevice *pDevice, f32 fCamAspectRatio, CGame *pGame, f32 fPan, bool bNetClient);
    CPlayer(ISceneNode *pNode, IrrlichtDevice *pDevice, f32 fAspectRatio, CGame *pGame);
    ~CPlayer();

    void init(IrrlichtDevice *pDevice, CGame *pGame);

    ICameraSceneNode *getCamera();		/**< get the camera */
    ISceneNode *getMarble();					/**< get the marble */

    void setActive(bool b);		/**< set the "active" flag */
    bool isActive();					/**< get the "active" flag */

    void step(bool bDoCamStep=true);		/**< IrrODE step callback */

    void toggleCheckpoint(u32 iNum, bool bPlaySound);		/**< toggle a checkpoint */
    void resetCheckpoints();														/**< reset all checkpoints */

		/**
		 * Show checkpoint info
		 * @param sText text to show
		 * @param iTime section time
		 * @param bFastest was this the fastest lap?
		 * @param bLastLap was this the last lap?
		 */
    void showCheckpointInfo(const wchar_t *sText, u32 iTime, bool bFastest, bool bLastLap);

    void setInfoText(const wchar_t *sText);		/**< change to info text */
    void setLapNo(u32 iLap, u32 iMaxLap);			/**< update the shown lap number */
    void updateGUI(u32 iTime);								/**< update the player's GUI */

		/**
		 * create the player's GUI
		 * @param bHSplit horizontal splitscreen?
		 * @param sName name of the player
		 * @param iCheckpoints number of checkpoints
		 */
    void createPlayerGUI(bool bHSplit, const wchar_t *sName, u32 iCheckpoints);

    void setTimer(ITimer *pTimer);	/**< set the timer */

    void setFinished(bool b);		/**< set the "finished" flag */
    bool hasFinished();					/**< get the "finished" flag */

    void clearGUI();					/**< clear the player's GUI */
    bool initialized();				/**< was the player initialized? */

    vector3df getPosition();	/**< get the position of the player's marble */

    void setController(IIrrOdeController *pController);
    IIrrOdeController *getController();
    void camStep();
};

#endif
