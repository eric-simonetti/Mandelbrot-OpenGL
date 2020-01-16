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

int zoom = 0;
int testCuttoff = 100;

int width = 750, height = 750;
int numPoints = width*height;
int topBarHeight = 20;

//double xMax = -.6, xMin = -.9;
//double yMax = .25, yMin = 0;

double xMax = 1.0, xMin = -2.0;
double yMax = 1.5, yMin = -1.5;
double dx = xMax-xMin, dy = yMax-yMin;

double scaleFactor = 2.0;

GLfloat mx, my;


void init();
void display();
void mandelbrot(vec2 positions[], GLfloat colors[]);
void scale(vec2 positions[]);
void unScale(float & mx, float & my);
void mouseCallback(int button, int state, int x, int y);
void redisplay();

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

    cout << "2" << endl;

    GLuint program = InitShader("shader.glsl","fshader.glsl");  //initializes vertex + fragment shaders
    cout << "2.5" << endl;
    glUseProgram(program);

    cout << "3" << endl;

    GLuint vao;                                                 //creates vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    cout << "4" << endl;

    GLuint buffer;                                              //create buffer to hold data
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    cout << "5" << endl;

    //allocate data
    glBufferData(GL_ARRAY_BUFFER, sizeof(posData)+sizeof(colorData), nullptr, GL_STATIC_DRAW);

    cout << "6" << endl;

    //sub in posAttr data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posData), posData);

    cout << "7" << endl;

    //sub in colorAttr data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(posData), sizeof(colorData), colorData);

    cout << "8" << endl;

    GLuint posAttr = glGetAttribLocation(program, "vPosition");//getting address of vposition from shader to write vertex data
    glEnableVertexAttribArray(posAttr);
    glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    cout << "9" << endl;

    GLuint colorAttr = glGetAttribLocation(program, "vColor");//getting address of vcolor from shader to write vertex data
    glEnableVertexAttribArray(colorAttr);
    glVertexAttribPointer(colorAttr, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(posData)));

    glClearColor(1.0, 1.0, 1.0, 1.0);                          //turns screen white
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(10);
    glDrawArrays(GL_POINTS, 0, numPoints);
    glFlush();
}

void mandelbrot(vec2 positions[], GLfloat colors[]){
    ofstream myfile;
    myfile.open("data.txt");
    double x = xMax, y = yMax;
    double xStep = dx/width, yStep = dy/height;
    int count = 0;

    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            positions[count] = vec2(x, y);

            int iterations = 0;
            double nextSquareAbs = 0;
            double nextSquareX = x, nextSquareY = y;
            while(iterations < testCuttoff && nextSquareAbs < 4.0){
                double bufferX = nextSquareX*nextSquareX - nextSquareY*nextSquareY + x;
                nextSquareY = 2*nextSquareX*nextSquareY + y;
                nextSquareX = bufferX;

                nextSquareAbs = nextSquareX*nextSquareX + nextSquareY*nextSquareY;

                iterations++;
            }
            myfile << " Count:"<< count << " X:" << x << " Y:" << y << " iterations:" << iterations << endl;

            colors[count] = (iterations == testCuttoff)?0.0:iterations/100.0;

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

void unScale(float & mx, float & my){
    mx = xMin + dx * ((mx-(-1.0))/(2.0));
    my = yMin + dy * ((my-(-1.0))/(2.0));
}


void mouseCallback(int button, int state, int x, int y){
    mx = (float) x / (width / 2) - 1.0;
    my = (float) (height - y) / (height / 2) - 1.0;


    if((button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) &&  state == GLUT_DOWN){
        float mscalex = mx, mscaley = my;
        cout << " mscaley: " << mscaley << " mscalex: " << mscalex << endl;

        unScale(mscalex, mscaley);

        cout << " mscaley: " << mscaley << " mscalex: " << mscalex << endl;

        double thisScale = scaleFactor;
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

        testCuttoff = 100 + 100*zoom;
        if(testCuttoff <= 0) testCuttoff = 100;

        cout << " xMin: " << xMin << " xMax: " << xMax << " yMax: " << yMax << " yMin: " << yMin << " cutoff: " << testCuttoff << endl;

        redisplay();
    }
}

void redisplay(){
    vec2 posData[numPoints];
    GLfloat colorData[numPoints];
    mandelbrot(posData, colorData);
    scale(posData);

    //sub in posAttr data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posData), posData);

    //sub in colorAttr data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(posData), sizeof(colorData), colorData);

    glutPostRedisplay();
}

