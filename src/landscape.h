#ifndef LANDSCAPE_H
#define LANDSCAPE_H
#include <stdlib.h>
#include <time.h>
#include <utility>
#include <iostream>
#include <cmath>
class landscape{
    public:
	int width,height;
	float ** heightmap;
	float maxval,minval,peturb_amount,peturb_power;
	landscape(int w,int h,float prtb,float ptrbpwr);
        landscape();
	void build_noisy_landscape();
    void build_faultline(int runs);
	void inc_all(float f);
	void set_equals(float f);
	void build(float x,float y,float swidth,float a,float b,float c,float d);
	float peturb(float);
	float scale(float,float,float);
	float scale2(float,float,float);
	void read_pgm (char * );
	void write_pgm (char * );
};
#endif

