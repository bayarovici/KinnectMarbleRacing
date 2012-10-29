#ifndef _C_MARBLES2_RECORDER
  #define _C_MARBLES2_RECORDER

  #include <IrrOdeNet/CMarbles2WorldObserver.h>
  #include <event/IIrrOdeEvent.h>
  #include <CMessage.h>
  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CMarbles2Recorder : public IIrrOdeWorldListener {
  protected:
    IrrlichtDevice *m_pDevice;

    c8 m_sFileName[0xFF],							/**< filename of the playback */
       m_sLevelName[0xFF];						/**< filename of the scene file */

    list<CMessage *> m_lMessages;

  public:
    CMarbles2Recorder(const c8 *sFileName, const c8 *sLevelName, const c8 *sAppName, IrrlichtDevice *pDevice);
    virtual ~CMarbles2Recorder();
    virtual void worldChange(IIrrOdeEvent *pMsg);

    void startRecording();		/**< start recording */
    void stopRecording();			/**< stop recording */
    void save();
};

#endif
