#ifndef _I_MARBLES2_MESSAGE_FACTORY
  #define _I_MARBLES2_MESSAGE_FACTORY

  #include <event/IIrrOdeEvent.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

class CIrrOdeMessageFactory;

class IMessageFactory {
  protected:
    CIrrOdeMessageFactory *m_pFactory;

  public:
    virtual IIrrOdeEvent *createMessage(CSerializer *pData)=0;
    void setMessageFactory(CIrrOdeMessageFactory *pFactory) { m_pFactory=pFactory; }
};

#endif

