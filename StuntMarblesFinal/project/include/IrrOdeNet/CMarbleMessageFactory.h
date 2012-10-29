#ifndef _C_MARBLE_MESSAGE_FACTORY
  #define _C_MARBLE_MESSAGE_FACTORY

  #include <IrrOdeNet/IMessageFactory.h>
  #include <CSerializer.h>

class CMarbleMessageFactory : public IMessageFactory {
  public:
    virtual IIrrOdeEvent *createMessage(CSerializer *pData);
};

#endif
