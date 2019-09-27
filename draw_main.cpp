//Challenge for Ekumen Labs, by Milner Desvars


 #include <draw_turtle/drawing.hpp> 

 int main(int argc, char** argv)
 {
	//We initialize the node "draw_main

	ros::init(argc, argv, "draw_main");
	ros::NodeHandle nh;

	//We subscribe to the topic "turtle1/pose"

	ros::Subscriber pose_sub = nh.subscribe("turtle1/pose", 1,&Callbacks::poseCallback,&callbacks);

	//We create a publisher to interact with the turtle velocity

	ros::Publisher twist_pub = nh.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);
	
	//We create a service in order to reset the turtle every time we activate
	// the node	
	
	ros::ServiceClient reset = nh.serviceClient<std_srvs::Empty>("reset");

	//We create another service, in this case is to clear the screen
	// when we finish a polygon
	
	ros::ServiceClient clear = nh.serviceClient<std_srvs::Empty>("clear");
	
	//We set the timer so all the updates are done in a determinate 
	// space of time 
ros::Timer timer = nh.createTimer(ros::Duration(0.016), boost::bind(&Callbacks::timerCallback,&callbacks, _1, twist_pub,clear));
   
   reset.call(empty);
 
   ros::spin();
}
