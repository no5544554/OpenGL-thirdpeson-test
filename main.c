#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL2/SDL.h>

#include <GL/glu.h>
#include <GL/gl.h>

#include "defs.h"

struct
{
    SDL_Window * window;
    SDL_GLContext * glContext;

    uint16_t w;
    uint16_t h;
} gameWindow;

struct
{
    float camRot;

    float x;
    float y;
    float z;

    float h;
} player;

GLuint tex_grassGround;

BOOL running;
BOOL keys[SDL_NUM_SCANCODES];
BOOL keysPrev[SDL_NUM_SCANCODES];


void Step(void);
void PollEvents(void);
void Display(void);
void DrawCube(int x, int y, int z, Color32 * color, BOOL rotate);
void DrawArrayTest(int x, int y, int z);
void DrawSkybox(void);

int main(int argc, char * argv[])
{
    uint64_t start;
    uint64_t end = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();
    float ups = 1.0f / FPS;
    float delta = 0;
    float accumulator = 0;

    gameWindow.w = 1280;
    gameWindow.h = 720;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        char buffer[512];

        sprintf(buffer, "Error with SDL_Init! SDL_Error: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);

        return 1;
    }

    gameWindow.window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gameWindow.w, gameWindow.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (gameWindow.window == NULL)
    {
        char buffer[512];

        sprintf(buffer, "Error with SDL window creation! SDL_Error: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);

        return 2;
    }

    gameWindow.glContext = SDL_GL_CreateContext(gameWindow.window);
    if (gameWindow.glContext == NULL)
    {
        char buffer[512];

        sprintf(buffer, "Error with OpenGL context creation! SDL_Error: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);

        return 3;
    }


    /* load grass texture */
    glGenTextures(1, &tex_grassGround);
    glBindTexture(GL_TEXTURE_2D, tex_grassGround);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    {
        int x, y, n;
        unsigned char * data = stbi_load("textures/grass.png", &x, &y, &n, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    running = TRUE;
    while (running)
    {
        start = SDL_GetPerformanceCounter();
        delta = (double)(start - end) / freq;
        end = start;
        accumulator += delta;

        PollEvents();

        while (accumulator >= ups)
        {
            printf("Player X: %.2f, Y: %.2f, Z:%.2f\n", player.x, player.y, player.z);
            Step();
            accumulator -= ups;
        }
        Display();

        memcpy(keysPrev, keys, sizeof(keys));


        SDL_Delay(16);
    }




    return 0;
}


void PollEvents(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            running = FALSE;
            break;

        case SDL_KEYDOWN:
            keys[e.key.keysym.scancode] = TRUE;
            break;

        case SDL_KEYUP:
            keys[e.key.keysym.scancode] = FALSE;
            break;

        }
    }
}


void Step(void)
{
    float camRotRad = player.camRot * DEG_TO_RAD;
    float pspeed = 0.2;

    if ((keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) && (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]))
        pspeed *= VECTOR_NORMALIZE;

    if (keys[SDL_SCANCODE_ESCAPE])
    {
        running = FALSE;
    }

    if (keys[SDL_SCANCODE_LEFT])
    {
        player.camRot -= 1;
    }

    if (keys[SDL_SCANCODE_RIGHT])
    {
        player.camRot += 1;
    }


    if (keys[SDL_SCANCODE_A])
    {
        player.x -= pspeed * cos(camRotRad);
        player.z -= pspeed * sin(camRotRad);
    }
    else if (keys[SDL_SCANCODE_D])
    {
        player.x += pspeed * cos(camRotRad);
        player.z += pspeed * sin(camRotRad);
    }

    if (keys[SDL_SCANCODE_W])
    {
        player.x += pspeed * sin(camRotRad);
        player.z -= pspeed * cos(camRotRad);
    }
    else if (keys[SDL_SCANCODE_S])
    {
        player.x -= pspeed * sin(camRotRad);
        player.z += pspeed * cos(camRotRad);
    }
}


void Display(void)
{
    Color32 playerColor;
    Color32 levelColor;

    glClearColor(0.6f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glEnable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (double)gameWindow.w / gameWindow.h, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* draw sky box */
    glDisable(GL_DEPTH_TEST);
    DrawSkybox();
    glEnable(GL_DEPTH_TEST);

    /*---------------------------*/
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    /* position player */
    glTranslatef(0, 0, -10);
    glRotatef(20, 1, 0, 0);

    playerColor.r = 255;
    playerColor.g = 0;
    playerColor.b = 0;
    playerColor.a = 255;
    DrawCube(0, 0, 0, &playerColor, FALSE);



    /* position world */
    glRotatef(player.camRot, 0, 1, 0);
    glTranslatef(-player.x, -player.y, -player.z);

    /* draw floor */
    glPushMatrix();
    glTranslatef(0, -0.5, 0);
    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_grassGround);
    glBegin(GL_QUADS);
    glTexCoord2f( 0.0,  0.0);     glVertex3f(-50, 0,  50);
    glTexCoord2f( 0.0, 50.0);     glVertex3f(50,  0,  50);
    glTexCoord2f(50.0, 50.0);     glVertex3f(50,  0, -50);
    glTexCoord2f(50.0,  0.0);     glVertex3f(-50, 0, -50);

    glEnd();
    glPopMatrix();


    /* draw arrays */
    DrawArrayTest(-4, 0.75, 4);


    /* draw white cube */
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    levelColor.r = 255;
    levelColor.g = 255;
    levelColor.b = 255;
    levelColor.a = 255;
    DrawCube(4, 0, 4, &levelColor, FALSE);




    SDL_GL_SwapWindow(gameWindow.window);
}


void DrawCube(int x, int y, int z, Color32 * color, BOOL rotate)
{
    glPushMatrix();



    glTranslatef(x, y, z);
    if (rotate)
        glRotatef(45, 1, 0, 0);

    /* front */
    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glBegin(GL_QUADS);
    glVertex3f(-0.5, -0.5,  0.5);
    glVertex3f( 0.5, -0.5,  0.5);
    glVertex3f( 0.5,  0.5,  0.5);
    glVertex3f(-0.5,  0.5,  0.5);
    glEnd();

    /* right */
    glColor3f((float)color->r / 2 / 255, (float)color->g / 2 / 255, (float)color->b/ 2 / 255);
    glBegin(GL_QUADS);
    glVertex3f( 0.5, -0.5,  0.5);
    glVertex3f( 0.5, -0.5, -0.5);
    glVertex3f( 0.5,  0.5, -0.5);
    glVertex3f( 0.5,  0.5,  0.5);
    glEnd();

    /* rear */
    glColor3f((float)color->r / 255, (float)color->g / 255, (float)color->b / 255);
    glBegin(GL_QUADS);
    glVertex3f( 0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5,  0.5, -0.5);
    glVertex3f( 0.5,  0.5, -0.5);
    glEnd();

    /* left */
    glColor3f((float)color->r / 2 / 255, (float)color->g / 2 / 255, (float)color->b/ 2 / 255);
    glBegin(GL_QUADS);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5,  0.5);
    glVertex3f(-0.5,  0.5,  0.5);
    glVertex3f(-0.5,  0.5, -0.5);
    glEnd();

    /* top */
    glColor3f((float)color->r / 1.5f / 255, (float)color->g / 1.5f / 255, (float)color->b/ 1.5f / 255);
    glBegin(GL_QUADS);
    glVertex3f(-0.5,  0.5,  0.5);
    glVertex3f( 0.5,  0.5,  0.5);
    glVertex3f( 0.5,  0.5, -0.5);
    glVertex3f(-0.5,  0.5, -0.5);
    glEnd();

    /* bottom */
    glColor3f((float)color->r / 3 / 255, (float)color->g / 3 / 255, (float)color->b/ 3 / 255);
    glBegin(GL_QUADS);
    glVertex3f( 0.5, -0.5, -0.5);
    glVertex3f( 0.5, -0.5,  0.5);
    glVertex3f(-0.5, -0.5,  0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glEnd();

    glPopMatrix();
}


void DrawArrayTest(int x, int y, int z)
{
    static int rot = 0;

    float coords[] =
    {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    };

    float colors[] =
    {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
        1, 1, 0,
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
        1, 1, 0
    };

    int elementArray[] =
    {
        0, 3, 2, 1,
        1, 2, 6, 5,
        5, 6, 7, 4,
        4, 7, 3, 0,
        7, 6, 2, 3,
        4, 0, 1, 5
    };
    glPushMatrix();

    glTranslatef(x, y, z);

    glVertexPointer(3, GL_FLOAT, 0, coords);
    glColorPointer(3, GL_FLOAT, 0, colors);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glRotatef(rot++, 0, 1, 0);
    glRotatef(45, 1, 0, 0);
    /*glDrawArrays(GL_TRIANGLE_FAN, 0, 4);*/
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, elementArray);


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}


void DrawSkybox(void)
{
    float coords[] =
    {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    };
/*
    float colors[] =
    {
        0.5, 0.8f, 1,
        0, 0.8f, 1,
        0.5, 0.2f, 1,
        0, 0.2f, 1,
        0.5, 0.8f, 1,
        0, 0.8f, 1,
        0.5, 0.2f, 1,
        0, 0.2f, 1
    };
*/

    float colors[] =
    {
        1.0f, 0.7f, 0.7f,
        0.7f, 1.0f, 0.7f,
        0.7f, 0.7f, 1.0f,
        1.0f, 1.0f, 0.7f,
        1.0f, 0.7f, 0.7f,
        0.7f, 1.0f, 0.7f,
        0.7f, 0.7f, 1.0f,
        1.0f, 1.0f, 0.7f
    };

    int elementArray[] =
    {
        0, 1, 2, 3,
        1, 5, 6, 2,
        5, 4, 7, 6,
        4, 0, 3, 7,
        7, 3, 2, 6,
        4, 5, 1, 0
    };


    glPushMatrix();

    //glRotatef(20, 1, 0, 0);
    glRotatef(player.camRot, 0, 1, 0);
    //glScalef(1, 1, 1);


    glVertexPointer(3, GL_FLOAT, 0, coords);
    glColorPointer(3, GL_FLOAT, 0, colors);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    /*glDrawArrays(GL_TRIANGLE_FAN, 0, 4);*/
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, elementArray);


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}
