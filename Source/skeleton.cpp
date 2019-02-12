#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <math.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define FULLSCREEN_MODE true

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
struct Intersection{
  vec4 position;
  float distance;
  int triangleIndex;
};


bool Update(vec4& cameraPos, float& yaw);
void Draw(screen* screen, vec4 cameraPos, float yaw);
bool ClosestIntersection( vec4 start,
                          vec4 dir,
                          const vector<Triangle>& triangles,
                          Intersection& closestIntersection );
float getFocalLength(vec4 camera);




int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vec4 cameraPos(0, 0, -1110/SCREEN_WIDTH, 1);

  float yaw = 0;

  while( Update(cameraPos, yaw) ) //NoQuitMessageSDL() )
    {
      // Update();
      Draw(screen, cameraPos, yaw);
      SDL_Renderframe(screen);
    }

  //   // Start of where I was testing ClosestIntersection
  //
  // vector<Triangle> triangles;
  // LoadTestModel( triangles );
  // vec4 start(278/555, 278/555, 1, 1);
  // vec4 dir(0, 1, 1, 1);
  //
  // // vec4 interPos(0, 0, 0, 0);
  // // float interDist = 0;
  // // int interTIndex = 0;
  // // closest.position = interPos;
  // // closest.distance = interDist;
  // // closest.triangleIndex = interTIndex;
  //
  // Intersection closest;
  //
  // bool result = ClosestIntersection(start, dir, triangles, closest);
  // if(result){
  //   cout << "we got one!" << ", "
  //        << closest.position.x << ", "
  //        << closest.position.y << ", "
  //        << closest.position.z << ", "
  //        << closest.position.w << ", "
  //        << closest.distance << ", "
  //        << closest.triangleIndex;
  // }
  // else{
  //   cout << "no closest found :(" << endl;
  // }
  //
  //   // End of where I was testing ClosestIntersection


  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

float getFocalLength(vec4 camera){
  // float toReturn = camera.z;
  // toReturn = toReturn * (-SCREEN_WIDTH/4) ;
  return 226.274;
}



bool ClosestIntersection( vec4 start,
                          vec4 dir,
                          const vector<Triangle>& triangles,
                          Intersection& closestIntersection ){

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

    // for debugging
    // cout << "("
    //      << i << ", "
    //      << v0.x << ", "
    //      << v0.y << ", "
    //      << v0.z << ", "
    //      << v0.w << ") " << endl;


    vec3 e1 = vec3(v1.x-v0.x, v1.y-v0.y, v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x, v2.y-v0.y, v2.z-v0.z);
    vec3 b = vec3(start.x-v0.x, start.y-v0.y, start.z-v0.z);

    vec3 threeDir(dir.x, dir.y, dir.z);
    mat3 A(-threeDir, e1, e2); //how do we get d?...its just d
    vec3 x = glm::inverse(A)*b;

    vec4 pos( (v0.x + (x.y*e1.x) + (x.z*e2.x)), (v0.y + (x.y*e1.y) + (x.z*e2.y)), (v0.z + (x.y*e1.z) + (x.z*e2.z)), 1);

    // cout << "(I: "
    //      << i << ","
    //      << pos.x << ", "
    //      << pos.y << ", "
    //      << pos.z << ", "
    //      << pos.w << ")" << endl;



    if(x.x >= 0){
      // cout << "(I: "
      //      << i << ","
      //      << x.x << ", "
      //      << x.y << ", "
      //      << x.z << ")" << endl;
      if((x.y > 0) && (x.z > 0) && ((x.y + x.z)<1) ){
        // cout << "Intersection!";
        toReturn = true;
        float tempDist = x.x; //compute this

        if(tempDist < closestSoFar){
          // vec4 pos( (v0.x + (x.x*e1.x) + (x.y*e2.x)), (v0.y + (x.x*e1.y) + (x.y*e2.y)), (v0.z + (x.x*e1.z) + (x.y*e2.z)), 1);
          closestIntersection.position = pos; //is this related to x?
          closestIntersection.distance = tempDist;
          closestIntersection.triangleIndex = i;
          closestSoFar = tempDist;
        }
      } //end of if checking u and v

    }//end of if statement checking if t > 0

  }//end of loop for all triangles



  return toReturn;

}

/*Place your drawing here*/
void Draw(screen* screen, vec4 cameraPos, float yaw)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  // // this is the stock code to draw stuff
  // vec3 colour(1.0,0.0,0.0);
  // for(int i=0; i<1000; i++)
  //   {
  //     uint32_t x = rand() % screen->width;
  //     uint32_t y = rand() % screen->height;
  //     PutPixelSDL(screen, x, y, colour);
  //   }

  // this is my implemented draw FUNCTION
  vector<Triangle> triangles;
  LoadTestModel( triangles );
  Intersection closest;
  float focalLength = getFocalLength(cameraPos); // NB: measured in units of pixels
  // vec4 cameraPos(0, 0, -277.5/555, 1); // is this in pixel units??

  //printing to see if the camera moving with keys works...
  // cout << "In draw, (x: " << cameraPos.x << ", y: " << cameraPos.y << ", z: " << cameraPos.z << ")" << endl;
  //end of moving camera testing



  // v/f = y/z, where f = focal length in pixels, z = distance for cameraPos, v = half of SCREEN_WIDTH, and y = half of model height
  //this is how we get focal length and z values for cameraPos
  for(int i = 0; i < SCREEN_WIDTH; i ++){ // usually SCREEN_WIDTH
    for(int j = 0; j < SCREEN_HEIGHT; j++){ // usually SCREEN_HEIGHT

      // cout << "In draw, (I: "
      //      << i << ","
      //      << "J: " << j << ", "
      //      << ")" << endl;

      vec4 dir(i - (SCREEN_WIDTH/2), j - (SCREEN_HEIGHT/2), focalLength, 1);
      if( ClosestIntersection(cameraPos, dir, triangles, closest) ){
        PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color );
        // cout << "We painted a pixel?" << endl;
      }
      else{
        // // to prove that draw works, draw with these two lines
        // vec3 tempColor(.02*i, .03*j, i+j);
        PutPixelSDL(screen, i, j, triangles[3].color );
      }


    }
  }
}

/*Place updates of parameters here*/
bool Update(vec4& cameraPos, float& yaw)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
  SDL_Event e;
  bool lastWasRight = true;

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
                cameraPos.z += .1;
                break;
              case SDLK_DOWN:
              /* Move camera backwards */
                cameraPos.z -= .1;
                break;
              case SDLK_LEFT:
              /* Move camera left */
                // cameraPos.x -= .1; //first task
                if(lastWasRight){
                  yaw = yaw*-1;
                  lastWasRight = false;
                }
                yaw += .01;
                cameraPos.x = ( cameraPos.x*cos(yaw) - cameraPos.z*sin(yaw) );
                cameraPos.z = ( cameraPos.z*cos(yaw) + cameraPos.z*sin(yaw) );
                break;
              case SDLK_RIGHT:
              /* Move camera right */
                // cameraPos.x = cameraPos.x + .1; // first task
                yaw -= .01;
                cameraPos.x = ( cameraPos.x*cos(yaw) - cameraPos.z*sin(yaw) );
                cameraPos.z = ( cameraPos.z*cos(yaw) + cameraPos.z*sin(yaw) );
                lastWasRight = true;
                break;
              case SDLK_ESCAPE:
              /* Move camera quit */
                return false;
              }
          }
    }
    //printing to see if the camera moving with keys works...
    cout << "In update, (x: " << cameraPos.x << ", y: " << cameraPos.y << ", z: " << cameraPos.z << ")" << endl;
    //end of moving camera testing

    return true;
  }
