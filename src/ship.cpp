#include "ship.h"
#include "math.h"
#include "landscape.h"
#include <iostream>

ship::ship(float px,float py,float pz,float pointing){
    x = px;
    y = py;
    z =pz;
    // thrust=0.2f;
    thrust = 0;
    horiz_rot = pointing;
}

void ship::update_position(float t){
    vz=sinf(horiz_rot);
    vx=cosf(horiz_rot);
    z+=thrust*vz;
    x+=thrust*vx;
    if (velocity < 0){
        velocity +=0.2f;
        if (velocity >0)
            velocity=0;
    }
    else if (velocity > 0 ){
        velocity -=0.2f;
        if (velocity <0)
            velocity=0;
    }
    y+=0.2*velocity;
    //std::cout<<"(x,z) = ("<<t*cosf(horiz_rot)<<","<<t*sinf(horiz_rot)<<")"<<std::endl;
}

float ship::get_angle_diff(float ox,float oy){
    return horiz_rot - atanf( (oy-z)/(ox-x) );
}

float ship::test_forward_collision(landscape & l,float offset){
    float tvz=sinf(horiz_rot + offset);
    float tvx=cosf(horiz_rot + offset);

    float coef = 50.0f/l.maxval;
    float k = 0;
    while (k < 50){
        int xoff = (int)(x + k*tvx);
        int zoff = (int) (z + k*tvz);
        if (xoff > l.width || xoff < 0 || zoff > l.height || zoff <0)
            return -1.0f;
        float proj_h = l.heightmap[xoff][zoff]*coef;
        if(y <= proj_h){
            return k;
        }
        k+=1;
    }
    return -1.0f;
}
