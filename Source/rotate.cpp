#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <math.h>
#include <cstdlib>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using std::max;


#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
struct Intersection{
  vec4 position;
  float distance;
  int triangleIndex;
};


bool Update(vec4& cameraPos,vec4& lightPos);
void updateR();
float deg2rad(float a);
float dx();
float dz();
float distance(vec4 camera);
void Draw(screen* screen, vec4 cameraPos, vec4 lightPos);
bool ClosestIntersection( vec4 start,
                          vec4 dir,
                          const vector<Triangle>& triangles,
                          Intersection& closestIntersection );
vec3 DirectLight(Intersection interS, vec4 lightPos);


const float focalLength = 226.274;
mat3 R;
float theta, theta1, theta2;
const float angle = 30;
vec4 initcamera(0, 0, -4, 1);

int main( int argc, char* argv[] ){

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  //initializing variables that will be updated/used throughout
  vec4 cameraPos = initcamera;
  vec4 lightPos( 0, -0.5, -0.7, 1.0 );

  theta,theta1,theta2 = 0;

  updateR();

  while( Update(cameraPos, lightPos) ) //could also use: NoQuitMessageSDL() and call Update() in the loop
    {
      Draw(screen, cameraPos,lightPos);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

void updateR(){
  R[0][0] = cos(deg2rad(theta));
  R[0][1] = 0;
  R[0][2] = sin(deg2rad(theta));
  R[1][0] = 0;
  R[1][1] = 1;
  R[1][2] = 0;
  R[2][0] = -sin(deg2rad(theta));
  R[2][1] = 0;
  R[2][2] = cos(deg2rad(theta));
}

float deg2rad(float a){
  return a*(M_PI/180);
}

float rad2deg(float a){
  return a*(180/M_PI);
}

float dx(){
  return ( 2*abs(initcamera.z)*(sin(deg2rad(theta)/2))*(cos(deg2rad(theta)/2)) );
}

float dz(){
  return ( 2*abs(initcamera.z)*(sin(deg2rad(theta)/2))*(sin(deg2rad(theta)/2)) );
}

float distance(vec4 camera){
  float d = sqrt((camera.x*camera.x)+(camera.z*camera.z));
  return d;
}

bool ClosestIntersection( vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection ){

  bool toReturn = false;
  float closestSoFar = std::numeric_limits<float>::max();

  for(int i = 0; i < triangles.size(); ++i){
    using glm::vec4;
    using glm::mat4;
    using glm::vec3;
    using glm::mat3;

    vec4 v0 = triangles[i].v0;
    vec4 v1 = triangles[i].v1;
    vec4 v2 = triangles[i].v2;

    vec3 e1 = vec3(v1.x-v0.x, v1.y-v0.y, v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x, v2.y-v0.y, v2.z-v0.z);
    vec3 b = vec3(start.x-v0.x, start.y-v0.y, start.z-v0.z);

    vec3 threeDir(dir.x, dir.y, dir.z);
    mat3 A(-threeDir, e1, e2);
    vec3 x = glm::inverse(A)*b;

    vec4 pos( (v0.x + (x.y*e1.x) + (x.z*e2.x)),
              (v0.y + (x.y*e1.y) + (x.z*e2.y)),
              (v0.z + (x.y*e1.z) + (x.z*e2.z)),
              1);

    if( (x.x >= 0) && (x.y > 0) && (x.z > 0) && ((x.y + x.z)<1) ){
        toReturn = true;
        float tempDist = x.x;

        if(tempDist < closestSoFar){
          closestIntersection.position = pos;
          closestIntersection.distance = tempDist;
          closestIntersection.triangleIndex = i;
          closestSoFar = tempDist;
        }

    }//end of if statement checking u, v, t

  }//end of loop for all triangles

  return toReturn;
}

vec3 DirectLight(Intersection interS, vec4 lightPos){
  vec3 toReturn(0, 0, 0);
  vec3 lightColor = 14.f * vec3( 1, 1, 1 );
  vector<Triangle> triangles;
  LoadTestModel( triangles );

  vec4 rCarrot(lightPos.x - interS.position.x,
         lightPos.y - interS.position.y,
         lightPos.z - interS.position.z,
         // lightPos.w - interS.position.w); // this is what I was doing when it didn't really work...maybe this should always just be 1 still?
         1);
  vec4 nCarrot = triangles[interS.triangleIndex].normal;
  float radius = sqrt( pow(lightPos.x - interS.position.x ,2) + pow(lightPos.y - interS.position.y, 2) + pow(lightPos.z - interS.position.z, 2) + pow(lightPos.w - interS.position.w, 2) ); //diong it with 1 for w now instead of subtracting them
  // float radius = sqrt( pow(lightPos.x - interS.position.x ,2) + pow(lightPos.y - interS.position.y, 2) + pow(lightPos.z - interS.position.z, 2) + pow(lightPos.w - interS.position.w, 2) );
  float dotProduct = (rCarrot.x*nCarrot.x + rCarrot.y*nCarrot.y + rCarrot.z*nCarrot.z + rCarrot.w*nCarrot.w );

  if( dotProduct > 0){
    float multiplyBy = ( dotProduct/ (4*pow(radius, 2)*M_PI) );
    toReturn.x = lightColor.x*multiplyBy;
    toReturn.y = lightColor.y*multiplyBy;
    toReturn.z = lightColor.z*multiplyBy;
  }

  //adding shadows here:
  //I check if the distance between interS and lightPos is >, <
  // the distance between lightPos and closestIntersection

  Intersection closestToLight;
  vec4 shadowDirection(rCarrot.x*-1, rCarrot.y*-1, rCarrot.z*-1, 1);
  if( ClosestIntersection(lightPos, shadowDirection, triangles, closestToLight) ){
    float tempDistance = sqrt( pow(lightPos.x - closestToLight.position.x, 2) + pow(lightPos.y - closestToLight.position.y, 2) + pow(lightPos.z - closestToLight.position.z, 2) + 1  ); //this is distance from light to closest intersection
    if(tempDistance < radius){
      return vec3(0, 0, 0);
    }
  }

  return toReturn;
}

/*Place your drawing here*/
void Draw(screen* screen, vec4 cameraPos, vec4 lightPos){
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  vector<Triangle> triangles;
  LoadTestModel( triangles );
  Intersection closest;

  //start of getting helper variables for rotated riection 18 Feb
  // float h = ( sqrt( pow(cameraPos.x, 2) + pow(cameraPos.z, 2) )*cos(theta) );
  // float h = focalLength;
  // float dirX = ( h*cos(M_PI/2-theta) );
  // float dirZ = ( h*sin(M_PI/2-theta) );

  // cout << "In draw, (dirX: "
  //      << dirX << ","
  //      << "dirZ: " << dirZ << ", "
  //      << "theta: " << theta << ", "
  //      << ")" << endl;
  //end of helpers from 18 Feb rotated dir

  vec4 right( R[0][0], R[0][1], R[0][2], 1 );
  vec4 down( R[1][0], R[1][1], R[1][2], 1 );
  vec4 forward( R[2][0], R[2][1], R[2][2], 1 );

  // cout << "(x= " << cameraPos.x
  //      << ", y= " << cameraPos.y
  //      << ", z= " << cameraPos.z
  //      << ") // angle = " << rad2deg(theta)
  //      << " // distance = " << distance(cameraPos)
  //      << endl;

  for(int i = 0; i < SCREEN_WIDTH; i ++){
    for(int j = 0; j < SCREEN_HEIGHT; j++){

      //for the regular working picture
      vec4 dir(i - (SCREEN_WIDTH/2), j - (SCREEN_HEIGHT/2), focalLength, 1); //this was how we did dir before starting the camera rotation on 18 Feb
      vec4 dir2(right.x*dir.x + right.y*dir.y + right.z*dir.z,
        down.x*dir.x + down.y*dir.y + down.z*dir.z,
        forward.x*dir.x + forward.y*dir.y + forward.z*dir.z,
        1);
      //for trying to do rotation
      // vec4 dir( dirX + i - (SCREEN_WIDTH/2), j - (SCREEN_HEIGHT/2), dirZ, 1);

      if( ClosestIntersection(cameraPos, dir2, triangles, closest) ){

        // //for regular colored no lighitng
        // PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color ); //this is how we did it pre-illumination
        // //for fancy black and white shading
        // PutPixelSDL(screen, i, j, DirectLight(closest, lightPos) ); // this is how we're diong it with B&W illumination
        // //for fancy in color shading (now with shadows too!)
        // PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color * DirectLight(closest, lightPos) ); // this is how we're diong it with B&W illumination

        //final illumination: direct light, shadows, and indirectLight approxomation
        vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );
        PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color * (DirectLight(closest, lightPos) + indirectLight) ); // this is how we're diong it with B&W illumination

      }
      else{
        // // to prove that draw works, draw with these two lines
        vec3 tempColor(0.25, 0.25, 0.25);
        PutPixelSDL(screen, i, j, tempColor );
      }

    }
  } //end of nested for loops where we draw each pixel

}

/*Place updates of parameters here*/
bool Update(vec4& cameraPos, vec4& lightPos){
  static int t = SDL_GetTicks();
  /* Compute frame time */
  // int t2 = SDL_GetTicks();
  // float dt = float(t2-t);
  // t = t2;
  //std::cout << "Render time: " << dt << " ms." << std::endl;

  float diff = abs(abs(theta)-deg2rad(360));
  if(diff < 0.01){
    theta = 0;
  }
  /* Update variables*/
  SDL_Event e;
  // bool lastWasRight = true;

  while(SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
        {
          return false;
        }
      else
        if (e.type == SDL_KEYDOWN)
          {
            int key_code = e.key.keysym.sym;
            switch(key_code)
            {
              case SDLK_UP:
                /* Move camera forward */
                cameraPos.z += 0.1;
                break;
              case SDLK_DOWN:
              /* Move camera backwards */
                cameraPos.z -= 0.1;
                break;
              case SDLK_LEFT:
              /* Move camera left */
                theta += angle;
                cameraPos.x = initcamera.x - dx();
                cameraPos.z = initcamera.z + dz();

                updateR();
                break;
              case SDLK_RIGHT:
              /* Move camera right */
                // if (theta == 0){
                //   theta = deg2rad(360);
                // }
                theta -= angle;
                cameraPos.x = initcamera.x - dx();
                cameraPos.z = initcamera.z + dz();

                updateR();
                break;

              case SDLK_i:
                /* Rotate camera down (x axis) */
                theta1 += angle;
                R[0][0] = 1;
                R[0][1] = 0;
                R[0][2] = 0;
                R[1][0] = 0;
                R[1][1] = cos(deg2rad(theta1));
                R[1][2] = -sin(deg2rad(theta1));
                R[2][0] = 0;
                R[2][1] = sin(deg2rad(theta1));
                R[2][2] = cos(deg2rad(theta1));
                break;
              case SDLK_k:
                /* Rotate camera up (y axis) */
                theta1 -= angle;
                R[0][0] = 1;
                R[0][1] = 0;
                R[0][2] = 0;
                R[1][0] = 0;
                R[1][1] = cos(deg2rad(theta1));
                R[1][2] = -sin(deg2rad(theta1));
                R[2][0] = 0;
                R[2][1] = sin(deg2rad(theta1));
                R[2][2] = cos(deg2rad(theta1));
                break;
              case SDLK_j:
                /* Rotate camera left (z axis) */
                theta2 += angle;
                R[0][0] = cos(deg2rad(theta2));
                R[0][1] = -sin(deg2rad(theta2));
                R[0][2] = 0;
                R[1][0] = sin(deg2rad(theta2));
                R[1][1] = cos(deg2rad(theta2));
                R[1][2] = 0;
                R[2][0] = 0;
                R[2][1] = 0;
                R[2][2] = 1;
                break;
              case SDLK_l:
                /* Rotate camera up */
                theta2 -= angle;
                R[0][0] = cos(deg2rad(theta2));
                R[0][1] = -sin(deg2rad(theta2));
                R[0][2] = 0;
                R[1][0] = sin(deg2rad(theta2));
                R[1][1] = cos(deg2rad(theta2));
                R[1][2] = 0;
                R[2][0] = 0;
                R[2][1] = 0;
                R[2][2] = 1;
                break;
              //for moving the light
              case SDLK_a:
                /* Move light left */
                lightPos.x -= .25;
                break;
              case SDLK_d:
                /* Move light right */
                lightPos.x += .25;
                break;
              case SDLK_w:
                /* Move light forward */
                lightPos.y -= .25;
                break;
              case SDLK_s:
                /* Move light back */
                lightPos.y += .25;
                break;
              case SDLK_q:
                /* Move light up */
                lightPos.z += .25;
                break;
              case SDLK_e:
                /* Move light down */
                lightPos.z -= .25;
                break;
              case SDLK_ESCAPE:
              /* Move camera quit */
                return false;
              }
          }
    }

    return true;
  }
