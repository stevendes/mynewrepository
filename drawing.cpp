#include <draw_turtle/drawing.hpp>

//This function is used to put the subscribed topic in a global variable in order to manipulate it

void Callbacks::poseCallback(const turtlesim::PoseConstPtr& pose)
{
	g_pose = pose;
}

//This function checks if the turtle has reached the goal

bool Mov_Ctrl::hasReachedGoal()
{
	return fabsf(g_pose->x - g_goal.x) < 0.1 && fabsf(g_pose->y - g_goal.y)
	< 0.1 	&& fabsf(g_pose->theta - g_goal.theta) < 0.01;
}

//This function checks if the velocity of the turtle is almost 0

bool Mov_Ctrl::hasStopped()
{
	return g_pose->angular_velocity < 0.0001 && g_pose->linear_velocity 
	< 0.0001;
}

//This function print the current goal

void Mov_Ctrl::printGoal()
{
	ROS_INFO("New goal [%f %f, %f]", g_goal.x, g_goal.y, g_goal.theta);
}

//This function receives the input of the velocities in order to publish it in the topic

void Asignation::commandTurtle(ros::Publisher twist_pub, float linear, float angular)
{
	asignation.twist.linear.x = linear;
	asignation.twist.angular.z = angular;
	twist_pub.publish(asignation.twist);
}

//The calculations of the goals we do in the "polygon_det.Turn_Calculation" function,
// besides that, the function change the State of the turtle or stop his movement 
 
void Movement::stopForward(ros::Publisher twist_pub,ros::ServiceClient clear )
{
	if (mov_ctrl.hasStopped())
	{
		polygon_det.Turn_Calculation();
		ROS_INFO("Reached goal");
		g_state = TURN;
		mov_ctrl.printGoal();
	}
	else
	{
		asignation.commandTurtle(twist_pub, 0, 0);
	}
 }
 
//The counter "mov_ctrl.forward_counter++" is very important, we use this to 
// count how many stops we had since the program star, so we know when to change
// the polygon we are drawing, that decision is made in the polygon_det.Determination 
// function

void Movement::stopTurn(ros::Publisher twist_pub,ros::ServiceClient clear)
{
	if (mov_ctrl.hasStopped())
	{	
		mov_ctrl.forward_counter++;
		polygon_det.Determination(mov_ctrl.forward_counter, clear);
		polygon_det.Forward_Calculation();
		ROS_INFO("Reached goal");
		g_state = FORWARD;
		mov_ctrl.printGoal();
   	}
   	else
   	{
     		asignation.commandTurtle(twist_pub, 0, 0);
   	}
 }

//This function is used in order to have linear movement

void Movement::forward(ros::Publisher twist_pub)
{
	if (mov_ctrl.hasReachedGoal())
	{
		g_state = STOP_FORWARD;
		asignation.commandTurtle(twist_pub, 0, 0);
	}
	else
	{
		asignation.commandTurtle(twist_pub, 1.0, 0.0);
	}
}

//This function is used in order to have angular movement

void Movement::turn(ros::Publisher twist_pub)
{
	if (mov_ctrl.hasReachedGoal())
	{
		g_state = STOP_TURN;
		asignation.commandTurtle(twist_pub, 0, 0);
	}
	else
	{
		asignation.commandTurtle(twist_pub, 0.0, 0.4);
	}
}

//This functions is used in order to set the first goal and check every specified amount time
// what is the state, and with that information update the publisher

void Callbacks::timerCallback(const ros::TimerEvent&, ros::Publisher twist_pub,ros::ServiceClient clear)
{
	if (!g_pose)
	{
		return;
   	}
 
   	if (!g_first_goal_set)
   	{
		g_first_goal_set = true;
		g_state = FORWARD;
		g_goal.x = cos(g_pose->theta) * 2 + g_pose->x;
		g_goal.y = sin(g_pose->theta) * 2 + g_pose->y;
		g_goal.theta = g_pose->theta;
		mov_ctrl.printGoal();
	}
 
	if (g_state == FORWARD)
	{
		movement.forward(twist_pub);
	}
	else if (g_state == STOP_FORWARD)
	{
		movement.stopForward(twist_pub,clear);
	}
	else if (g_state == TURN)
	{
		movement.turn(twist_pub);
	}
	else if (g_state == STOP_TURN)
	{
		movement.stopTurn(twist_pub,clear);
	}
}


//This function is used to determine what is the polygon that we are going to draw, 
// starts with a square and as soon as is complete, pass to another polygon, also 
// clears the screen every time a polygon is finished

void Polygon_Det::Determination(int turn_counter, ros::ServiceClient clear)
{
	if(turn_counter<4)
	{
		current_pol=SQUARE;
	}
	else if (turn_counter<10)
	{
		current_pol=HEXAGON;
		if(turn_counter==4)
		{
			clear.call(empty);
		}
	}
	else if (turn_counter<14)
	{
		current_pol=TRAPEZOID;
		if(turn_counter==10)
		{		
			clear.call(empty);
		}
	}
	else
	{
		current_pol=STAR;
		if(turn_counter==14)
		{
		clear.call(empty);
		}
	}
}

//This function is used in order to calculate the turns, uses the Polygon enumeration 
// to set what variables will be changing according to what turn you need, also has some
// extra counters for the more complicated polygons
	
void Polygon_Det::Turn_Calculation()
{
	if (current_pol==SQUARE)
	{
		g_goal.x = g_pose->x;
		g_goal.y = g_pose->y;
		g_goal.theta = fmod(g_pose->theta + PI/2.0, 2*PI);
	}
	else if (current_pol==HEXAGON)
	{
		g_goal.x = g_pose->x;
		g_goal.y = g_pose->y;
		g_goal.theta = fmod(g_pose->theta + PI/3.0, 2*PI);
	}
	else if (current_pol==TRAPEZOID)
	{
		if(trapezoid_turn_counter==0)
		{
			g_goal.x = g_pose->x;
    			g_goal.y = g_pose->y;
    			g_goal.theta = fmod(g_pose->theta + PI/3.0, 2*PI);
			trapezoid_turn_counter++;
		}
		else if(trapezoid_turn_counter<4)
		{	
			g_goal.x = g_pose->x;
			g_goal.y = g_pose->y;
			g_goal.theta = fmod(g_pose->theta + 2*PI/3.0, 2*PI);
			trapezoid_turn_counter++;
		}
	}
	else  if( current_pol==STAR)
	{
		if(star_turn_counter==0)
		{
			g_goal.x = g_pose->x;
			g_goal.y = g_pose->y;
			g_goal.theta = fmod(g_pose->theta + 4*PI/3.0, 2*PI);
			star_turn_counter++;
		}
		
		else if(star_turn_counter==1 ||star_turn_counter==3 || star_turn_counter==5)
		{
			g_goal.x = g_pose->x;
			g_goal.y = g_pose->y;
			g_goal.theta = fmod(g_pose->theta + PI/6.0, 2*PI);
			star_turn_counter++;
		}
		
		else if (star_turn_counter==2 || star_turn_counter==4 ||star_turn_counter==6 ) 
		{
			g_goal.x = g_pose->x;
			g_goal.y = g_pose->y;
			g_goal.theta = fmod(g_pose->theta + 4*PI/3.0, 2*PI);
			star_turn_counter++;
		}	
	}	
}
	

//Similar to his counterpart, this function is used to calculate the forward movement, 
// there are some counters in order to help the more complex figures and the variables 
// also depend on what polygon we are drawing at the moment

void Polygon_Det::Forward_Calculation()
{
	if (current_pol==SQUARE || current_pol==HEXAGON)
	{
		g_goal.x = cos(g_pose->theta) * 2 + g_pose->x;
		g_goal.y = sin(g_pose->theta) * 2 + g_pose->y;
		g_goal.theta = g_pose->theta;
	}
	else if (current_pol==TRAPEZOID)
	{
		if (trapezoid_forward_counter==0 || trapezoid_forward_counter==1)
		{
			g_goal.x = cos(g_pose->theta) * 2 + g_pose->x;
	     		g_goal.y = sin(g_pose->theta) * 2 + g_pose->y;
	     		g_goal.theta = g_pose->theta;
			trapezoid_forward_counter++;
		}
		else if (trapezoid_forward_counter==2)
		{
			g_goal.x = cos(g_pose->theta) *(2+2*0.5*2) + g_pose->x;
     			g_goal.y = sin(g_pose->theta) *(2+2*2*sqrt(3)/2) + g_pose->y;
     			g_goal.theta = g_pose->theta;
			trapezoid_forward_counter++;
		}
		else if (trapezoid_forward_counter==3)
		{
			g_goal.x = cos(g_pose->theta) * 2 + g_pose->x;
	     		g_goal.y = sin(g_pose->theta) * 2 + g_pose->y;
	     		g_goal.theta = g_pose->theta;
			
		}
	}
	else if(current_pol==STAR)
	{		if(star_forward_counter<8)
			{
				g_goal.x = cos(g_pose->theta) * 2 + g_pose->x;
				g_goal.y = sin(g_pose->theta) * 2 + g_pose->y;
				g_goal.theta = g_pose->theta;
				star_forward_counter++;
			}
			else
			{
				g_goal.x = g_pose->x;
				g_goal.y = g_pose->y;
				g_goal.theta = g_pose->theta;
			}
	}
}
