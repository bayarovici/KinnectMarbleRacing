  #include <GameClasses/CGhostRecorder.h>
  #include <GameClasses/CGhostClasses.h>
  #include <CGame.h>

  #include <IrrOdeNet/CEventCpInfo.h>
  #include <IrrOdeNet/WorldObserverMessages.h>

CGhostRecorder::CGhostRecorder(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
  m_pSmgr=pDevice->getSceneManager();
  m_pCurLap=NULL;
  m_pNode=NULL;
  m_iLapNo=0;
}

CGhostRecorder::~CGhostRecorder() {
  while (m_lLaps.getSize()>0) {
    list<CGhostLaps *>::Iterator it=m_lLaps.begin();
    CGhostLaps *l=(*it);
    m_lLaps.erase(it);
    delete l;
  }
}

void CGhostRecorder::activate() {
  CMarbles2WorldObserver::getSharedInstance()->addListener(this);
  m_iLapNo=0;
  m_pCurLap=NULL;
}

void CGhostRecorder::setData(u32 iNodeId, const c8 *sPlayer, const c8 *sLevel) {
  m_pHeader=new CGhostHeader(sPlayer,sLevel);
  m_pNode=m_pSmgr->getSceneNodeFromId(iNodeId);
}

void CGhostRecorder::deactivate() {
  CMarbles2WorldObserver::getSharedInstance()->removeListener(this);
  printf("level=\"%s\"\nplayer=\"%s\"\n",m_pHeader->getLevel(),m_pHeader->getPlayer());

  list<CGhostLaps *>::Iterator it;
  for (it=m_lLaps.begin(); it!=m_lLaps.end(); it++) {
    if ((*it)->m_iLapNo==m_iLapNo) {
      printf("\tremoving last lap!\n");
      CGhostLaps *p=*it;
      m_lLaps.erase(it);
      delete p;
      break;
    }
    else printf("\tlap %i: %i steps\n",(*it)->m_iLapNo,(*it)->m_lStates.getSize());
  }
}

void CGhostRecorder::worldChange(IIrrOdeEvent *pMsg) {
  if (pMsg->getType()==eMessageCpInfo) {
    CEventCpInfo *pInfo=(CEventCpInfo *)pMsg;
    if (pInfo->getCode()==eMessageLap) {
      m_iLapNo++;
      m_pCurLap=new CGhostLaps(m_iLapNo);
      m_lLaps.push_back(m_pCurLap);
    }
  }

  if (pMsg->getType()==irr::ode::eIrrOdeEventStep) {
    if (m_pCurLap && m_pNode) {
      CGhostState *p=new CGhostState(m_pNode->getAbsolutePosition(),m_pNode->getRotation());
      m_pCurLap->m_lStates.push_back(p);
    }
  }
}

int CGhostRecorder::getLapCount() {
  return m_lLaps.getSize();
}

void CGhostRecorder::save() {
  CGhostLaps *pToSave=NULL;
  list<CGhostLaps *>::Iterator it;

  for (it=m_lLaps.begin(); it!=m_lLaps.end(); it++)
    if (pToSave==NULL || pToSave->m_lStates.getSize()>(*it)->m_lStates.getSize())
      pToSave=*it;

  if (pToSave==NULL) {
    printf("Ooops ... that's not OK\n");
    return;
  }

  IFileSystem *pFs=m_pDevice->getFileSystem();
  c8 sName[0xFF];
  u32 i=0;
  do {
    sprintf(sName,"data/ghost/%s_%i.ghost",m_pHeader->getLevel(),i++);
  }
  while (pFs->existFile(path(sName)));
  printf("saving ghost \"%s\" (lap %i) ... \n",sName,pToSave->m_iLapNo);
  IWriteFile *f=pFs->createAndWriteFile(sName);

  CSerializer *pSerializer=m_pHeader->serialize();
  u32 iSize=pSerializer->getBufferSize();
  f->write(&iSize,sizeof(u32));
  f->write(pSerializer->getBuffer(),pSerializer->getBufferSize());

  list<CGhostState *>::Iterator it2;
  for (it2=pToSave->m_lStates.begin(); it2!=pToSave->m_lStates.end(); it2++) {
    CGhostState *p=*it2;
    pSerializer=p->serialize();
    iSize=pSerializer->getBufferSize();
    f->write(&iSize,sizeof(u32));
    f->write(pSerializer->getBuffer(),pSerializer->getBufferSize());
  }

  f->drop();
  printf("Ready.\n");
}
