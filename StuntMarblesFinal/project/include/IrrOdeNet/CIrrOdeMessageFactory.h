#ifndef _C_MARBLES2_MESSAGE_FACTORY
  #define _C_MARBLES2_MESSAGE_FACTORY

  #include <IrrOdeNet/IMessageFactory.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CIrrOdeDefaultMessageFactory : public IMessageFactory {
  protected:

  public:
    CIrrOdeDefaultMessageFactory();
    virtual IIrrOdeEvent *createMessage(CSerializer *pData);
};

class CIrrOdeMessageFactory {
  protected:
    ISceneManager *m_pSmgr;
    CIrrOdeDefaultMessageFactory *m_pDefaultFact;
    list<IMessageFactory *> m_lFactories;
    CIrrOdeMessageFactory();
    ~CIrrOdeMessageFactory();

  public:
    static CIrrOdeMessageFactory *getSharedMessageFactory();
    IIrrOdeEvent *createMessage(CSerializer *pData);
    void registerMessageFactory(IMessageFactory *p);
    void setSceneManager(ISceneManager *pSmgr);
    ISceneManager *getSceneManager() { return m_pSmgr; }
};

#endif
