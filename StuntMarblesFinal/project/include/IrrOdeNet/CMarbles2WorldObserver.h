#ifndef _C_MARBLES2_WORLD_OBSERVER
  #define _C_MARBLES2_WORLD_OBSERVER

  #include <IrrOde.h>
  #include <CMessage.h>
  #include <irrlicht.h>
  #include <CSerializer.h>

  #define _FLAG_COLLISION 1

using namespace irr;
using namespace ode;

enum EnumMarbleMessages {
};

class IIrrOdeWorldListener {
  public:
    virtual void worldChange(IIrrOdeEvent *pMsg)=0;
};

class CMarbles2WorldObserver : public IIrrOdeEventListener {
  protected:
    IrrlichtDevice *m_pDevice;
    CMarbles2WorldObserver();

    list<IIrrOdeEvent *> m_lMessages;
    list<IIrrOdeWorldListener *> m_lListeners;

    bool m_bOdeInitialized;
    u32 m_iStep;
    stringc m_sAppName,
            m_sLevel;
    CSerializer m_cSerializer;

    void distributeMessage(IIrrOdeEvent *msg);

  public:
    static CMarbles2WorldObserver *getSharedInstance();   /*!< get a pointer to the singleton instance of this class */

    void setIrrlichtDevice(IrrlichtDevice *pDevice);

    virtual ~CMarbles2WorldObserver();

    void addMessage(IIrrOdeEvent *msg);

    void install(const c8 *sAppName);
    void destall();

    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);

    void addListener(IIrrOdeWorldListener *pListener);
    void removeListener(IIrrOdeWorldListener *pListener);
};

#endif
