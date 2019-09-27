//The header file content is mostly the declarations of what we are going to use, this 
//includes the movement options, goals definitions and specific variables according to
//the polygon that we are drawing


#ifndef _DRAWING_HEADER_ 
#define _DRAWING_HEADER

#include <ros/ros.h>
#include <turtlesim/Pose.h>
#include <geometry_msgs/Twist.h>
#include <std_srvs/Empty.h>
#include <boost/bind.hpp>


turtlesim::PoseConstPtr g_pose;
turtlesim::Pose g_goal;

//The enumeration State objective is to know what movement we are doing next

enum State
{
	FORWARD,
	STOP_FORWARD,
	TURN,
	STOP_TURN,
};

//The enumeration Polygon objective is to declare what geometric figure we are drawing

enum Polygon
{
	SQUARE,
	HEXAGON,
	TRAPEZOID,
	STAR,
};

//We define these global variables to use in all the progra

State g_state = FORWARD;
State g_last_state = FORWARD;
bool g_first_goal_set = false; 
Polygon current_pol= SQUARE;

#define PI 3.141592

std_srvs::Empty empty;

//The class Movement main objective is to encapsulate all the movement functions
//for that purpose we create the "movement" object

class Movement
{
	public:
	void stopForward(ros::Publisher twist_pub, ros::ServiceClient clear);
	void stopTurn(ros::Publisher twist_pub, ros::ServiceClient clear);
	void forward(ros::Publisher twist_pub);
	void turn(ros::Publisher twist_pub);
}movement;

//The class Mov_Ctrl (Move Control) objective is to check if the turtle made to the
// objective, and if the movement has stopped, also prints the goal in every update
// and have a counter to check what polygon comes next,for this class we create the
// "mov_ctrl" object

class Mov_Ctrl
{
	public:
	bool hasReachedGoal();
	bool hasStopped();
	void printGoal();
	int forward_counter;
	void init(int forward_counter=0,int turn_counter=0);
}mov_ctrl;

//The class Asignation is what we use to set up the variables in order to publish in the 
//main topic, we create the "asignation" object for that purpose

class Asignation
{
	private:
	geometry_msgs::Twist twist;
	public:
	void commandTurtle(ros::Publisher twist_pub, float linear, float angular);
}asignation;

//A class to have the callbacks we are going to use, for that purpose we create the
//"callbacks" object

class Callbacks
{	public:
	void poseCallback(const turtlesim::PoseConstPtr& pose);
	void timerCallback(const ros::TimerEvent&, ros::Publisher twist_pub,ros::ServiceClient clear);
}callbacks;

//The class Polygon_Det is the most important, with this class we have the control about
//what polygon we are drawing, changing the goals according on what we are drawing,
//the class have some counters to help with the more complex drawings

class Polygon_Det
{	private:
	int trapezoid_turn_counter;
	int star_turn_counter;
	int trapezoid_forward_counter;
	int star_forward_counter;
	public:
	void init(int trapezoid_turn_counter=0,int star_turn_counter=0,
	          int trapezoid_forward_counter=0,int star_forward_counter=0);
	void Determination(int turn_counter,ros::ServiceClient clear);
	void Turn_Calculation();
	void Forward_Calculation();		
}polygon_det;

#endif
