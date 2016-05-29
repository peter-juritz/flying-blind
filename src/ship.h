#ifndef SHIP_H
#define SHIP_H
#include "landscape.h"

class ship{
    public:
	float x,y,z;
	float vz,vx;
	float horiz_rot,vert_rot;
	float velocity,thrust;
	ship(float posx,float posy,float posz,float point);
	void update_position(float t);
	float get_angle_diff(float x,float y);
	float test_forward_collision(landscape & l,float offset);

};
#endif
