#include <KinectFramework.h>
//void KFLeanController2::drawController()
//{
//	joystickView.setTo(0);
//	cv::rectangle(joystickView,cv::Rect(320-50/2,240-50/2,50,50),cv::Scalar(255,155,0));
//
//	for(int i=0;i<3;i++)
//		for(int j=0;j<3;j++)
//			cv::rectangle(joystickView,cv::Rect(i*220,j*160,220,160),cv::Scalar(255,155,0));
//
//}
void KFLeanController2::processUserGestures()
{
	//leanLeft();
	//	leanRight();
	//	jump();
	cv::Point3d leftHipPt = getJointPos(currentSkelReal, XN_SKEL_LEFT_HIP);
	cv::Point3d rightHipPt = getJointPos(currentSkelProj, XN_SKEL_RIGHT_HIP);
	cv::Point3d headPt = getJointPos(currentSkelReal, XN_SKEL_HEAD);
	double sideLean = headPt.x-leftHipPt.x;
	//std::cout << "LEFT LEAN IS " <<sideLean <<"\n";
	cv::Point3d torsoPt = getJointPos(currentSkelReal, XN_SKEL_TORSO);

	zMovement = (headPt.z - torsoPt.z)*-1 ;
	//std::cout << "FRONT MOVEMENT IS : "<<zMovement <<"\n";
		if (sideLean < -100) {    // head is forward
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
		if (sideLean > 180) {    // head is forward
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
			//keyFwd(true);
		}
	}  else {   // not forward
		if (moveFwd) {
			moveFwd = false;
			//keyFwd(false);			
		}
	}
	if (zMovement < -120) {    // head is forward
		if (!isLeanBack) {			
			isLeanBack=true;
			//keyBack(true);
		}
	}  else {   // not forward
		if (isLeanBack) {
			isLeanBack = false;
			//keyBack(false);

		}
	}	
}
