  #include <CMenu.h>
  #include <CStateMachine.h>

CMenu::CMenu(IrrlichtDevice *pDevice, CStateMachine *pStateMachine) : IState(pDevice,pStateMachine) {
  m_iMenuItem=0;
}

void CMenu::activate(IState *pPrevious) {
  m_pDevice->getSceneManager()->loadScene("data/menu_scene.xml");
  m_pDevice->setEventReceiver(this);

  m_bMoving=false;
  m_bSelect=false;
  m_pTimer=m_pDevice->getTimer();
  while (m_pTimer->isStopped()) m_pTimer->start();
  m_iLastTime=m_pTimer->getTime();
  m_pMenu=m_pDevice->getSceneManager()->getSceneNodeFromName("menu");
  m_pDevice->getCursorControl()->setVisible(false);
  m_pSndEngine=m_pStateMachine->getSoundEngine();
  m_pSmgr->getActiveCamera()->setAspectRatio(m_pStateMachine->getAspectRatio());

  m_pMenu->setPosition(vector3df(0.0f,2.5*m_iMenuItem,0.0f));

  dimension2du cScreenSize=m_pDevice->getVideoDriver()->getScreenSize();

  IGUIImage *pImg=m_pGuienv->addImage(rect<s32>(0,0,cScreenSize.Width,cScreenSize.Height));
  pImg->setScaleImage(true);
  //pImg->setImage(m_pDriver->getTexture("data/mainmenu_back.png"));
  pImg->setUseAlphaChannel(true);
}

void CMenu::deactivate(IState *pNext) {
  m_pDevice->getSceneManager()->clear();
  m_pDevice->getCursorControl()->setVisible(true);
  m_pGuienv->clear();
}

u32 CMenu::update() {
  u32 iDiff,iThisTime;

  iThisTime=m_pTimer->getTime();
  iDiff=iThisTime-m_iLastTime;
  m_iLastTime=iThisTime;

	//is the menu moving?
  if (m_bMoving) {
    f32 posY=m_pMenu->getPosition().Y,fMove=((f32)iDiff)/150.0f;
    if (posY>2.5*m_iMenuItem) {
      posY-=fMove;
      if (posY<2.5*m_iMenuItem) {
        posY=(f32)(2.5*m_iMenuItem);
        m_bMoving=false;
      }
    }

    if (posY<2.5*m_iMenuItem) {
      posY+=fMove;
      if (posY>2.5*m_iMenuItem) {
        posY=(f32)(2.5*m_iMenuItem);
        m_bMoving=false;
      }
    }
    m_pMenu->setPosition(vector3df(0.0f,posY,0.0f));
  }

  if (m_bSelect) {
    switch (m_iMenuItem) {
      case 0: return  2; break; //start game
      case 1: return  7; break; //tournament
      case 2: return 11; break; //net game
      case 3: return  9; break; //replay
      case 4: return  3; break; //options
      case 5: return  4; break; //hiscore
      case 6: return  5; break; //credits
      case 7: return 99; break; //quit
    }
  }

  return 0;
}

bool CMenu::OnEvent(const SEvent &event) {
  if (event.EventType==EET_KEY_INPUT_EVENT) {
    if (!event.KeyInput.PressedDown) {
      switch (event.KeyInput.Key) {
        case KEY_UP:
          if (m_iMenuItem>0) {
            m_iMenuItem--;
            m_bMoving=true;
            if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
          }
          return true;
          break;

        case KEY_DOWN:
          if (m_iMenuItem<7) {
            m_iMenuItem++;
            m_bMoving=true;
            if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuchange.ogg");
          }
          return true;
          break;

        case KEY_RETURN:
          m_bSelect=true;
          if (m_pSndEngine) m_pSndEngine->play2D("data/sounds/menuselect.ogg");
          break;

        default:
          break;
      }
    }
  }

  return false;
}
