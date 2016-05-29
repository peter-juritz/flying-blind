#include "landscape.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>

landscape::landscape(int w,int h,float ptrb,float ptrbpwr){

    srand(time(NULL));
    heightmap = new float *[w];
    width=w;
    height=h;
    maxval=0.0f;
    peturb_amount = ptrb;
    peturb_power = ptrbpwr;
    minval=0.0f;
    for (int i=0;i < w;i++){
        heightmap[i] = new float[h];
    }

}

landscape::landscape(){
}

void landscape::build_noisy_landscape(){
    srand(time(NULL));
    for (int i=0;i < width;i++){
        for (int j=0;j < height;j++){
            float r = ((float)rand()/(RAND_MAX + 1.0f)) -0.2f;
            heightmap[i][j]=/*(25-i)*(25-j)*/ 35.0f +  r*95.0f;
        }
    }
}

void landscape::build_faultline(int runs)
{
    //     for(int x = 0; x < width; ++x)
    //         fill(heightmap[x], heightmap[x]+height, 0.0);

    float** tmp = new float*[width];
    minval=0;
    maxval=0;

    for (int i=0; i < width; i++)
    {
        tmp[i] = new float[height];
        std::fill(tmp[i], tmp[i] + height, 0);
    }

    while(runs)
    {
        int p1x = rand() % width;
        int p1y = rand() % height;
        int p2x = rand() % width;
        int p2y = rand() % height;

        int d1x = p1x - p2x;
        int d1y = p1y - p2y;

        for(int y = 0; y < height; ++y)
            for(int x = 0; x < width; ++x)
            {
                int d2x = p1x - x;
                int d2y = p1y - y;

                int cp = d1x*d2y - d1y*d2x;

                if(cp < 0)
                    tmp[x][y] -= 0.25;
                else
                    tmp[x][y] += 0.25;
            }

        --runs;
    }

    float matrix[3][3] = {{0.25, 0.5, 0.25}, {0.5, 1.0, 0.5}, {0.25, 0.5, 0.25}};

    for(int x = 0; x < width; ++x)
        for(int y = 0; y < height; ++y)
        {
            float sum = 0.0;
            float value = 0.0;

            for(int dx = 0; dx < 3; ++dx)
                for(int dy = 0; dy < 3; ++dy)
                {
                    int ax = x - 1 + dx;
                    int ay = y - 1 + dy;

                    if(ax >= 0 && ax < width && ay >= 0 && ay < height)
                    {
                        sum += matrix[dx][dy];
                        value += matrix[dx][dy]*tmp[ax][ay];
                    }
                }

            heightmap[x][y] = value/sum;
            float k = heightmap[x][y];
            if (k > maxval)
                maxval=k;
            if (k < minval)
                minval=k;
        }

    for(int x = 0; x < width; ++x)
        delete [] tmp[x];

    delete [] tmp;

}

void landscape::read_pgm(char * f){

    char buf[128];
    std::ifstream in(f);
    in.getline(buf,128);
    //  printf("%s\n",buf);
    in.getline(buf,128);
    // printf("%s\n",buf);
    in>>width;
    in>>height;
    int max_val;
    in>>max_val;
    heightmap = new float * [width];
    for (int i=0;i < width;i++){
        heightmap[i] = new float[height];
    }

    float tmp;
    for (int h=0;h < height;h++){
        for (int w =0;w < width;w++){
            in>>tmp;
            tmp/= 40.0f;
            tmp=max_val/40 - tmp;
            //	    std::cout<<"TMP = "<<tmp<<std::endl;
            if (tmp > maxval)
                maxval=tmp;
            if (tmp < minval)
                minval=tmp;
            heightmap[w][h] = tmp ;
        }

    }
}



void landscape::write_pgm(char *f){
    std::ofstream out(f);
    out<<"P2"<<std::endl;
    out<<"# created by flying blind"<<std::endl;
    out<<(int)width<<" "<<(int)height<<std::endl;
    out<<(int)maxval<<std::endl;
    for (int h=0;h < height;h++){
        for (int w =0;w < width;w++){
            out<<(int)heightmap[w][h]<<std::endl;
        }
    }
    out.close();

}

float landscape::peturb(float w){
    float r =  ((float)rand()/(RAND_MAX + 1.0f)) -0.5f;
    return  powf(w,peturb_power)*r*peturb_amount;
}
float landscape::scale(float a, float b, float local_width){
    float t = (a+b)/2.0f;
    return t + peturb(local_width);
}
inline float landscape::scale2(float a, float b, float local_width){
    float t = (a+b)/2.0f;
    return t;
}
void landscape::build(float x,float y,float swidth,float a,float b,float c,float d){
    //float r =  ((float)rand()/(RAND_MAX + 1.0f)) -0.5f;
    float mean = (a+b+c+d + scale2(a,b,2*swidth) + scale2(b,c,2*swidth) + scale2(c,d,2*swidth) + scale2(a,d,2*swidth) )/8.0f;

    float set_val = mean + peturb(swidth);

    float k = set_val;
    if (swidth<=1.0f){
        heightmap[(int)roundf(x)][(int)roundf(y)] = k;

        if (k > maxval)
            maxval = k;

        if (k < minval)
            minval = k;
        return;
    }
    else{
        float inc = swidth/2.0f;
        build(x,y,inc,a,scale2(a,b,swidth),scale2(a,c,swidth),set_val);	// upper left

        build(x+inc,y,inc,scale2(b,a,swidth),b,set_val,scale2(b,d,swidth));	// upper right

        build(x,y+inc,inc,scale2(c,a,swidth),set_val,c,scale2(c,d,swidth));	// lower left

        build(x+inc,y+inc,inc,set_val,scale2(d,b,swidth),scale2(d,c,swidth),d);	// lower right
    }
}
void landscape::inc_all(float f){
    for (int i=0;i < width;i++){
        for (int j=0;j < height;j++){
            heightmap[i][j]+=f;
        }
    }
}

void landscape::set_equals(float f){
    for (int i=0;i < width;i++){
        for (int j=0;j < height;j++){
            heightmap[i][j]=f;
        }
    }
}

