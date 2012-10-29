#include <KinectGestures.h>
static KinectGestures *gestures = &KinectGestures::Instance();

void  KinectGestures::setUserGenerator(xn::UserGenerator& usergen){
	mUserGenerator = &usergen;
}
int KinectGestures::init()
{
	return 0;
}
void KinectGestures::detectGesture(XnUserID userID){
	currentSkel = userSkels->at(userID);
	if(currentSkel.empty())
		return;
	calculateHumanLength(currentSkel);
	leanForward();
	leanLeft();
	leanRight();
	//leanFwd(userID, skel);
	//leanBack(userID, skel);
	//std::cout << "I AM DETECTING\n";
}
void KinectGestures::calculateHumanLength(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel){
	cv::Point3d neckPt = getJointPos(skel, XN_SKEL_NECK);
	cv::Point3d  shoulderPt = getJointPos(skel, XN_SKEL_RIGHT_SHOULDER);
	cv::Point3d  handPt = getJointPos(skel, XN_SKEL_RIGHT_HAND); 
	cv::Point3d  elbowPt = getJointPos(skel, XN_SKEL_RIGHT_ELBOW);

	neckLength = distApart(neckPt, shoulderPt);    // neck to shoulder length
	armLength = distApart(handPt, shoulderPt);     // hand to shoulder length
	lowerArmLength = distApart(handPt, elbowPt);    // hand to elbow length


	//std::cout << neckPt.x;

	/*
	useage :
	HashMap<SkeletonJoint, SkeletonJointPosition> skel = userSkels.get(userID);
	if (skel == null)
	return;

	private void calcSkelLengths(HashMap<SkeletonJoint, SkeletonJointPosition> skel)
	Point3D neckPt = getJointPos(skel, SkeletonJoint.NECK);
	Point3D shoulderPt = getJointPos(skel, SkeletonJoint.RIGHT_SHOULDER);
	Point3D handPt = getJointPos(skel, SkeletonJoint.RIGHT_HAND);
	Point3D elbowPt = getJointPos(skel, SkeletonJoint.RIGHT_ELBOW);

	if ((neckPt != null) && (shoulderPt != null) &&
	(handPt != null) && (elbowPt != null)) {
	neckLength = distApart(neckPt, shoulderPt);    // neck to shoulder length
	// System.out.println("Neck Length: " + neckLength);

	armLength = distApart(handPt, shoulderPt);     // hand to shoulder length
	// System.out.println("Arm length: " + armLength);

	lowerArmLength = distApart(handPt, elbowPt);    // hand to elbow length
	// System.out.println("Lower arm length: " + lowerArmLength);
	*/
}
cv::Point3d KinectGestures::getJointPos(std::map<XnSkeletonJoint, XnSkeletonJointPosition> skel, XnSkeletonJoint j){
	if(skel.find(j) == skel.end()) 
		return NULL;
	XnSkeletonJointPosition pos = skel.at(j);

	//if (pos.fConfidence== 0) WTF
	//return NULL;
	return cv::Point3d(pos.position.X,pos.position.Y,pos.position.Z);
}
void KinectGestures::setUserSkels(std::map<XnUserID, std::map<XnSkeletonJoint,XnSkeletonJointPosition>> &userSkeletons){
	userSkels= &userSkeletons;
}
float KinectGestures::distApart(cv::Point3d& p1, cv::Point3d& p2)
{
	float dist =sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)+(p1.z-p2.z)*(p1.z-p2.z));
	return dist;
}
void KinectGestures::leanForward(){
	cv::Point3d torsoPt = getJointPos(currentSkel, XN_SKEL_TORSO);
	cv::Point3d headPt = getJointPos(currentSkel, XN_SKEL_HEAD);
	float zDiff = headPt.z - torsoPt.z;
	frontLean = zDiff*-1;
	//std::cout << "head:" << headPt.z;
	//	std::cout << "torso:" << torsoPt.z;
	//std::cout << "length:" << lowerArmLength << "\n";
	//std::cout <<"diference:" <<zDiff <<"\n";
	//std::cout << "armLength true is :" <<zDiff-lowerArmLength << "\n";
	//if (zDiff < -1*lowerArmLength) THIS IS NOT WORKING
	if (zDiff< -115) {    // head is forward
		if (!isLeanFwd) {
			isLeanFwd=true;
		}
	}  else {   // not forward
		if (isLeanFwd) {
			//watcher.pose(userID, GestureName.LEAN_FWD, false);  // stopped
			isLeanFwd = false;
		}
	}
	//std::cout << "leaning forward" << isLeanFwd << "\n";
}
void KinectGestures::leanBack(){
}
void KinectGestures::leanLeft(){
	cv::Point3d leftHipPt = getJointPos(currentSkel, XN_SKEL_LEFT_HIP);
	cv::Point3d headPt = getJointPos(currentSkel, XN_SKEL_HEAD);
		horizontalLean = headPt.x-leftHipPt.x;

	//float zDiff = headPt.z - torsoPt.z;
	if (headPt.x+10 <=leftHipPt.x) {    // head is forward
		if (!isLeanRight) {
			isLeanRight=true;
		}
	}  else {   // not forward
		if (isLeanRight) {
			//watcher.pose(userID, GestureName.LEAN_FWD, false);  // stopped
			isLeanRight = false;
		}
	}
	//std::cout << "leaning left" << isLeanLeft << "\n";
}
void KinectGestures::leanRight(){
	cv::Point3d rightHipPt = getJointPos(currentSkel, XN_SKEL_RIGHT_HIP);
	cv::Point3d headPt = getJointPos(currentSkel, XN_SKEL_HEAD);

	horizontalLean = headPt.x-rightHipPt.x;
	//float zDiff = headPt.z - torsoPt.z;
	if (rightHipPt.x <= headPt.x) {    // head is forward
		if (!isLeanLeft) {
			isLeanLeft=true;
		}
	}  else {   // not forward
		if (isLeanLeft) {
			//watcher.pose(userID, GestureName.LEAN_FWD, false);  // stopped
			isLeanLeft = false;
		}
	}
	//std::cout << "leaning right" << isLeanRight << "\n";
}
/*
private Point3D getJointPos(HashMap<SkeletonJoint, SkeletonJointPosition> skel,
SkeletonJoint j)
// get the (x, y, z) coordinate for the joint (or return null)
{
SkeletonJointPosition pos = skel.get(j);
if (pos == null)
return null;

if (pos.getConfidence() == 0)
return null;

return pos.getPosition();
}  // end of getJointPos()*/
;
