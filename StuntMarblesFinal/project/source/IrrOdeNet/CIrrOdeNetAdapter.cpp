  #include <IrrOdeNet/CIrrOdeMessageFactory.h>
  #include <IrrOdeNet/CEventControlMessage.h>
  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/CEventClientMessage.h>
  #include <IrrOdeNet/CIrrOdeNetAdapter.h>
  #include <IrrOdeNet/NetMessages.h>
  #include <CIrrOdeKlangManager.h>
  #include <CIrrOdeManager.h>
  #include <CSerializer.h>
  #include <irrklang.h>

  #include <event/IIrrOdeEvent.h>
  #include <event/CIrrOdeEventBodyRemoved.h>

using namespace net;

CIrrOdeNetAdapter::CIrrOdeNetAdapter() {
  printf("Creating CIrrOdeNetAdapter ... \n");
  m_pNetManager=NULL;
  m_pSmgr=NULL;

  m_bIsServer=false;
  m_bInitialized=false;

  m_iTime=0;
  m_iPing=0;
  m_iLastPing=0;
  m_iLastStep=0;
  m_iMaxClients=64;
  m_iSentCnt=0;
  m_iRecvCnt=0;
  m_iSentSize=0;
  m_iRecvSize=0;
  m_pCommandVerify=NULL;
  printf("Ready.\n");
}

static CIrrOdeNetAdapter *s_pAdapter=NULL;

CIrrOdeNetAdapter *CIrrOdeNetAdapter::getSharedInstance() {
  printf("\nrequesting shared CIrrOdeNetAdapter...\n");
  if (s_pAdapter==NULL) {
    printf("\tshared instance==NULL ... creating ...\n");
    s_pAdapter=new CIrrOdeNetAdapter();
  }
  printf("Ready.\n\n");
  return s_pAdapter;
}

void CIrrOdeNetAdapter::setTimer(ITimer *pTimer) {
  m_pTimer=pTimer;
}

void CIrrOdeNetAdapter::setSceneManager(ISceneManager *pSmgr) {
  m_pSmgr=pSmgr;
}

bool CIrrOdeNetAdapter::isValid(u16 iPlayerId, u16 iCommand, u16 iBodyId) {
  return m_pCommandVerify==NULL || m_pCommandVerify->validCommand(iPlayerId,iCommand,iBodyId);
}

void CIrrOdeNetAdapter::setPing(u16 iPlayerId, u16 iPing) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++)
    if ((*it)->getPlayerId()==iPlayerId) (*it)->setPing(iPing);
}

void CIrrOdeNetAdapter::handlePacket(SInPacket& packet) {
  CSerializer cBuffer;
  cBuffer.setBuffer(packet.getConstData(),packet.getSize());

  m_iRecvCnt++;
  m_iRecvSize+=packet.getSize();

  cBuffer.resetBufferPos();

  CIrrOdeMessageFactory *pFactory=CIrrOdeMessageFactory::getSharedMessageFactory();
  IIrrOdeEvent *pEvent=pFactory->createMessage(&cBuffer);
  if (pEvent) {
    switch(pEvent->getType()) {
      case eNetClientMessage: {
          CEventClientMessage *e=(CEventClientMessage *)pEvent;
          e->setPlayerId(packet.getPlayerId());
        }
        break;

      case eNetControlMessage:
        if (m_pSmgr) {
          CEventControlMessage *e=(CEventControlMessage *)pEvent;

          u16 iCode=e->getCode();
          s32 iBodyId=e->getBodyId();
          CIrrOdeBody *pBody=(CIrrOdeBody *)m_pSmgr->getSceneNodeFromId(iBodyId);
          if (pBody && isValid(packet.getPlayerId(),iCode,iBodyId)) {
            switch (iCode) {
              case eNetAddTorque         : pBody->addTorque         (e->getVector()); break;
              case eNetSetPosition       : pBody->setPosition       (e->getVector()); break;
              case eNetSetLinearVelocity : pBody->setLinearVelocity (e->getVector()); break;
              case eNetSetAngularVelocity: pBody->setAngularVelocity(e->getVector()); break;
              case eNetSetAngularDamping : pBody->setAngularDamping (e->getFloat ()); break;
            }
          }
        }
        break;
    }
    CIrrOdeManager::getSharedInstance()->getQueue()->postEvent(pEvent);
  }
  distribute(&cBuffer,packet.getPlayerId());
}

void CIrrOdeNetAdapter::onConnect(const u16 playerId) {
  if (m_bIsServer) {
    printf("CIrrOdeNetAdapter::onConnect (%i) | %i\n",playerId,m_iMaxClients);

    if (m_lPeers.getSize()<m_iMaxClients-1) {
      CIrrOdePeer *pPeer=new CIrrOdePeer(playerId);
      m_lPeers.push_back(pPeer);
      m_bInitialized=true;
      distributeConnectionChange(playerId,true);
    }
    else {
      CEventServerMessage cMsg(eMessageServerFull);
      sendPacket(&cMsg,playerId);
      m_pNetManager->update();
      //m_pNetManager->kickClient(playerId,false);
      //m_pNetManager->update();
    }
  }
}

void CIrrOdeNetAdapter::onDisconnect(const u16 playerId) {
  if (m_bIsServer) {
    printf("CIrrOdeNetAdapter::onDisconnect (%i)\n",playerId);

    distributeConnectionChange(playerId,false);

    list<CIrrOdePeer *>::Iterator itPeer;
    for (itPeer=m_lPeers.begin(); itPeer!=m_lPeers.end(); itPeer++) {
      CIrrOdePeer *pPeer=*itPeer;
      if (pPeer->getPlayerId()==playerId) {
        m_lPeers.erase(itPeer);
        delete pPeer;
        return;
      }
    }
  }
}

void CIrrOdeNetAdapter::update() {
  if (m_pNetManager!=NULL && m_pNetManager->getConnectionStatus()!=EICS_ESTABLISHED) {
    printf("connection problem!\n");
  }
  if (m_pNetManager) {
    m_pNetManager->update();
    m_iTime=m_pTimer->getRealTime();
    if (m_iTime-m_iLastPing>1000) {
      m_iLastPing=m_iTime;
      if (!m_bIsServer) {
        CEventClientMessage cMsg(eMessageClientPing);
        m_iPing=m_pNetManager->getPing();
        cMsg.setData1(m_iPing);
        sendPacket(&cMsg);
      }
    }

    if (m_iTime-m_iLastStep>16) {
      m_iLastStep=m_iTime;
    }
  }
  else printf("m_pNetManager==NULL\n");
}

bool CIrrOdeNetAdapter::isInitialized() {
  return m_pNetManager!=NULL;
}

void CIrrOdeNetAdapter::sendPacket(IIrrOdeEvent *pEvent, u16 iPlayerId) {
  if (!m_pNetManager) return;
  if (pEvent==NULL || pEvent->serialize()==NULL) return;

  CSerializer *pBuffer=pEvent->serialize();

  m_iSentCnt++;
  m_iSentSize+=pBuffer->getBufferSize();

  if (m_pNetManager->getConnectionStatus()!=EICS_ESTABLISHED) {
    //printf("connection problem!\n");
  }

  pBuffer->resetBufferPos();
  SOutPacket cPacket(pBuffer->getBuffer(),pBuffer->getBufferSize());
  m_pNetManager->sendOutPacket(cPacket,iPlayerId);

  #ifdef _TRACE_SENT_MESSAGES
    u16 i=pEvent->getType();
    printf("message of sent packet: %i to %i\n",i,iPlayerId);
  #endif
}

void CIrrOdeNetAdapter::sendPacket(IIrrOdeEvent *pEvent) {
  if (!m_pNetManager) return;
  if (pEvent==NULL || pEvent->serialize()==NULL) return;

  CSerializer *pBuffer=pEvent->serialize();

  m_iSentCnt++;
  m_iSentSize+=pBuffer->getBufferSize();

  if (m_pNetManager->getConnectionStatus()!=EICS_ESTABLISHED) {
    //printf("connection problem!\n");
  }

  pBuffer->resetBufferPos();
  SOutPacket cPacket(pBuffer->getBuffer(),pBuffer->getBufferSize());
  m_pNetManager->sendOutPacket(cPacket);

  #ifdef _TRACE_SENT_MESSAGES
    pBuffer->resetBufferPos();
    u8 i=pBuffer->getU8();
    printf("message of sent packet: %i\n",i);
  #endif
}

void CIrrOdeNetAdapter::sendPacketUnreliable(CSerializer *pBuffer, u16 iPlayerId) {
  if (!m_pNetManager) return;

  m_iSentCnt++;
  m_iSentSize+=pBuffer->getBufferSize();

  if (m_pNetManager->getConnectionStatus()!=EICS_ESTABLISHED) {
    //printf("connection problem!\n");
  }

  pBuffer->resetBufferPos();
  SOutPacket cPacket(pBuffer->getBuffer(),pBuffer->getBufferSize());
  m_pNetManager->sendOutPacketUnreliable(cPacket,iPlayerId);

  #ifdef _TRACE_SENT_MESSAGES
    pBuffer->resetBufferPos();
    u8 i=pBuffer->getU8();
    printf("message of unreliable sent packet: %i to %i\n",i,iPlayerId);
  #endif
}

void CIrrOdeNetAdapter::sendPacketUnreliable(CSerializer *pBuffer) {
  if (!m_pNetManager) return;

  m_iSentCnt++;
  m_iSentSize+=pBuffer->getBufferSize();

  if (m_pNetManager->getConnectionStatus()!=EICS_ESTABLISHED) {
    //printf("connection problem!\n");
  }

  pBuffer->resetBufferPos();
  SOutPacket cPacket(pBuffer->getBuffer(),pBuffer->getBufferSize());
  m_pNetManager->sendOutPacketUnreliable(cPacket);

  #ifdef _TRACE_SENT_MESSAGES
    pBuffer->resetBufferPos();
    u8 i=pBuffer->getU8();
    printf("message of unreliable sent packet: %i\n",i);
  #endif
}

bool CIrrOdeNetAdapter::createServer(u32 iPortNo, u32 iUpBandwidth, u32 iDownBandwidth, u32 iMaxClients) {
  //if (m_pNetManager) delete m_pNetManager;

  SNetParams cParams;
  cParams.maxClients=iMaxClients;
  cParams.downBandwidth=iDownBandwidth;
  cParams.upBandwidth=iUpBandwidth;

  m_iMaxClients=iMaxClients;

  m_pNetManager=createIrrNetServer(this,iPortNo,cParams);
  m_bIsServer=true;
  return m_pNetManager!=NULL && m_pNetManager->getConnectionStatus()!=EICS_FAILED;
}

bool CIrrOdeNetAdapter::createClient(const char *sServer, u32 iPortNo, u32 iUpBandwidth, u32 iDownBandwidth, u32 iTimeout) {
  /*if (m_pNetManager) {
    delete m_pNetManager;
    m_pNetManager=NULL;
  }*/

  SNetParams cParams;
  cParams.connectionTimeout=iTimeout;
  cParams.downBandwidth=iDownBandwidth;
  cParams.upBandwidth=iUpBandwidth;

  m_pNetManager=createIrrNetClient(this,sServer,23005,cParams);
  m_bIsServer=false;
  return m_pNetManager!=NULL && m_pNetManager->getConnectionStatus()!=EICS_FAILED;
}

void CIrrOdeNetAdapter::close() {
  printf("\nclosing CIrrOdeNetAdapter...\n");
  if (m_bInitialized)
    while (m_lPeers.getSize()>0) {
      list<CIrrOdePeer *>::Iterator it=m_lPeers.begin();
      CIrrOdePeer *peer=*it;
      m_lPeers.erase(it);
      delete peer;
    }

  if (m_pNetManager) {
    delete m_pNetManager;
    delete s_pAdapter;
    s_pAdapter=NULL;
  }
  printf("Ready.\n");
}

void CIrrOdeNetAdapter::setPeerData(u16 iPlayerId, void *pData) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pPeer=*it;
    if (pPeer->getPlayerId()==iPlayerId) {
      pPeer->setData(pData);
      return;
    }
  }
}

void CIrrOdeNetAdapter::clientAddPeerData(u16 iPlayerId, void *pData) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pOldPeer=*it;
    if (pOldPeer->getPlayerId()==iPlayerId) return;
  }

  CIrrOdePeer *pPeer=new CIrrOdePeer(iPlayerId);
  pPeer->setData(pData);
  m_lPeers.push_back(pPeer);
  m_bInitialized=true;
}

void *CIrrOdeNetAdapter::getPeerData(u16 iPlayerId) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pPeer=*it;
    if (pPeer->getPlayerId()==iPlayerId) {
      return pPeer->getData();
    }
  }
  return NULL;
}

void *CIrrOdeNetAdapter::getPeerDataByIndex(u16 iIdx) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    if (iIdx==0) {
      CIrrOdePeer *pPeer=*it;
      return pPeer->getData();
    }
    else iIdx--;
  }
  return NULL;
}

u16 CIrrOdeNetAdapter::getIdOfPeerAtIndex(u16 iIdx) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    if (iIdx==0) {
      CIrrOdePeer *pPeer=*it;
      return pPeer->getPlayerId();
    }
    else iIdx--;
  }
  return 0;
}

u16 CIrrOdeNetAdapter::getIndexOfPeer(u16 iPlayerId) {
  list<CIrrOdePeer *>::Iterator it;
  u16 iPos=0;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pPeer=*it;
    if (pPeer->getPlayerId()==iPlayerId) return iPos;
    iPos++;
  }
  return iPos;
}

u16 CIrrOdeNetAdapter::getPing(u16 iPlayerId) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pPeer=*it;
    if (pPeer->getPlayerId()==iPlayerId) {
      return pPeer->getPing();
    }
  }
  return 0;
}

u32 CIrrOdeNetAdapter::getPeerCount() {
  return m_pNetManager!=NULL?m_lPeers.getSize():0;
}

s32 CIrrOdeNetAdapter::getNumOfPeer(u16 iPlayerId) {
  s32 iRet=0;
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    CIrrOdePeer *pPeer=*it;
    if (pPeer->getPlayerId()==iPlayerId) return iRet;
    iRet++;
  }
  return -1;
}

u32 CIrrOdeNetAdapter::getPing() {
  return m_iPing;
}

void CIrrOdeNetAdapter::worldChange(IIrrOdeEvent *pMsg) {
  switch (pMsg->getType()) {
    case eIrrOdeEventNoEvent:
    case eIrrOdeEventBeforeStep:
    case eIrrOdeEventStep:
    case eIrrOdeEventActivationChanged:
    case eIrrOdeEventBodyCreated:
      break;

    default:
      sendPacket(pMsg);
      break;
  }
}

void CIrrOdeNetAdapter::startWorldUpdate() {
  printf("starting world update...\n");
  CMarbles2WorldObserver::getSharedInstance()->addListener(this);
}

void CIrrOdeNetAdapter::stopWorldUpdate() {
  printf("stopping world update...\n");
  CMarbles2WorldObserver::getSharedInstance()->removeListener(this);
}

void CIrrOdeNetAdapter::removeFromPhysics(ISceneNode *pNode) {
  if (pNode->getType()==IRR_ODE_BODY_ID) {
    CIrrOdeBody *p=reinterpret_cast<CIrrOdeBody *>(pNode);
    p->removeFromPhysics();
  }
}

u16 CIrrOdeNetAdapter::getPlayerIdByIndex(u32 iIdx) {
  list<CIrrOdePeer *>::Iterator it;
  for (it=m_lPeers.begin(); it!=m_lPeers.end(); it++) {
    if (iIdx==0) {
      CIrrOdePeer *pPeer=*it;
      return pPeer->getPlayerId();
    }
    else iIdx--;
  }
  return 0;
}

void CIrrOdeNetAdapter::setCommandVerify(ICommandVerify *pVerify) {
  m_pCommandVerify=pVerify;
}

void CIrrOdeNetAdapter::setNetIterations(u32 iIterations) {
  if (m_pNetManager) m_pNetManager->setNetIterations(iIterations);
}

void CIrrOdeNetAdapter::setGlobalPacketRelay(bool bRelay) {
  if (m_pNetManager) m_pNetManager->setGlobalPacketRelay(bRelay);
}

u32 CIrrOdeNetAdapter::getIpAddress() {
  return 0;
  //return m_pNetManager!=NULL?m_pNetManager->getAddress():0;
}
