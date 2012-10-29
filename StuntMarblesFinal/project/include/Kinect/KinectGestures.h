#include <XnCppWrapper.h>
#include <XnOS.h>
#include <iostream>
#include <map>
#include <cmath>
#ifndef KinectGestures_H_
#define KinectGestures_H_
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/highgui/highgui.hpp"

static const float NECK_LEN = 50.0f;
static const float LOWER_ARM_LEN = 150.0f;
static const float ARM_LEN = 400.0f;

class KinectGestures{
public:
				double horizontalLean;
	double frontLean;
	KinectGestures(){
		neckLength = NECK_LEN;            
		lowerArmLength = LOWER_ARM_LEN;   
		armLength = ARM_LEN;  
		isLeanFwd= false;
		isLeanLeft=false;
		isLeanRight=false;
	}
	float distApart(cv::Point3d& p1, cv::Point3d& p2);
	// standard skeleton lengths

	float neckLength,lowerArmLength,armLength;         

	std::map<XnSkeletonJoint, XnSkeletonJointPosition> currentSkel;
	//map for skeleton
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> *userSkels;

	void leanForward();
	void leanBack();
	void leanLeft();
	void leanRight();



	xn::UserGenerator* mUserGenerator;
	static KinectGestures* getInstance();
	bool isLeanFwd,isLeanLeft,isLeanRight;
	int init ();
	void detectGesture(XnUserID userID);

	static KinectGestures& Instance()
	{
		static KinectGestures singleton;
		return singleton;
	}
	void setUserGenerator(xn::UserGenerator& usergen);
	void setUserSkels(std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> &userSkeletons);
	void calculateHumanLength(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel);
	cv::Point3d getJointPos(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel, XnSkeletonJoint j);
};
#endif // CameraSystem_H_