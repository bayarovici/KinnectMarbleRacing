#include <KinectFramework.h>
//void KFTurnController::drawController()
//{
//	joystickView.setTo(0);
//	cv::rectangle(joystickView,cv::Rect(320-50/2,240-50/2,50,50),cv::Scalar(255,155,0));
//
//	for(int i=0;i<3;i++)
//		for(int j=0;j<3;j++)
//			cv::rectangle(joystickView,cv::Rect(i*220,j*160,220,160),cv::Scalar(255,155,0));
//}
void KFTurnController::processUserGestures()
{
	cv::Point3d leftShPt = getJointPos(currentSkelReal, XN_SKEL_LEFT_SHOULDER);
	cv::Point3d rightShPt = getJointPos(currentSkelProj, XN_SKEL_RIGHT_SHOULDER);
	cv::Point3d headPt = getJointPos(currentSkelReal, XN_SKEL_HEAD);
	xMovement = rightShPt.x-leftShPt.x;
	cv::Point3d torsoPt = getJointPos(currentSkelReal, XN_SKEL_TORSO);

	zMovement = (headPt.z - torsoPt.z)*-1 ;
	//std::cout << "REAL FRONT MOVEMENT IS : "<<zMovement <<"\n";
		if (xMovement < -100) {    // head is forward
		if (!moveLeft) {
			//std::cout <<"MOVING FORWARD" <<"\n";
			moveLeft=true;
			//keyFwd(true);
		}
	}  else {   // not forward
		if (moveLeft) {
			moveLeft = false;
			//keyFwd(false);			
		}
	}
		if (xMovement> 180) {    // head is forward
		if (!moveRight) {
			moveRight=true;
		}
	}  else {   // not forward
		if (moveRight) {
			moveRight = false;
		}
	}
	if (zMovement > 100) {    // head is forward
		if (!moveFwd) {
			moveFwd=true;
		}
	}  else {   // not forward
		if (moveFwd) {
			moveFwd = false;	
		}
	}
	if (zMovement < -120) {    // head is forward
		if (!isLeanBack) {			
			isLeanBack=true;
		}
	}  else {   // not forward
		if (isLeanBack) {
			isLeanBack = false;
		}
	}	
}
