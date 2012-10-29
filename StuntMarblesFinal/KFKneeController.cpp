#include <KinectFramework.h>
//void KFKneeController::drawController()
//{
//	joystickView.setTo(0);
//	cv::rectangle(joystickView,cv::Rect(320-50/2,240-50/2,50,50),cv::Scalar(255,155,0));
//
//	for(int i=0;i<3;i++)
//		for(int j=0;j<3;j++)
//			cv::rectangle(joystickView,cv::Rect(i*220,j*160,220,160),cv::Scalar(255,155,0));
//}
void KFKneeController::processUserGestures()
{
	double accumulator=0;
	//transmissionData=0;
	cv::Point3d leftKneePt = getJointPos(currentSkelReal, XN_SKEL_LEFT_KNEE);
	cv::Point3d rightKneePt = getJointPos(currentSkelProj, XN_SKEL_RIGHT_KNEE);
	
	cv::Point3d leftHandPt = getJointPos(currentSkelProj, XN_SKEL_LEFT_HAND);
	cv::Point3d rightHandPt = getJointPos(currentSkelProj, XN_SKEL_RIGHT_HAND);
	
	double handAngle = atan2(leftHandPt.x - rightHandPt.x, leftHandPt.y-rightHandPt.y);
	if(!b_angleInitialized && handAngle!=0){
	b_angleInitialized = true;
	initialAngle = handAngle;
	}
	handAngle=(handAngle+1.50)*-120;
	transmissionData=handAngle;
	xMovement=transmissionData;
	derivatives[derivativesIndex] = abs(leftKneePt.z - kneeLeftOld.z);
	derivativesIndex+=1;
	if(derivativesIndex==10)
		derivativesIndex=0;
	for(int i =0;i<10;i++)
		accumulator+=derivatives[derivativesIndex];
	//zMovement=zMovement /5;
	if(accumulator < 80)
		accumulator=0;
	zMovement=accumulator;
	kneeLeftOld = leftKneePt;
}
