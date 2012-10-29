  #include <irrlicht.h>
  #include <CStateMachine.h>

  #include <windows.h>

#include <KinectFramework.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
#if defined(_MSC_VER)
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "OpenNI.lib")
#endif
int main(int argc, char** argv) {
	KFController* player1Joystick;
	KFController* player2Joystick;
		int choice1,choice2;
	 std::cout << "Please choose controller for player no 1:\n ";
	 std::cout << "1: Lean Controller:\n ";
	 std::cout << "2:Wii Balance Board-like Controller\n ";
	 std::cout << "3: Location Controller:\n ";
	 std::cout << "4: knee controller with steering virtual steering wheel:\n ";
	 std::cin >> choice1;

	 	 std::cout << "Please choose controller for player no 2:\n ";
	 std::cout << "0: NO 2nd player, focus game on player who is nearest to the Camera:\n ";
	std::cout << "1: Lean Controller:\n ";
	 std::cout << "2:Wii Balance Board-like Controller\n ";
	 std::cout << "3: Location Controller:\n ";
	 std::cout << "4: knee controller with steering virtual steering wheel:\n ";

	 std::cin >> choice2;
	 // start the kinect camera and its boost thread
	 KFCVWrapper* kinectWrapper= KFCVWrapper::getInstance();
	 kinectWrapper->runKinect=true;





	//kinectWrapper->noPlayers=2;
	
	switch (choice1) {
  case 1:
	  //not great
	   player1Joystick = new KFLeanController();
	  break;
  case 2:
	  // very good
	     player1Joystick = new KFPendulumController();
	  break;
  case 3:
	  // kinda good, requires a lot of movement
	   player1Joystick = new KFLocationController();
	  break;
	case 4:
		// very bad
	   player1Joystick = new KFKneeController();
	  break;
  default:
	    player1Joystick = new KFPendulumController();
    std::cout << "Player 1 will be Wii balance board based\n";
  }

	switch (choice2) {
	case 0:
		  kinectWrapper->focusNearestUser=true;
		  std::cout <<"No split screen. Just give focus to user who is in front\n";
  case 1:
	   player2Joystick = new KFLeanController();
	  break;
  case 2:
	     player2Joystick = new KFPendulumController();
	  break;
  case 3:
	   player2Joystick = new KFLocationController();
	  break;
	case 4:
	   player2Joystick = new KFKneeController();
	  break;
  default:
	   // player2Joystick = new KFPendulumController();
    std::cout << "Only 1 player. FOCUS on the nearest player\n";
  }

	 player2Joystick = new KFPendulumController(); //need to remove
	kinectWrapper->setKinectController1(*player1Joystick);	
	kinectWrapper->setKinectController2(*player2Joystick);	

  //Create our state machine, start it and delete it when it returns
  CStateMachine *theMachine=new CStateMachine();
  theMachine->run();

 
  delete theMachine;
  return 0;
}

