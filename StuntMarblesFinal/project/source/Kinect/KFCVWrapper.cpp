/*
*  KFCVWrapper.cpp
*
*  Created on: 02/11/2011
*     Original Author: biel
	Modified by : Bayar Menzat

*/
/*
- get the calibration of user by distance done correctly
- split screen gaemplay
http://viml.nchc.org.tw/blog/paper_info.php?CLASS_ID=1&SUB_ID=1&PAPER_ID=272
*/
#include <KinectFramework.h>

KFCVWrapper*  KFCVWrapper::_instance;
static KinectGestures *gestureDetector = &KinectGestures::Instance();

xn::UserGenerator *g_UserGenerator;
bool gUserDetected=false;


void XN_CALLBACK_TYPE KFCVWrapper::User_NewUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("New user identified: %d\n", user);

	if (generator.GetSkeletonCap().NeedPoseForCalibration())
	{
		generator.GetPoseDetectionCap().StartPoseDetection("Psi", user);
	}
	else
	{
		generator.GetSkeletonCap().RequestCalibration(user, TRUE);
	}
}

void XN_CALLBACK_TYPE KFCVWrapper::User_LostUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("User %d lost\n", user);
}

void XN_CALLBACK_TYPE UserExit(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("User %d exit\n", user);
}

void XN_CALLBACK_TYPE UserReEnter(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("User %d reenter\n", user);
}

void XN_CALLBACK_TYPE  KFCVWrapper::UserCalibration_CalibrationStart(xn::SkeletonCapability& skeleton, XnUserID user, void* pCookie)
{
	printf("Calibration start for user %d\n", user);
}

void XN_CALLBACK_TYPE  KFCVWrapper::UserCalibration_CalibrationComplete(xn::SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus eStatus, void* pCookie)
{
	printf("Calibration complete for user %d: %s\n", user, (eStatus == XN_CALIBRATION_STATUS_OK)?"Success":"Failure");
	if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
		skeleton.StartTracking(user);
	}
	else if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
	{
		printf("Manual abort occurred, stop attempting to calibrate!");
	}
	else if (skeleton.NeedPoseForCalibration())
	{

		g_UserGenerator->GetPoseDetectionCap().StartPoseDetection("Psi", user);
	}
	else
	{
		skeleton.RequestCalibration(user, TRUE);
	}
}

void XN_CALLBACK_TYPE KFCVWrapper::UserPose_PoseDetected(xn::PoseDetectionCapability& poseDetection, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	printf("Pose '%s' detected for user %d\n", strPose, nId);
	g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, FALSE);
	g_UserGenerator->GetPoseDetectionCap().StopPoseDetection(nId);
}



//static KinectGestures *gestures = &KinectGestures::Instance();



KFCVWrapper:: KFCVWrapper() {
	focusNearestUser=false;
	isPlayer1LeanFwd=isPlayer1LeanRight=isPlayer1LeanLeft=false;
	isPlayer2LeanFwd=isPlayer2LeanRight=isPlayer2LeanLeft=false;
	zMovement1=zMovement2=xMovement1=xMovement2=0;

	started = false;
	userDetected=false;
	gesturePt=gestureDetector;
	//starts a separate thread on which the kinect processing occurs
	internalThread_ =boost::thread(& KFCVWrapper::_boostThread, this);
	internalThread_.detach();

}

KFCVWrapper*  KFCVWrapper::getInstance(){

	if(!_instance){
		//std::cout <<"NEW INSTANCE";
		_instance = new  KFCVWrapper();
	}

	return _instance;
}

KFCVWrapper::~ KFCVWrapper() {
	internalThread_.interrupt(); 
	internalThread_.join(); // make damn sure that the internal thread is gone 
	// before we destroy the class data. 

	started = false;
	_instance = 0;

}


bool  KFCVWrapper::init(string CalibFilePath)
{

	this->CalibFilePath = CalibFilePath;
	XnStatus rc;
	rc =g_context.Init();
	g_context.SetGlobalMirror(true); // set mirror
	if(rc!=XN_STATUS_OK)
	{
		//std::cout <<"fuck";
		return started;
	}
	rc= g_depth.Create(g_context);
	if(rc!=XN_STATUS_OK)
	{
		//std::cout <<"fuck";
		return started;
	}

	g_image.Create(g_context);
	XnMapOutputMode outputMode;
	outputMode.nFPS = 30;
	outputMode.nXRes = 640;
	outputMode.nYRes = 480;

	g_image.SetMapOutputMode(outputMode);
	g_depth.SetMapOutputMode(outputMode);
	g_depth.GetMetaData(g_depthMD);
	g_image.GetMetaData(g_imageMD);


	//_rgbImage = new Mat(480, 640, CV_8UC3, Scalar::all(0));
	//_comboImage = new Mat(480, 640, CV_8UC3, Scalar::all(0));
	//_depthImage = new Mat(480, 640, CV_8UC1, Scalar::all(0));
	//_rawDepth = new Mat(480, 640, CV_16UC1, Scalar::all(0));

	//Create User Generator

	mUserGenerator.Create(g_context);
	g_UserGenerator = &mUserGenerator;
	//Register callback function
	XnCallbackHandle hUserCB;
	mUserGenerator.RegisterUserCallbacks(User_NewUser,User_LostUser,NULL,hUserCB);

	mUserGenerator.RegisterToUserExit(UserExit, NULL, hUserCB);
	mUserGenerator.RegisterToUserReEnter(UserReEnter, NULL, hUserCB);
	//Register callback functions of skeleton capability

	xn::SkeletonCapability mSC = mUserGenerator.GetSkeletonCap();
	mSC.SetSkeletonProfile(XN_SKEL_PROFILE_ALL);//Only upper body
	XnCallbackHandle hCalibCB;


	//mSC.RegisterCalibrationCallbacks(CalibrationStart, CalibrationEnd, &mUserGenerator, hCalibCB);
	p_mSC = &mSC;
	//Register callback functions of pose detection

	//XnCallbackHandle hPoseCB;

	XnCallbackHandle hCalibStart, hCalibComplete;
	XnCallbackHandle hPoseDetected;


	mUserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibStart);
	mUserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibComplete);
	mUserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);

	//mUserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(PoseDetected, NULL, &mUserGenerator, hPoseCB);

	//gestureDetector->setUserGenerator(mUserGenerator); // setter of user gen
	gestureDetector->setUserSkels(userSkels);
	g_context.StartGeneratingAll(); 

	m_gestureController1->setUserSkels(userSkels);
	m_gestureController1->setUserSkelsReal(userSkelsReal);

	m_gestureController2->setUserSkels(userSkels);
	m_gestureController2->setUserSkelsReal(userSkelsReal);
	started = true;


	return started;


}

bool  KFCVWrapper::update(){

	if (!started) return false;

	XnStatus rc = XN_STATUS_OK;

	const XnDepthPixel* pDepth;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return  false;
	}

	g_depth.GetMetaData(g_depthMD);
	g_image.GetMetaData(g_imageMD);

	pDepth = g_depthMD.Data();

	const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
	const XnDepthPixel* pDepthRow = g_depthMD.Data();


	//openCV stuff
	//for (XnUInt y = 0; y < g_imageMD.YRes(); ++y){

	//	const XnRGB24Pixel* pImage = pImageRow;
	//	const XnDepthPixel* pDepth = pDepthRow;

	//	for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage,++pDepth){

	//		_rgbImage->at<Vec3b>(y,x) = Vec3b(pImage->nBlue, pImage->nGreen , pImage->nRed);

	//		if (*pDepth != 0)

	//			_rawDepth->at<short>(y,x) = *pDepth;
	//		else

	//			_rawDepth->at<short>(y,x) = 0 ;
	//	}

	//	pDepthRow += g_depthMD.XRes();
	//	pImageRow += g_imageMD.XRes();
	//}

	//double min, max;

	//Mat aux(480,640,CV_8UC3);

	//minMaxLoc(*_rawDepth, &min, &max,NULL,NULL);

	//_rawDepth->convertTo(*_depthImage, CV_8UC1, 255.0/max);

	//cvtColor(*_depthImage,aux,CV_GRAY2BGR);
	////getUser();

	//_rgbImage->copyTo(*_comboImage);
	//aux.copyTo(*_comboImage, *_depthImage);
	processUsers();
	return true;
}

void KFCVWrapper::processUsers(){

	XnUInt16 nUsers = mUserGenerator.GetNumberOfUsers(); // Get User Number
	xn::SkeletonCapability p_mSC =mUserGenerator.GetSkeletonCap();
	if(nUsers > 0)
	{
		//cout<< "User detected love";
		xn::SkeletonCapability p_mSC =mUserGenerator.GetSkeletonCap();
		XnUserID *aUserID = new XnUserID;
		XnUInt16 nUsers = mUserGenerator.GetNumberOfUsers();
		mUserGenerator.GetUsers(aUserID,nUsers); //not nUsers
		for(int i = 0; i<nUsers;++i)
		{
			if(p_mSC.IsTracking(aUserID[i]))
			{
				//std::cout <<" I AM TRACKING";
				std::map<XnSkeletonJoint, XnSkeletonJointPosition> newuser;
				userSkels[aUserID[i]] = newuser;
				userSkelsReal[aUserID[i]] = newuser;
				updateJoints(aUserID[i]);		


				//FIX
				if(noPlayers==1){ // if playing with 1 player find the nearest player and track him
					if(focusNearestUser){ // focus will be given to player who is in front
						this->nearUserDetect();
						if(userDetected){
							m_gestureController1->updateController(*this->currentUser);
							isPlayer1LeanFwd=m_gestureController1->moveFwd;
							isPlayer1LeanLeft=m_gestureController1->moveLeft;
							isPlayer1LeanRight=m_gestureController1->moveRight;
							zMovement1=m_gestureController1->zMovement;
							xMovement1=m_gestureController1->xMovement;
							transmissionData = m_gestureController1->transmissionData;
						}

					}else{ // always remain focused on first user detected
					if(!userDetected){
						this->nearUserDetect();
					}
					else{
						m_gestureController1->updateController(*this->currentUser);
						isPlayer1LeanFwd=m_gestureController1->moveFwd;
						isPlayer1LeanLeft=m_gestureController1->moveLeft;
						isPlayer1LeanRight=m_gestureController1->moveRight;
						zMovement1=m_gestureController1->zMovement;
						xMovement1=m_gestureController1->xMovement;
						transmissionData = m_gestureController1->transmissionData;
						//drawStickFigure(); NOT NEEDED FOR THIS GAME
					}
					}
				}else if(noPlayers==2){
					//std::cout << "IAM EXECUTING";
					if(i==0){
						m_gestureController1->updateController(aUserID[i]);
						isPlayer1LeanFwd=m_gestureController1->moveFwd;
						isPlayer1LeanLeft=m_gestureController1->moveLeft;
						isPlayer1LeanRight=m_gestureController1->moveRight;
						zMovement1=m_gestureController1->zMovement;
						xMovement1=m_gestureController1->xMovement;
					}
					if(i==1){
						m_gestureController2->updateController(aUserID[i]);
						isPlayer2LeanFwd=m_gestureController2->moveFwd;
						isPlayer2LeanLeft=m_gestureController2->moveLeft;
						isPlayer2LeanRight=m_gestureController2->moveRight;
						zMovement2=m_gestureController2->zMovement;
						xMovement2=m_gestureController2->xMovement;



					}
				}
			}
		}
	}
}

//void  KFCVWrapper::getRGB(Mat *rgb){
//
//	_rgbImage->copyTo(*rgb);
//
//}
//
//void  KFCVWrapper::getRawDepth(Mat *rawDepth){
//
//	_rawDepth->copyTo(*rawDepth);
//
//}
//
//void  KFCVWrapper::getCombo(Mat *combo){
//
//	_comboImage->copyTo(*combo);
//}
//
//void  KFCVWrapper::getDisplayDepth(Mat *displayDepth){
//
//	_depthImage->copyTo(*displayDepth);
//}

void  KFCVWrapper::getUser(){
	XnUInt16 nUsers = mUserGenerator.GetNumberOfUsers(); // Get User Number
	xn::SkeletonCapability p_mSC =mUserGenerator.GetSkeletonCap();
	if(nUsers > 0)
	{
		//std::cout<< "User detected love";
		verifyGesture();

	}

}

void  KFCVWrapper::nearUserDetect()
{
	double zNearest=9999;
	//std::map<int, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> userSkels;
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>>::iterator iter;
	for(iter = userSkels.begin();iter!=userSkels.end();iter++)
	{
		//std::cout << "VERIFYING USER\n";
		XnSkeletonJointPosition neckPos = iter->second.at(XN_SKEL_NECK);
		//std::cout << "position is " << neckPos.position.Z;
		if(neckPos.position.Z > 0){ // we have the position
			if(neckPos.position.Z < zNearest){	
				//std::cout <<focusNearestUser;
				if(&iter->first != currentUser && focusNearestUser){
					currentUser= &iter->first;
					m_gestureController1->calibrated=false;
						//std::cout <<"ANNOYING";
				}
				zNearest= neckPos.position.Z;
				currentUser= &iter->first;
				//std::cout << "user id is :" <<iter->first;
				userDetected=true;
			}
		}
	}
}

const XnUserID*  KFCVWrapper::verifyClosestUser()
{
	double zNearest=999;
	//std::map<int, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> userSkels;
	std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>>::iterator iter;
	for(iter = userSkels.begin();iter!=userSkels.end();iter++)
	{
		//std::cout << "VERIFYING USER\n";
		XnSkeletonJointPosition neckPos = iter->second.at(XN_SKEL_NECK);
		//std::cout << "position is " << neckPos.position.Z;
		if(neckPos.position.Z < zNearest){
			//	zNearest= neckPos.position.Z;
			//currentUser= *iter->first;
		}

	}
	return currentUser;
}
void  KFCVWrapper::verifyGesture(){
	xn::SkeletonCapability p_mSC =mUserGenerator.GetSkeletonCap();


	XnUInt16 nUsers = mUserGenerator.GetNumberOfUsers();
	XnUserID* aUserID = new XnUserID[nUsers];
	mUserGenerator.GetUsers(aUserID,nUsers); //not nUsers
	for(int i = 0; i<nUsers;++i)
	{
		if(p_mSC.IsTracking(aUserID[i]))
		{

			std::map<XnSkeletonJoint, XnSkeletonJointPosition> newuser;
			userSkels[aUserID[i]] = newuser;
			updateJoints(aUserID[i]);		
			if(!userDetected){
				this->nearUserDetect();
			}
			else{
				gestureDetector->detectGesture(*this->currentUser);
				//isLeanFwd=gestureDetector->isLeanFwd;
				//isLeanLeft=gestureDetector->isLeanLeft;
				//isLeanRight=gestureDetector->isLeanRight;
				//frontLean=gestureDetector->frontLean;
				drawMovementRectangle(gestureDetector->horizontalLean,gestureDetector->frontLean);
			}

		}
	}
}

void  KFCVWrapper::_boostThread(){
	CvFont font;
	//Mat *rgbImage = new Mat(480, 640, CV_8UC3, Scalar::all(0));
	//Mat *depthImage = new Mat(480, 640, CV_8UC1, Scalar::all(0));
//	Mat *comboImage = new Mat(480, 640, CV_8UC3, Scalar::all(0));
	//Mat *rawDepth = new Mat(480, 640, CV_16UC1, Scalar::all(0));
	//	namedWindow( "RGB");
	//namedWindow( "Depth" ); //Ventana de la imagen
	if(!init("SamplesConfig.xml")) exit(50);
	int i =0;
	while(runKinect){
		if(update()){
			//getDisplayDepth(depthImage);
			//cv::imshow("Depth", *depthImage);
			//cv::imshow("RGB", *rgbImage);
		}
		int k = waitKey(25);
		if(k == 27 ) exit(0);
	}

	// KFCVWrapper *wrapper =  KFCVWrapper::getInstance();

}

void  KFCVWrapper::updateJoints(XnUserID userID){
	/*
	cv::Point3f neckPt = getJointPos(skel, XN_SKEL_NECK);
	cv::Point3f  shoulderPt = getJointPos(skel, XN_SKEL_RIGHT_SHOULDER);
	cv::Point3f  handPt = getJointPos(skel, XN_SKEL_RIGHT_HAND); XN_SKEL_HEAD;
	cv::Point3f  elbowPt = getJointPos(skel, XN_SKEL_RIGHT_ELBOW);
	*/
	// vmap<XnSkeletonJoint, XnSkeletonJointPosition> &skel = userSkels.at(userID);
	updateJoint(userID, XN_SKEL_HEAD);
	updateJoint(userID, XN_SKEL_NECK);
	updateJoint(userID, XN_SKEL_TORSO);

	updateJoint(userID, XN_SKEL_RIGHT_SHOULDER);
	updateJoint(userID, XN_SKEL_LEFT_SHOULDER);

	updateJoint(userID, XN_SKEL_RIGHT_HAND);
	updateJoint(userID, XN_SKEL_LEFT_HAND);

	updateJoint(userID, XN_SKEL_LEFT_HIP);
	updateJoint(userID, XN_SKEL_RIGHT_HIP);

	updateJoint(userID, XN_SKEL_RIGHT_KNEE);
	updateJoint(userID, XN_SKEL_RIGHT_FOOT);

	updateJoint(userID, XN_SKEL_LEFT_KNEE);
	updateJoint(userID, XN_SKEL_LEFT_FOOT);

	updateJoint(userID, XN_SKEL_LEFT_ELBOW);
	updateJoint(userID,XN_SKEL_RIGHT_ELBOW);
}
void  KFCVWrapper::updateJoint(XnUserID userID, XnSkeletonJoint joint){
	XnSkeletonJointPosition myjoint;
	XnPoint3D proj_point;
	mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(userID,joint,myjoint);
	g_depth.ConvertRealWorldToProjective(1,&myjoint.position,&proj_point);
	userSkelsReal.at(userID)[joint] =myjoint;

	XnSkeletonJointPosition joint_projective;
	joint_projective.fConfidence=0;
	joint_projective.position=proj_point;
	userSkels.at(userID)[joint] =joint_projective;
}


void  KFCVWrapper::drawMovementRectangle(int x,int y){
	if(abs(oldLeanFront-y)>20)
		oldLeanFront=y;	

	//cv::rectangle(*this->_depthImage,cv::Rect(0,0,100,100),cv::Scalar(255,255,0));
	//cv::rectangle(*this->_depthImage,cv::Rect(50-10,50+60-oldLeanFront,20,20),cv::Scalar(255,255,255),4);

}
;