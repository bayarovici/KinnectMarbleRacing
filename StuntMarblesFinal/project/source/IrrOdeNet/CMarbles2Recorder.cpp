  #include <IrrOdeNet/CMarbles2Recorder.h>

CMarbles2Recorder::CMarbles2Recorder(const c8 *sFileName, const c8 *sLevelName, const c8 *sAppName, IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;

  strcpy(m_sFileName,sFileName);
  strcpy(m_sLevelName,sLevelName);
}

CMarbles2Recorder::~CMarbles2Recorder() {
  CMarbles2WorldObserver::getSharedInstance()->removeListener(this);
  while (m_lMessages.getSize()>0) {
    list<CMessage *>::Iterator it=m_lMessages.begin();
    CMessage *pMsg=*it;
    m_lMessages.erase(it);
    delete pMsg;
  }
}

void CMarbles2Recorder::worldChange(IIrrOdeEvent *pMsg) {
  CMessage *myMsg=new CMessage(pMsg->serialize());
  m_lMessages.push_back(myMsg);
}

void CMarbles2Recorder::startRecording() {
  CMarbles2WorldObserver::getSharedInstance()->addListener(this);
}

void CMarbles2Recorder::stopRecording() {
  CMarbles2WorldObserver::getSharedInstance()->removeListener(this);
}


void CMarbles2Recorder::save() {
  printf("saving replay to file \"%s\" ",m_sFileName);
  IWriteFile *f=m_pDevice->getFileSystem()->createAndWriteFile(m_sFileName);

  if (!f) { printf("error while saving\n"); return; }

  list<CMessage *>::Iterator it;
  for (it=m_lMessages.begin(); it!=m_lMessages.end(); it++) {
    (*it)->save(f);
  }

  f->drop();
  printf("... Ready.\n");
}
