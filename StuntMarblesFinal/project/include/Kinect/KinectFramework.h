/****************************************************************************
*                                                                           *
*  OpenCVNIWrapper                                                          *
*  Copyright (C) 2011 PrimeSense Ltd.                                       *
*                                                                           *                                         *
*  This program is free software: you can redistribute it and/or modify     *
*  it under the terms of the GNU Lesser General Public License as published *
*  by the Free Software Foundation, either version 3 of the License, or     *
*  (at your option) any later version.                                      *
*                                                                           *
*  This program is distributed in the hope that it will be useful,          *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
*  GNU Lesser General Public License for more details.                      *
*                                                                           *
*  See <http://www.gnu.org/licenses/>.                                      *
*                                                                           *
*  By Gabriel Sanmart√≠n & Gabriel Moy√                                       *
*  gabriel.sanmartin@usc.es & gabriel.moya@uib.es                           *
****************************************************************************/

#ifndef CVKINECTWRAPPER_H_
#define CVKINECTWRAPPER_H_

#include <XnOpenNI.h>
#include <XnList.h>
#include <XnCodecIDs.h>


#include <XnCppWrapper.h>

#include <math.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include <boost\thread.hpp>
#include <map>
#include <KinectGestures.h>
#include <irrlicht.h>
using namespace xn;
using namespace cv;


#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_OVERLAY

class KFController;
class KFCVWrapper {

public:
	bool focusNearestUser;
	bool rightHandUp;
	bool runKinect;
	double transmissionData;
	int noPlayers;
	KinectGestures* gesturePt;
	KFController* m_gestureController1;
	KFController* m_gestureController2;
	
	float frontLean;
	int oldLeanFront;

	void drawMovementRectangle(int x,int y);
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> userSkels;
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> userSkelsReal;

	void updateJoints(XnUserID userID);
	void updateJoint(XnUserID userID, XnSkeletonJoint joint);

	//gesture booleans
	bool isPlayer1LeanFwd,isPlayer1LeanLeft,isPlayer1LeanRight;
	bool isPlayer2LeanFwd,isPlayer2LeanLeft,isPlayer2LeanRight;
	double zMovement1,xMovement1,zMovement2,xMovement2;
	bool userDetected;
	boost::thread internalThread_;

	bool init(std::string CalibFilePath);
	static  KFCVWrapper* getInstance();

	bool update();
	void getUser();
	void verifyGesture();
	void _boostThread();


	const XnUserID* verifyClosestUser();
	void nearUserDetect();
	const XnUserID* currentUser;

	bool started;

	void setKinectController1(KFController& controller){
		m_gestureController1 = &controller;
	}

		void setKinectController2(KFController& controller){
		m_gestureController2 = &controller;
	}

private:
	void processUsers();
	static KFCVWrapper *_instance;

	KFCVWrapper();
	virtual ~KFCVWrapper();


	std::string CalibFilePath;


	Context g_context;
	xn::UserGenerator mUserGenerator;
	xn::SkeletonCapability* p_mSC;

	ScriptNode g_scriptNode;
	DepthGenerator g_depth;
	ImageGenerator g_image;

	DepthMetaData g_depthMD;
	ImageMetaData g_imageMD;

	static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus calibrationError, void* pCookie);
	// user tracking stuff
};


class KFController
{
public:
	bool calibrated;
	double transmissionData;
	bool firstData;
	float zMovementFirst,xMovementFirst;
	float zMovement,xMovement;
	bool moveFwd,moveLeft,moveRight;

	void keyFwd(bool);
	void keyBack(bool);
	void leanForward();
	void leanBack();
	void leanLeft();
	void leanRight();
	void jump();
	void turnLeft();
	void turnRight();

	void rightHandUp();
	void rightHandFwd();

	void leftHandUp();
	void leftHandFwd();

	cv::Point3d torsoPt;
	cv::Mat* depthImage;
	cv::Mat joystickView;
	float neckLength,lowerArmLength,armLength;
	KFController(){
		calibrated=false;
		neckLength = NECK_LEN;            
		lowerArmLength = LOWER_ARM_LEN;   
		armLength = ARM_LEN;  

		joystickView = Mat(480, 640, CV_8UC3, Scalar::all(65));
	}
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> *userSkels;
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> *userSkelsReal;
	std::map<XnSkeletonJoint, XnSkeletonJointPosition> currentSkelProj;
	std::map<XnSkeletonJoint, XnSkeletonJointPosition> currentSkelReal;

	void calculateHumanLength(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel);
	//virtual void drawController()=0;
	virtual void processUserGestures()=0;

	void setUserGenerator(xn::UserGenerator& usergen);
	void setUserSkels(std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> &userSkeletons);
	void setUserSkelsReal(std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> &userSkeletons);	
	cv::Point3d getJointPos(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel, XnSkeletonJoint j);
	float distApart(cv::Point3d& p1, cv::Point3d& p2);

	void updateController(XnUserID userID);
protected:
	double horizontalLean;
	double frontLean;
	bool isLeanFwd,isLeanLeft,isLeanRight,isTurnLeft,isTurnRight,isLeanBack;
	cv::Point3d torsoOld;
};

class KFLocationController : public KFController
{
public:

	float zOffset, xOffset;
	cv::Point3d initialPosition;
	bool startPosition, bothHandsUp;
	KFLocationController() : KFController(){
		startPosition=false;
		bothHandsUp=false;
	}
	void drawController();
	void processUserGestures();
	void calculateHumanLength(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel);
};

class KFPendulumController : public KFController
{
public:

	float zOffset, xOffset;
	cv::Point3d initialPosition;
	bool startPosition, bothHandsUp;
	KFPendulumController() : KFController(){
		startPosition=false;
		bothHandsUp=false;
	}
	//void drawController();
	void processUserGestures();
	void calculateHumanLength(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel);
};



class KFLeanController : public KFController
{
public:
	//void drawController();
	void processUserGestures();
	KFLeanController() : KFController(){
		firstData=true;
		moveFwd=moveLeft=moveRight=false;
		isLeanFwd= false;
		isLeanLeft=false;
		isLeanRight=false;
		torsoOld = cv::Point3d(0.0f,0.0f,0.0f);
	}
private:

};

class KFKneeController : public KFController
{
public:
	bool b_angleInitialized;
	double initialAngle;
	double derivatives[10];
	int derivativesIndex;
	cv::Point3d kneeLeftOld,kneeRightOld;
	//void drawController();
	void processUserGestures();
	KFKneeController() : KFController(){
		for(int i=0;i<10;i++)
			derivatives[i]=0;
		b_angleInitialized=false;
		firstData=true;
		kneeLeftOld= cv::Point3d(0.0f,0.0f,0.0f);
		kneeRightOld= cv::Point3d(0.0f,0.0f,0.0f);
		derivativesIndex  =0;
	}
private:

};


class KFTurnController : public KFController
{
public:
	//void drawController();
	void processUserGestures();
	KFTurnController() : KFController(){
		moveFwd=moveLeft=moveRight=false;
		isLeanFwd= false;
		isLeanLeft=false;
		isLeanRight=false;
		torsoOld = cv::Point3d(0.0f,0.0f,0.0f);
	}
private:

};

class KFLeanController2 : public KFController
{
public:
	//void drawController();
	void processUserGestures();
	KFLeanController2() : KFController(){
		moveFwd=moveLeft=moveRight=false;
		isLeanFwd= false;
		isLeanLeft=false;
		isLeanRight=false;
		torsoOld = cv::Point3d(0.0f,0.0f,0.0f);
	}
private:

};

#endif /* CVKINECTWRAPPER_H_ */
