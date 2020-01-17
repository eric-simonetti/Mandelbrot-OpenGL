#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <math.h>

#include <fstream>

#include "Angel.h"


#define BUFFER_OFFSET(bytes) ((GLvoid*) (bytes))

using namespace std;

int scaleFactor = 4;
int baseIterations = 100;
int iterationInc = 50*scaleFactor;

int zoom = 0;
int testCutoff = baseIterations;

int width = 750, height = 750;
int numPoints = width*height;
int topBarHeight = 20;

//__float128 xMax = -.6, xMin = -.9;
//__float128 yMax = .25, yMin = 0;

__float128 xMax = 1.0, xMin = -2.0;
__float128 yMax = 1.5, yMin = -1.5;
__float128 dx = xMax-xMin, dy = yMax-yMin;

float mx, my;


void init();
void display();
void mandelbrot(vec2 positions[], GLfloat colors[]);
void scale(vec2 positions[]);
void unScale(__float128 & mx, __float128 & my);
void mouseCallback(int button, int state, int x, int y);
void redisplay();
void mandelbrotQuad(vec2 positions[], GLfloat colors[]);

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-width)/2, (glutGet(GLUT_SCREEN_HEIGHT)-height)/2);

    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutCreateWindow("OpenGL Program");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouseCallback);

    glutMainLoop();
    return 0;
}

void init(){
    vec2 posData[numPoints];
    GLfloat colorData[numPoints];
    mandelbrot(posData, colorData);
    scale(posData);


    GLuint program = InitShader("shader.glsl","fshader.glsl");  //initializes vertex + fragment shaders
    glUseProgram(program);



    GLuint vao;                                                 //creates vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);



    GLuint buffer;                                              //create buffer to hold data
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);



    //allocate data
    glBufferData(GL_ARRAY_BUFFER, sizeof(posData)+sizeof(colorData), nullptr, GL_STATIC_DRAW);


    //sub in colorAttr data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colorData), colorData);

    //sub in posAttr data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(colorData), sizeof(posData), posData);



    GLuint posAttr = glGetAttribLocation(program, "vPosition");//getting address of vposition from shader to write vertex data
    glEnableVertexAttribArray(posAttr);
    glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(colorData)));


    GLuint colorAttr = glGetAttribLocation(program, "vColor");//getting address of vcolor from shader to write vertex data
    glEnableVertexAttribArray(colorAttr);
    glVertexAttribPointer(colorAttr, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glClearColor(1.0, 1.0, 1.0, 1.0);                          //turns screen white
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(1);
    glDrawArrays(GL_POINTS, 0, numPoints);
    glFlush();
}

void mandelbrot(vec2 positions[], GLfloat colors[]){
    //ofstream myfile;
    //myfile.open("data.txt");
    long double x = xMax, y = yMax;
    long double xStep = dx/width, yStep = dy/height;
    int count = 0;

    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            positions[count] = vec2(x, y);

            int iterations = 0;
            long double nextSquareAbs = 0;
            long double nextSquareX = x, nextSquareY = y;
            while(iterations < testCutoff && nextSquareAbs < 4.0){
                long double bufferX = nextSquareX*nextSquareX - nextSquareY*nextSquareY + x;
                nextSquareY = 2*nextSquareX*nextSquareY + y;
                nextSquareX = bufferX;

                nextSquareAbs = nextSquareX*nextSquareX + nextSquareY*nextSquareY;

                iterations++;
            }
            //myfile << " Count:"<< count << " X:" << positions[count].x << " Y:" << positions[count].y << " iterations:" << iterations << endl;

            colors[count] = (iterations == testCutoff)?0.0:iterations/100.0;

            y-=yStep;
            count++;
        }
        y = yMax;
        x-= xStep;
    }
}

void scale(vec2 positions[]){
    for(int i = 0; i < numPoints; i++){
        positions[i].x = -1.0+(2.0)*((positions[i].x-xMin)/dx);
        positions[i].y = -1.0+(2.0)*((positions[i].y-yMin)/dy);
    }
}

void unScale(__float128 & mx, __float128 & my){
    mx = xMin + dx * ((mx-(-1.0))/(2.0));
    my = yMin + dy * ((my-(-1.0))/(2.0));
}


void mouseCallback(int button, int state, int x, int y){
    mx = (__float128) x / (width / 2) - 1.0;
    my = (__float128) (height - y) / (height / 2) - 1.0;


    if((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) &&  state == GLUT_DOWN){
        __float128 mscalex = mx, mscaley = my;
//        cout << " mscaley: " << mscaley << " mscalex: " << mscalex << endl;

        unScale(mscalex, mscaley);

//        cout << " mscaley: " << mscaley << " mscalex: " << mscalex << endl;

        __float128 thisScale = scaleFactor;
        if(button == GLUT_LEFT_BUTTON){
            printf("left click\n");
            thisScale = 1.0/scaleFactor;
            zoom++;
        }
        else{
            printf("right click\n");
            zoom--;
        }

        xMax = mscalex+(dx/2.0)*thisScale;
        xMin = mscalex-(dx/2.0)*thisScale;
        dx = xMax-xMin;

        yMax = mscaley+(dy/2.0)*thisScale;
        yMin = mscaley-(dy/2.0)*thisScale;
        dy = yMax-yMin;

        testCutoff = baseIterations + iterationInc*zoom;
        if(testCutoff <= 0) testCutoff = baseIterations;

//        cout << " xMin: " << xMin << " xMax: " << xMax << " yMax: " << yMax << " yMin: " << yMin << " cutoff: " << testCutoff << endl;

        redisplay();
    }
}

void redisplay(){
    vec2 rawPosData[numPoints];
    GLfloat colorData[numPoints];

    cout << zoom << endl;

    if(zoom>=29) {
        cout << "running quad" << endl;
        mandelbrotQuad(rawPosData, colorData);
    }
    else mandelbrot(rawPosData, colorData);

    //sub in colorAttr data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colorData), colorData);

    glutPostRedisplay();
}

void mandelbrotQuad(vec2 positions[], GLfloat colors[]){
    ofstream myfile;
    myfile.open("data.txt");
    __float128 x = xMax, y = yMax;
    __float128 xStep = dx/width, yStep = dy/height;
    int count = 0;

    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            positions[count] = vec2(x, y);

            int iterations = 0;
            __float128 nextSquareAbs = 0;
            __float128 nextSquareX = x, nextSquareY = y;
            while(iterations < testCutoff && nextSquareAbs < 4.0){
                __float128 bufferX = nextSquareX*nextSquareX - nextSquareY*nextSquareY + x;
                nextSquareY = 2*nextSquareX*nextSquareY + y;
                nextSquareX = bufferX;

                nextSquareAbs = nextSquareX*nextSquareX + nextSquareY*nextSquareY;

                iterations++;
            }
            myfile << " Count:"<< count << " X:" << positions[count].x << " Y:" << positions[count].y << " iterations:" << iterations << endl;

            colors[count] = (iterations == testCutoff)?0.0:iterations/100.0;

            y-=yStep;
            count++;
        }
        y = yMax;
        x-= xStep;
    }
}

