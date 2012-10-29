  #include <IrrOdeNet/CMarbles2WorldObserver.h>
  #include <IrrOdeNet/CEventIdentify.h>


CMarbles2WorldObserver::CMarbles2WorldObserver() {
  m_bOdeInitialized=false;
  m_iStep=0;
}

CMarbles2WorldObserver::~CMarbles2WorldObserver() {
  while (m_lListeners.getSize()>0) {
    list<IIrrOdeWorldListener *>::Iterator it=m_lListeners.begin();
    m_lListeners.erase(it);
  }

  while (m_lMessages.getSize()>0) {
    list<IIrrOdeEvent *>::Iterator it=m_lMessages.begin();
    IIrrOdeEvent *pMsg=*it;
    m_lMessages.erase(it);
    delete pMsg;
  }
}

void CMarbles2WorldObserver::distributeMessage(IIrrOdeEvent *msg) {
  list<IIrrOdeWorldListener *>::Iterator it;
  for (it=m_lListeners.begin(); it!=m_lListeners.end(); it++) {
    IIrrOdeWorldListener *p=*it;
    p->worldChange(msg);
  }
}

/**
 * Get the shared singleton instance of the manager
 * @return the shared singleton instance of the manager
 */
CMarbles2WorldObserver *CMarbles2WorldObserver::getSharedInstance() {
  static CMarbles2WorldObserver theObserver;
  return &theObserver;
}

void CMarbles2WorldObserver::setIrrlichtDevice(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
}

void CMarbles2WorldObserver::install(const c8 *sAppName) {
  CIrrOdeManager::getSharedInstance()->getQueue()->addEventListener(this);
  m_sAppName=stringc(sAppName);
}

void CMarbles2WorldObserver::destall() {
  CIrrOdeManager::getSharedInstance()->getQueue()->removeEventListener(this);
}

void CMarbles2WorldObserver::addMessage(IIrrOdeEvent *msg) {
  m_lMessages.push_back(msg);
  distributeMessage(msg);
}

bool CMarbles2WorldObserver::onEvent(IIrrOdeEvent *pEvent) {
  switch (pEvent->getType()) {
    case irr::ode::eIrrOdeEventBodyMoved: {
        bool bIn=false;
        CIrrOdeEventBodyMoved *evt=(CIrrOdeEventBodyMoved *)pEvent;

        if (!evt->getBody()) return false;
        list<IIrrOdeEvent *>::Iterator it;
        for (it=m_lMessages.begin(); it!=m_lMessages.end(); it++) {
          IIrrOdeEvent *myMsg=*it;
          if (myMsg->getType()==irr::ode::eIrrOdeEventBodyMoved) {
            CIrrOdeEventBodyMoved *p=(CIrrOdeEventBodyMoved *)myMsg;
            if (p->getBody()->getID()==evt->getBody()->getID()) {
              p->merge(evt);
              bIn=true;
              break;
            }
          }
        }

        if (!bIn) {
          evt=new CIrrOdeEventBodyMoved(pEvent);
          m_lMessages.push_back(evt);
        }
      }
      break;

    case irr::ode::eIrrOdeEventBodyRemoved: {
        CIrrOdeEventBodyRemoved *evt=new CIrrOdeEventBodyRemoved(pEvent);
        if (!evt->getBody()) return false;
        m_lMessages.push_back(evt);
      }
      break;

    case irr::ode::eIrrOdeEventInit: {
        m_iStep=0;
        CEventIdentify *pEvt=new CEventIdentify(m_sAppName.c_str());
        m_lMessages.push_back(pEvt);
        distributeMessage(pEvt);
      }
      break;

    case irr::ode::eIrrOdeEventClose:
      while (m_lMessages.getSize()>0) {
        list<IIrrOdeEvent *>::Iterator it=m_lMessages.begin();
        IIrrOdeEvent *msg=*it;
        m_lMessages.erase(it);
        delete msg;
      }
      m_bOdeInitialized=false;
      m_iStep=0;
      break;

    case irr::ode::eIrrOdeEventStep:
      break;

    case irr::ode::eIrrOdeEventLoadScene: {
        irr::ode::CIrrOdeEventLoadScene *p=(irr::ode::CIrrOdeEventLoadScene *)pEvent;
        stringc cwd=m_pDevice->getFileSystem()->getWorkingDirectory(),
                lvl=stringc(p->getScene());

        if (lvl.equalsn(cwd,cwd.size())) lvl=lvl.subString(cwd.size()+1,lvl.size());

        m_sLevel=stringc(lvl.c_str());
        irr::ode::CIrrOdeEventLoadScene *pEvt=new irr::ode::CIrrOdeEventLoadScene(lvl.c_str());
        m_lMessages.push_back(pEvt);
      }

    default:
      break;
  }
  distributeMessage(pEvent);
  return false;
}

bool CMarbles2WorldObserver::handlesEvent(IIrrOdeEvent *pEvent) {
  return true;
}

void CMarbles2WorldObserver::addListener(IIrrOdeWorldListener *pListener) {
  list<IIrrOdeWorldListener *>::Iterator itListener;
  for (itListener=m_lListeners.begin(); itListener!=m_lListeners.end(); itListener++)
    if (*itListener==pListener) return;

  m_lListeners.push_back(pListener);
  list<IIrrOdeEvent *>::Iterator it;
  for (it=m_lMessages.begin(); it!=m_lMessages.end(); it++) pListener->worldChange(*it);
}

void CMarbles2WorldObserver::removeListener(IIrrOdeWorldListener *pListener) {
  list<IIrrOdeWorldListener *>::Iterator it;
  for (it=m_lListeners.begin(); it!=m_lListeners.end(); it++)
    if (*it==pListener) {
      m_lListeners.erase(it);
      return;
    }
}
