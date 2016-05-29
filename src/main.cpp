#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <utility>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <vector>
#include <fstream>
#include <pthread.h>
#include <SDL.h>
#include "landscape.h"
#include "ship.h"
#include "math.h"
#include "beacon.h"
#include "include/irrKlang.h"


#include "data.h"
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define SCREEN_BPP     16

using namespace std;

float xrot=0,yrot=0,zrot=0,ud=0.0f,scal = 100.0f,max =0;
float djl=0.0f,dik=-50.0f;
float dist =0.0f;
float check;
float siz=1.0f;
int done=0,sz = 0;
float play_freq;
int completed = 0;
SDL_Surface *surface;

vector<struct play_pos *> play_positions;

struct sound_sample{
    char * buf;
    int n;
};

struct sound_sample * sound_buffers;

struct play_pos{
    float x;
    float z;
    float h;
    float horiz_rot;
    float height_h;

};

void Quit( int returnCode ) {
    SDL_Quit( );
    exit( returnCode );
}

int resizeWindow( int width, int height ) {
    GLfloat ratio;
    if (height == 0)
        height = 1;

    ratio = (GLfloat)width / (GLfloat)height;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0f,ratio, 0.1f, 400.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return 1;
}


int initGL() {

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    return 1;
}

float dot(float a1,float  a2,float b1,float b2){
    return a1*b1+a2*b2;
}

float size(float a1,float  a2,float b1,float b2){
    return sqrtf((a1-b1)*(a1-b1) + (a2-b2)*(a2-b2));
}

int build_sound_buf(char * buf, int len, float freq){ // len == period

    float sample = 8000;
    int i;
    float cycles = 3;
    int period  = (int) floorf(sample / freq * cycles);

    for (i=0;i < period;i++){
        char c = 128 + sinf(cycles * 2 * M_PI  * i / period) * 128;
        buf[i] = c;
    }
    return period;
}

int get_sound_buf(struct sound_sample * ss,float freq){ // len == period

    float sample = 8000;
    int i;
    float cycles = 3;
    int period  = (int) floorf(sample / freq * cycles);
    ss->buf = (char*) malloc(period);
    ss->n = period;
    for (i=0;i < period;i++){
        char c = 128 + sinf(cycles * 2 * M_PI  * i / period) * 128;
        ss->buf[i] = c;
    }
    return 1;
}

int gameLogic(landscape &l, ship & s,beacon & b, int file_dec, irrklang::ISoundEngine * engine){
    float dist = s.test_forward_collision(l,0);
    float distl = s.test_forward_collision(l, -1*M_PI/6); //left
    float distr = s.test_forward_collision(l, M_PI/6); //right

    float theta = s.get_angle_diff(b.x1,b.y1);
    float dir=1;

    if (theta >1.570796 ){
        irrklang::vec3df pos(-1, 0,1); // -1 = left , 1 =right
        dir=-1;
    }
    else{
        irrklang::vec3df pos(1, 0,1); // -1 = left , 1 =right
        dir=1;
    }

    irrklang::vec3df pos(dir, 0,1); // -1 = left , 1 =right
    check+=0.3;

    if (dist > 0 && check > dist/3){
        float diff = abs(distl - distr);
        if (diff < 10.0f){
            dir = 0;
        }
        else if(distl<distr){
            dir = 1;
        }
        else{
            dir = -1;
        }
        irrklang::vec3df pos(dir, 0,1); // -1 = left , 1 =right
        engine->play3D("radar.wav", pos);
        check=0;
    }

    // This code has to be disabled as the method i was using to
    // play pitch frequencies is no longer viable (writing to /dev/dsp)
    float k=50.0f/l.maxval;
    //char buf[1024];
    float test =(s.y)  - l.heightmap[(int)s.x][(int)s.z]*k;
    if(test >0){
        // int n = build_sound_buf(buf,1204,165 + 5*test*test);
        // int written = 0;
        // for (written=0;written < 1000;written+=n){
        //test*=test;
        test*=8;
        if (test > 600)
            test=600;
        //int num =  min(sound_buffers[(int)test].n,200);
        play_freq = test;
        //int num = sound_buffers[(int)test].n;
        //    write(file_dec, sound_buffers[(int)test].buf,num);
        // }
    }


    struct play_pos * newp  = (struct play_pos * ) malloc(sizeof(struct play_pos ));
    newp->x= s.x;
    newp->z = s.z;
    newp->h = s.y;
    newp->horiz_rot = s.horiz_rot;
    newp->height_h = l.heightmap[(int)s.x][(int)s.z]*k;
    play_positions.push_back(newp);

    return 0;
}

int drawGLScene(landscape &l, ship & s ,beacon &b)
{
    static GLint T0     = 0;
    static GLint Frames = 0;
    float k=50.0f/l.maxval;
    float k2 = 0.8f/l.maxval;


    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();


    float c1 = s.x - l.width/2.0f;
    float c2 =/*l.heightmap[(int)s.x][(int)s.y]*k+1.0f*/ s.y;
    float c3 = s.z - l.width/2.0f;
    gluLookAt(c1, c2 , c3,   c1 + cosf(s.horiz_rot), c2,  c3+sinf(s.horiz_rot),0, 1, 0);
    int granul = 10;

    for (int i =0;i < l.width-granul;i+=1){
        for (int j=0;j< l.height-granul;j+=1){
            float d = sqrtf((i-s.x)*(i-s.x) + (j-s.z)*(j-s.z));
            if (d < 30){
                granul = 1;
            }
            else{
                granul = 10;
            }
            if (i%granul ==0 && j%granul==0){
                glBegin(GL_TRIANGLES);
                    glColor3f(k2*l.heightmap[i][j],0.0f,0.5f);
                    glVertex3f( i- l.width/2.0f, l.heightmap[i][j]*k, j- l.width/2.0f);
                    glColor3f(k2*l.heightmap[i+granul][j],0.0f,0.5f);
                    glVertex3f(i+granul- l.width/2.0f, l.heightmap[i+granul][j]*k,j- l.width/2.0f);
                    glColor3f(k2*l.heightmap[i+granul][j+granul],0.0f,0.5f);
                    glVertex3f( i+granul- l.width/2.0f,l.heightmap[i+granul][j+granul]*k,j+granul- l.width/2.0f );
                glEnd();

                glBegin(GL_TRIANGLES);
                    glColor3f(k2*l.heightmap[i][j],0.0f,0.5f);
                    glVertex3f( i- l.width/2.0f,l.heightmap[i][j]*k, j- l.width/2.0f );
                    glColor3f(k2*l.heightmap[i][j+granul],0.0f,0.5f);
                    glVertex3f(i- l.width/2.0f,l.heightmap[i][j+granul]*k,j+granul- l.width/2.0f) ;
                    glColor3f(k2*l.heightmap[i+granul][j+granul],0.0f,0.5f);
                    glVertex3f( i+granul- l.width/2.0f,l.heightmap[i+granul][j+granul]*k,j+granul- l.width/2.0f );
                glEnd();
                if (i < (l.width -1) && j < (l.height -1)){
                    glBegin(GL_LINES);
                        glColor3f(1.0f,1.0f,1.0f);
                        glVertex3f( i- l.width/2.0f,l.heightmap[i][j]*k, j- l.width/2.0f );
                        glVertex3f( i+granul- l.width/2.0f,l.heightmap[i+granul][j]*k,   j- l.width/2.0f );
                        glVertex3f( i- l.width/2.0f,l.heightmap[i][j]*k, j- l.width/2.0f );
                        glVertex3f( i- l.width/2.0f,l.heightmap[i][j+granul]*k,   j+granul- l.width/2.0f );
                    glEnd();
                }
            }
        }
    }
    glBegin(GL_TRIANGLES);
        glColor3f(0.0f,1.0f,0.0f);
        glVertex3f( b.x1- l.width/2.0f, l.heightmap[(int)b.x1][(int)b.y1]*k + 1.0f, b.y1- l.width/2.0f);
        glVertex3f(b.x1- l.width/2.0f,l.heightmap[(int)b.x1][(int)b.y2]*k+1.0f,b.y2- l.width/2.0f );
        glVertex3f( b.x2- l.width/2.0f,l.heightmap[(int)b.x2][(int)b.y2]*k+1.0f,b.y2- l.width/2.0f );
    glEnd();

    SDL_GL_SwapBuffers( );

    Frames++;
    GLint t = SDL_GetTicks();
    if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
        GLfloat fps = Frames / seconds;
        //	    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
        T0 = t;
        Frames = 0;
    }


    return 1;
}

void  sound_loop(void *ptr){
    // Code written to play frequencies, disabled.

    int front_buffer_size = 1000;
    char * front_buffer = (char * ) malloc (front_buffer_size);

    while(1){
        /*float pf = play_freq;
          int num = sound_buffers[(int)pf].n;
          int written;
          if (num > 0){
          for(written =0; written < 1000;written+=num){
          write(sound_file_dec, sound_buffers[(int)pf].buf,num);
        //		    char * b =  (char * ) malloc(10000);
        //		    for (int i =0; i <000;i++){
        //			    b[i] = radar[2*i + 584*8];
        }*/
        //
        /*	char * b = (char *) malloc(sizeof(radar)/2);
            for (int i =0;i < sizeof(radar)/2;i+=1){
            unsigned short* x = (unsigned short*) &radar[2*i ];
            b[i] = ((*x)*255)/65535 + 128;//(char) ((*x)>>8 + 128);
        //    printf("%hi\n",(*x));

        }
        write(sound_file_dec, b ,sizeof(radar)/2);
        sleep(1);*/
        //			write(sound_file_dec,b,10000);
        //		}
        //	}
        //printf("play_freq: %f",play_freq);
    }
}

struct sound_sample * init_sound_buffers(float lowfreq,float highfreq,float step){

    int num_buf = (highfreq - lowfreq + 1) / step;
    struct sound_sample * buffers =  (struct sound_sample *) calloc(num_buf,sizeof(struct sound_sample));
    float f = (float) lowfreq;
    for (int i = 0;i < num_buf;i++){
        get_sound_buf(&buffers[i],f);
        f+=step;
    }
    return buffers;
}

void handleKeyPress( landscape & l,ship &s) {
    Uint8* keys;

    keys = SDL_GetKeyState(NULL);

    // Rotate the screen
    if ( keys[SDLK_LEFT] == SDL_PRESSED ) {
        yrot--;
        if(yrot<0) yrot+=360;
    }
    if ( keys[SDLK_RIGHT] == SDL_PRESSED ) {
        yrot++;
        if(yrot>360) yrot-=360;
    }
    if ( keys[SDLK_UP] == SDL_PRESSED ) {
        xrot--;
        if(xrot<0) xrot+=360;
    }
    if ( keys[SDLK_DOWN] == SDL_PRESSED ) {
        xrot++;
        if(xrot>360) xrot-=360;
    }

    if(keys['j'] == SDL_PRESSED){
        djl+=1.0f;
    }
    if(keys['l'] == SDL_PRESSED){
        djl-=1.0f;
    }
    if(keys['i'] == SDL_PRESSED){
        dik+=1.0f;
    }
    if(keys['k'] == SDL_PRESSED){
        dik-=1.0f;
    }

    if ( keys[SDLK_PAGEUP] == SDL_PRESSED ) {
        if(siz>0) siz-=0.1;
    }
    if ( keys[SDLK_PAGEDOWN] == SDL_PRESSED ) {
        siz+=0.1;
    }
    if ( keys[SDLK_HOME] == SDL_PRESSED ) {
        if(dist>0) dist-=0.1;
    }
    if ( keys[SDLK_END] == SDL_PRESSED ) {
        dist+=0.1;
    }
    if (keys[SDLK_F1] == SDL_PRESSED){
        SDL_WM_ToggleFullScreen( surface );
    }
    if (keys[SDLK_ESCAPE] == SDL_PRESSED){
        completed=1;
    }
    if (keys['y'] == SDL_PRESSED){

        l.minval=20.0f;
        l.maxval=80.0f;
        l.build(0.0f,0.0f,l.width,20.0f,40.0f,60.0f,80.0f);
        l.inc_all(-1.0f*l.minval);
        l.maxval-=l.minval;
        l.minval=0;
    }
    if (keys['t'] == SDL_PRESSED){
        l.minval=0;
        l.maxval=0;
        l.read_pgm((char*)"arch.pgm");
        //l.inc_all(-1.0f* l.minval);
    }
    if (keys['g'] == SDL_PRESSED){
        l.write_pgm((char *) "out.pgm");
    }

    if(keys['u'] == SDL_PRESSED){
        // Generate a new heightmap
        l.minval=0;
        l.maxval=0;
        l.build_faultline(10000);
        l.inc_all(-1.0f*l.minval);
        l.maxval-=l.minval;
        l.minval=0;
    }
    if(keys['w'] == SDL_PRESSED) {
        // s.y+=0.5f;
        // s.update_position(1.0f);
        s.velocity+=1.0f;
        if (s.velocity >= 2.0f)
            s.velocity = 2.0f;
    }
    if(keys['s'] == SDL_PRESSED) {
        // s.update_position(-1.0f);
        s.velocity-=1.0f;
        if (s.velocity <= -2.0f)
            s.velocity = -2.0f;
    }
    if(keys['a'] == SDL_PRESSED) {
        s.horiz_rot-=0.05f;
    }
    if(keys['d'] == SDL_PRESSED) {
        s.horiz_rot+=0.05f;
    }
    if(keys['h'] == SDL_PRESSED) {
        s.thrust+=0.01f;
    }
    if(keys['b'] == SDL_PRESSED) {
        s.thrust-=0.01f;
    }
}



int main( int argc, char **argv )
{

   if (argc < 7 ){
       printf ("Usage %s: [MAP_FILE] [STARTX]  [STARTZ] [POINTING] [SLEEP_VALUE] [PARTICIPANT_NAME]",argv[0]);
       return 0;
   }
    int videoFlags;
    int done = 0;
    SDL_Event event;
    const SDL_VideoInfo *videoInfo;
    int start_time = time(NULL);


    landscape l;
    l.minval=0;
    l.maxval=0;
    l.read_pgm((char*)argv[1]);

    float shipx = atof(argv[2]);
    float shipz = atof(argv[3]);
    float pointing = atof(argv[4]);
    float shiph = l.heightmap[(int)shipx][(int)shipz] + 10.0f;
    int sleep_val = atoi(argv[5]);
    ship s(shipx,shiph,shipz,pointing);
    beacon b(120.0f,120.0f,130.0f,130.0f);
    irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice(irrklang::ESOD_ALSA );
    engine->setListenerPosition(irrklang::vec3df(0,0,0), irrklang::vec3df(0,0,1));
    play_positions = vector<struct play_pos *> ();


    pthread_t sound;
    sound_buffers = init_sound_buffers(60,600,1);
    /*
     * Disabled for now
    pthread_create(&sound,NULL, (void *(*)(void *)) &sound_loop,(void *) NULL);
    */


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Video initialization failed: %s\n",
                SDL_GetError());
        Quit( 1 );
    }

    videoInfo = SDL_GetVideoInfo();

    if (!videoInfo) {
        fprintf(stderr, "Video query failed: %s\n",
                SDL_GetError());
        Quit(1);
    }

    videoFlags  = SDL_OPENGL;
    videoFlags |= SDL_GL_DOUBLEBUFFER;
    videoFlags |= SDL_HWPALETTE;
    videoFlags |= SDL_RESIZABLE;

    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
            videoFlags );

    if (!surface) {
        fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
        Quit(1);
    }

    initGL();

    resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    GLint old_time,new_time,dt;
    old_time = SDL_GetTicks();
    while (!completed) {
        handleKeyPress(l,s);
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_VIDEORESIZE:
                    surface = SDL_SetVideoMode( event.resize.w,
                            event.resize.h,
                            16, videoFlags );
                    if (!surface) {
                        fprintf(stderr, "Could not get a surface after resize: %s\n", SDL_GetError());
                        Quit(1);
                    }
                    resizeWindow(event.resize.w, event.resize.h);
                    break;
                case SDL_KEYDOWN:
                    break;
                case SDL_QUIT:
                    done = 1;
                    break;
                default:
                    break;
            }
        }
        new_time = SDL_GetTicks();
        dt = new_time - old_time;
        old_time=new_time;

        gameLogic(l,s,b,0, engine);
        s.update_position(0.2f);

        drawGLScene( l,s,b);
        usleep(sleep_val);
    }

    char buf[200];
    snprintf(buf,200,"DATA/%s__%s__%i.OUTPUT",argv[1],argv[6],start_time);
    cout<<"Writing output file "<<buf<<endl;

    ofstream out(buf);
    for (int m =0;m < play_positions.size();m++){
        out << play_positions[m]->x<<" "<<play_positions[m]->z<<" "<<play_positions[m]->h<<" "<<play_positions[m]->height_h<<" "<<play_positions[m]->horiz_rot<<endl;
    }
    out.close();
    Quit(0);
    return 0;
}

