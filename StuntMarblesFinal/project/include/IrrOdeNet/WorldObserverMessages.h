#ifndef _WORLD_OBSERVER_MESSAGES
  #define _WORLD_OBSERVER_MESSAGES

  #include <event/IIrrOdeEvent.h>

enum WorldObserverMessages {
  eMessageIdentify=irr::ode::eIrrOdeEventUser,
  eMessagePlayer,
  eMessageInit,
  eMessageCpInfo,
  eNetServerMessage,
  eNetClientMessage,
  eNetControlMessage
};

#endif
