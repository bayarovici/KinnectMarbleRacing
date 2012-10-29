  #include <IrrOdeNet/CMarbleMessageFactory.h>
  #include <IrrOdeNet/CEventControlMessage.h>
  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CEventIdentify.h>
  #include <IrrOdeNet/CEventCpInfo.h>
  #include <IrrOdeNet/CEventPlayer.h>
  #include <IrrOdeNet/CEventInit.h>

IIrrOdeEvent *CMarbleMessageFactory::createMessage(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iCode=pData->getU16();

  switch(iCode) {
    case eMessageIdentify  : return new CEventIdentify      (pData); break;
    case eMessagePlayer    : return new CEventPlayer        (pData); break;
    case eMessageInit      : return new CEventInit          (pData); break;
    case eMessageCpInfo    : return new CEventCpInfo        (pData); break;
    case eNetClientMessage : return new CEventClientMessage (pData); break;
    case eNetServerMessage : return new CEventServerMessage (pData); break;
    case eNetControlMessage: return new CEventControlMessage(pData); break;
    default: return NULL;
  }
}
