#include <KinectFramework.h>
//void KFLeanController::drawController()
//{
//	joystickView.setTo(0);
//	cv::rectangle(joystickView,cv::Rect(320-50/2,240-50/2,50,50),cv::Scalar(255,155,0));
//
//	for(int i=0;i<3;i++)
//		for(int j=0;j<3;j++)
//			cv::rectangle(joystickView,cv::Rect(i*220,j*160,220,160),cv::Scalar(255,155,0));
//}
void KFLeanController::processUserGestures()
{

	cv::Point3d leftHipPt = getJointPos(currentSkelReal, XN_SKEL_LEFT_HIP);
	cv::Point3d rightHipPt = getJointPos(currentSkelProj, XN_SKEL_RIGHT_HIP);
	cv::Point3d headPt = getJointPos(currentSkelReal, XN_SKEL_HEAD);
	xMovement = headPt.x-leftHipPt.x;
	cv::Point3d torsoPt = getJointPos(currentSkelReal, XN_SKEL_TORSO);
	zMovement = (headPt.z - torsoPt.z)*-1 ;
	
	if(firstData && zMovement!=0 &&xMovement!=0){
	zMovementFirst=zMovement;
	xMovementFirst=xMovement;
	firstData=false;
	}
	//normalize the data by subtracting the data stored when the user is first detected presumably in upright position
	
	xMovement-=xMovementFirst;
	zMovement-=zMovementFirst;
	//std::cout << "REAL FRONT MOVEMENT IS : "<<zMovement <<"\n";
	
	//normalize the data so that we only receive values between - 100 and 100
	// leanForward max is 300
	//leanSide min is -200 max is 200
	if (xMovement < -60) {    // head is forward
		if (!moveLeft) {
			//std::cout <<"MOVING FORWARD" <<"\n";
			moveLeft=true;
		}
	}  else {   // not forward
		if (moveLeft) {
			moveLeft = false;
			//keyFwd(false);			
		}
	}
		if (xMovement> 60) {    // head is forward
		if (!moveRight) {
			//std::cout <<"MOVING RIGHT" <<"\n";
			moveRight=true;
		}
	}  else {   // not forward
		if (moveRight) {
			moveRight = false;
		}
	}
	if (zMovement > 100) {    // head is forward
		if (!moveFwd) {
			//std::cout <<"MOVING FORWARD" <<"\n";
			moveFwd=true;
		}
	}  else {   // not forward
		if (moveFwd) {
			moveFwd = false;
			//keyFwd(false);			
		}
	}
	if (zMovement < -30) {    // leaning backward
		if (!isLeanBack) {			
			isLeanBack=true;
			//keyBack(true);
		}
	}  else {   // not forward
		if (isLeanBack) {
			isLeanBack = false;
		}
	}	
}
