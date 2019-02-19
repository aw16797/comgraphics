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
using std::max;


#define SCREEN_WIDTH 555
#define SCREEN_HEIGHT 555
#define FULLSCREEN_MODE true

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
struct Intersection{
  vec4 position;
  float distance;
  int triangleIndex;
};


bool Update(vec4& cameraPos, float& yaw, vec4& lightPos);
void Draw(screen* screen, vec4 cameraPos, float yaw, vec4 lightPos);
bool ClosestIntersection( vec4 start,
                          vec4 dir,
                          const vector<Triangle>& triangles,
                          Intersection& closestIntersection );
vec3 DirectLight(Intersection interS, vec4 lightPos);
float getFocalLength(vec4 camera);




int main( int argc, char* argv[] ){

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vec4 cameraPos(0, 0, -2.75, 1);

  float yaw = 0;

  //this will be passe dinto update and then into DirectLight
  vec4 lightPos( 0, -0.5, -0.7, 1.0 );

  while( Update(cameraPos, yaw, lightPos) ) //NoQuitMessageSDL() )
    {
      // Update();
      Draw(screen, cameraPos, yaw, lightPos);
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

vec3 DirectLight(Intersection interS, vec4 lightPos){
  vec3 toReturn(0, 0, 0);

  vec3 lightColor = 14.f * vec3( 1, 1, 1 ); //original from directions
  // vec3 lightColor = 14.f * vec3( .01, .01, .01); //looks better with this, but idk why (there's too much power (pixels are getting too much light) so all the pixels are white)

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
void Draw(screen* screen, vec4 cameraPos, float yaw, vec4 lightPos){
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

  //start of getting helper variables for rotated riection 18 Feb
  // float h = ( sqrt( pow(cameraPos.x, 2) + pow(cameraPos.z, 2) )*cos(yaw) );
  float h = focalLength;
  float dirX = ( h*cos(M_PI/2-yaw) );
  float dirZ = ( h*sin(M_PI/2-yaw) );

  // cout << "In draw, (dirX: "
  //      << dirX << ","
  //      << "dirZ: " << dirZ << ", "
  //      << "yaw: " << yaw << ", "
  //      << ")" << endl;
  //end of helpers from 18 Feb rotated dir

  for(int i = 0; i < SCREEN_WIDTH; i ++){ // usually SCREEN_WIDTH
    for(int j = 0; j < SCREEN_HEIGHT; j++){ // usually SCREEN_HEIGHT

      // cout << "In draw, (dirX: "
      //      << dirX << ","
      //      << "dirZ: " << dirZ << ", "
      //      << ")" << endl;

      //for the regular working picture
      vec4 dir(i - (SCREEN_WIDTH/2), j - (SCREEN_HEIGHT/2), focalLength, 1); //this was how we did dir before starting the camera rotation on 18 Feb

      //for trying to do rotation
      // vec4 dir( dirX + i - (SCREEN_WIDTH/2), j - (SCREEN_HEIGHT/2), dirZ, 1);

      if( ClosestIntersection(cameraPos, dir, triangles, closest) ){
        //for regular colored no lighitng
        // PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color ); //this is how we did it pre-illumination

        //for fancy black and white shading
        // PutPixelSDL(screen, i, j, DirectLight(closest, lightPos) ); // this is how we're diong it with B&W illumination

        //for fancy in color shading (now with shadows too!)
        // PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color * DirectLight(closest, lightPos) ); // this is how we're diong it with B&W illumination

        //final illumination: direct light, shadows, and indirectLight approxomation
        vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );
        PutPixelSDL(screen, i, j, triangles[closest.triangleIndex].color * (DirectLight(closest, lightPos) + indirectLight) ); // this is how we're diong it with B&W illumination



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
bool Update(vec4& cameraPos, float& yaw, vec4& lightPos){
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
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
                cameraPos.z += .1;
                break;
              case SDLK_DOWN:
              /* Move camera backwards */
                cameraPos.z -= .1;
                break;
              case SDLK_LEFT:
              /* Move camera left */
                // cameraPos.x -= .1; //first task
                // if(lastWasRight){  //probably doens't work 18 Feb
                //   yaw = yaw*-1;
                //   lastWasRight = false;
                // }
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
                // lastWasRight = true; //probably doens't work 18 Feb
                break;
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
    //printing to see if the camera moving with keys works...
    // cout << "In update, (x: " << cameraPos.x << ", y: " << cameraPos.y << ", z: " << cameraPos.z << ")" << endl;
    //end of moving camera testing

    return true;
  }
