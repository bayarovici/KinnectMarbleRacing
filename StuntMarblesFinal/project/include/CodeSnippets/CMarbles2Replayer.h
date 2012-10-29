#ifndef _C_MARBLES_2_REPLAYER
  #define _C_MARBLES_2_REPLAYER

  #include <IrrOdeNet/CIrrOdeRePlayer.h>
  #include <event/IIrrOdeEventListener.h>

/**
 * @class CMarbles2Replayer
 * This class adds game specific GUI elements to the CIrrOdeRePlayer
 * @see CIrrOdeRePlayer
 * @author Christian Keimel / dustbin::games
 */
class CMarbles2Replayer : public CIrrOdeRePlayer, public IIrrOdeEventListener {
  protected:
    s32 m_iGoStep;
    CSerializer m_cBuffer;				/**< the serializer */
    IGUIStaticText *m_pStepNo,			/**< the step number GUI element */
                   *m_pCountDown,		/**< the countdown GUI element */
                   *m_pPlayer[4],		/**< the player name GUI elements */
                   *m_pCp[4],				/**< the checkpoint GUI elements */
                   *m_pRt[4],				/**< the racetime GUI elements */
                   *m_pFastest;			/**< the fastest lap GUI elements */

  public:
    CMarbles2Replayer(IrrlichtDevice *pDevice);

    void createGUI();		/**< call this method to create the GUI */

		/**
		 * Start the replay
		 */
    virtual void startReplay();

		/**
		 * This method is called whenever an ODE step is found in the replay file
		 * @param iStepNo the current step
		 */
    virtual void step(u32 iStepNo);

		/**
		 * This method is called when the replay has finished
		 */
    virtual void replayFinished();

    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);
};

#endif
