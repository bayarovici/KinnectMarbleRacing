  #include <CIrrOdeKlangManager.h>
  #include <CMarbles2RePlayer.h>
  #include <CMessage.h>
  #include <CGame.h>

  #include <IrrOdeNet/CEventPlayer.h>
  #include <IrrOdeNet/CEventCpInfo.h>
  #include <IrrOdeNet/CEventInit.h>

void CMarbles2Replayer::startReplay() {
  m_pStepNo->setVisible(true);
}

CMarbles2Replayer::CMarbles2Replayer(IrrlichtDevice *pDevice) : CIrrOdeRePlayer(pDevice) {
}

void CMarbles2Replayer::createGUI() {
  dimension2du screen=m_pDevice->getVideoDriver()->getScreenSize();
  u32 dimx=screen.Width,dimy=screen.Height;
  m_iGoStep=-1;

  IGUIEnvironment *gui=m_pDevice->getGUIEnvironment();

  m_pStepNo=gui->addStaticText(L"",rect<s32>(dimx/2-100,10,dimx/2+100,30),true,true,NULL,-1,true);
  m_pStepNo->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  m_pStepNo->setVisible(false);

  for (u32 i=0; i<4; i++) {
    rect<s32> cRect;

    switch (i) {
      case 0: cRect=rect<s32>(      10,     10,    280,     30); break;
      case 1: cRect=rect<s32>(dimx-280,     10,dimx-10,     30); break;
      case 2: cRect=rect<s32>(      10,dimy-80,    280,dimy-60); break;
      case 3: cRect=rect<s32>(dimx-280,dimy-80,dimx-10,dimy-60); break;
    }

    m_pPlayer[i]=gui->addStaticText(L"Player",cRect                  ,true,true,NULL,-1,true);
    m_pCp[i]=gui->addStaticText(L"Checkpoint",cRect+position2di(0,25),true,true,NULL,-1,true);
    m_pRt[i]=gui->addStaticText(L"RaceTime"  ,cRect+position2di(0,50),true,true,NULL,-1,true);

    m_pPlayer[i]->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pCp[i]->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pRt[i]->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
    m_pPlayer[i]->setVisible(false);
    m_pCp[i]->setVisible(false);
    m_pRt[i]->setVisible(false);
  }

  m_pCountDown=gui->addStaticText(L"Countdown",rect<s32>(dimx/2-100,dimy/2-15,dimx/2+100,dimy/2+15),true,true,NULL,-1,true);
  m_pFastest=gui->addStaticText(L"Fastest Lap",rect<s32>(dimx/2-100,65,dimx/2+100,125),true,true,NULL,-1,true);
  m_pCountDown->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  m_pFastest->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);
  m_pCountDown->setVisible(false);
  m_pFastest->setVisible(false);
}

bool CMarbles2Replayer::handlesEvent(IIrrOdeEvent *pEvent) {
  return pEvent->getType()>=irr::ode::eIrrOdeEventUser;
}

bool CMarbles2Replayer::onEvent(IIrrOdeEvent *pMessage) {
  u16 iIdx=0,iMsgCode=pMessage->getType();
  u32 iTime;
  wchar_t ws[0xFF];

  switch (iMsgCode) {
    case eMessagePlayer: {
        CEventPlayer *p=(CEventPlayer *)pMessage;
        iIdx=p->getNum();
        m_pPlayer[iIdx]->setText(stringw(stringc(p->getName())).c_str());
        m_pPlayer[iIdx]->setVisible(true);
        m_pCp[iIdx]->setVisible(true);
      }
      break;

    case eMessageInit: {
        CEventInit *p=(CEventInit *)pMessage;
        switch (p->getCode()) {
          case eMessageReady:
            m_pCountDown->setText(L"Ready");
            m_pCountDown->setVisible(true);
            break;

          case eMessageCntdn:
            iIdx=p->getData();
            swprintf(ws,0xFF,L"%i",iIdx);
            m_pCountDown->setText(ws);
            break;

          case eMessageGo:
            m_pCountDown->setText(L"GO!");
            m_iGoStep=180;
            break;
        }
      }
      break;

    case eMessageCpInfo: {
        CEventCpInfo *p=(CEventCpInfo *)pMessage;
        iIdx=p->getCp();
        iTime=p->getTime();
        if (iMsgCode==eMessageFastestLap) {
          swprintf(ws,0xFF,L"Fastest Lap\n%.2f / %s",((f32)iTime)/1000,m_pPlayer[iIdx]->getText());
          m_pFastest->setText(ws);
          m_pFastest->setVisible(true);
        }
        else
          if (iMsgCode==eMessagefinished) {
            swprintf(ws,0xFF,L"Racetime: %.2f",((f32)iTime)/1000);
            m_pRt[iIdx]->setText(ws);
            m_pRt[iIdx]->setVisible(true);
          }
          else {
            swprintf(ws,0xFF,iMsgCode==eMessageLap?L"Lap: %.2f":L"Checkpoint: %.2f sec",((f32)iTime)/1000);
            m_pCp[iIdx]->setText(ws);
          }
      }
      break;
  }
  return true;
}

void CMarbles2Replayer::step(u32 iStepNo) {
  if (m_pCountDown->isVisible()) {
    if (m_iGoStep==0)
      m_pCountDown->setVisible(false);
    else
      if (m_iGoStep>0) m_iGoStep--;
  }

  wchar_t s[0xFF];
  swprintf(s,0xFF,L"%.2f sec",((f32)(iStepNo*16))/1000.0f);
  m_pStepNo->setText(s);
}

void CMarbles2Replayer::replayFinished() {
  printf("replay finished!\n");
  m_iGoStep=10000;
  m_pCountDown->setText(L"Replay Finished");
  m_pCountDown->setVisible(true);
  CIrrOdeKlangManager::getSharedInstance()->deactivate();
}

