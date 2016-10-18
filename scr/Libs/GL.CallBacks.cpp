#include "GL.Callbacks.h"
#include "../Classes/nTMap.hpp"
#include "../Classes/Player.hpp"
#include "../Classes/Shader.hpp"

bool glCallbacksReleaseMouseOffSet=false;//evita de definir o mouse solto no meio ou final da funcao update e o programa nao fazer a leitura
bool glCallbacksReleaseMouseROffSet=false;//evita de definir o mouse solto no meio ou final da funcao update e o programa nao fazer a leitura
bool glCallbacksReleaseZOffSet=false;

void drawScene(void){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    GL::framesInGame++;
    if(GL::framesInGame>=100000000000)
            GL::framesInGame=0;
    if(Scenes::current==Scenes::game)
        Scenes::drawGame();
    else if(Scenes::current==Scenes::mapEdit)
        Scenes::drawMapEdit();
    else if(Scenes::current==Scenes::menu)
        Scenes::drawMenu();
    else if(Scenes::current==Scenes::credits)
        Scenes::drawCredits();
    else if(Scenes::current==Scenes::splash)
        Scenes::drawSplash();
    else if(Scenes::current==Scenes::options)
        Scenes::drawOptions();
    else if(Scenes::current==Scenes::preGame)
        Scenes::drawPreGame();
    else if(Scenes::current==Scenes::preCampaign)
        Scenes::drawPreCampaign();
    else if(Scenes::current==Scenes::preFreeMode)
        Scenes::drawPreFreeMode();
    else if(Scenes::current==Scenes::posGame)
        Scenes::drawPosGame();
    else if(Scenes::current==Scenes::posGameEnd)
        Scenes::drawEndGame();
    else if(Scenes::current==Scenes::posYouWin)
        Scenes::drawYouWin();
    glDisable(GL_DEPTH_TEST);
    glutSwapBuffers();
}

void reshape(int width, int height){
   glViewport (0, 0,width,height);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(65.0,width/height,1,GL::defaultSize.z);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   GL::currentSize.x=width;
   GL::currentSize.y=height;
}

int GLSpecReleaseKey=0;

void specialKeyboard(int key, int x, int y){
  switch(key)
  {
    case GLUT_KEY_UP:
        if(Scenes::current==Scenes::game){
            GL::cam.rotateZaxis(1*10);
        }
    break;

    case GLUT_KEY_DOWN:
        if(Scenes::current==Scenes::game){
            GL::cam.rotateZaxis(-1*10);
        }
        
    break;

    case GLUT_KEY_LEFT:
        if(GLSpecReleaseKey==-1)
            GLSpecReleaseKey=0;
        if(Scenes::current==Scenes::game){
            GL::cam.rotateYaxis(-1*10);
        }
    break;

    case GLUT_KEY_RIGHT:
        if(GLSpecReleaseKey==1)
            GLSpecReleaseKey=0;
        if(Scenes::current==Scenes::game){
            GL::cam.rotateYaxis(1*10);
        }
        
      break;
}
}
void keyboard(unsigned char key, int x, int y){
    switch(key){
        case '+':
            GL::cam.moveYaxis(1);
      break;
      
      case '-':
          GL::cam.moveYaxis(-1);
      break;
      case ' ':
          Player::jump();
      break;
    case 'W':
    case 'w':
        Player::moveFoward(1);
        break;
        
    case 'S':
    case 's':
       Player::moveFoward(-1);
    break;

        case 'A':
        case 'a':
            Player::moveSide(1);
      break;
      
      case 'D':
        case 'd':
            Player::moveSide(-1);
      break;
            
      case 'Z':
      case 'z':
        glCallbacksReleaseZOffSet=false;
      break;

      case 'X':
      case 'x':

      break;
      
      case 27:
        if(Scenes::current==Scenes::game) 
            GL::isPaused=!GL::isPaused;
      break;
      default:break;
    }
}

void specialKeyboardUp(int key,int x,int y){
  switch(key)
  {
    case GLUT_KEY_UP:
    
    break;

    case GLUT_KEY_DOWN:
    
    break;

    case GLUT_KEY_LEFT:
    
    break;

    case GLUT_KEY_RIGHT:
    
    break;

    default:
    break;
  }
}
void keyboardUp(unsigned char key,int x,int y){
    switch(key){
        case ' ':
        if(!GL::isPaused&&Scenes::current==Scenes::game){
            
        }
        break;
        case 'z':
        case 'Z':
            glCallbacksReleaseZOffSet=true;
        break;
            
        case '0':
              
        break;
        case '1':
              
        break;
        case '2':
              
        break;
        case '3':
              
        break;
        case '4':
              
        break;
        case '5':
              
        break;
        case '6':
              
        break;
        case '7':
              
        break;
        case '8':
              
        break;
        case '9':
              
        break;
        
        case 'O':
        case 'o':
            Player::dontGiveAShitForCollision=!Player::dontGiveAShitForCollision;
        break;
        
        case 'L':
        case 'l':
            Shader::isOn=!Shader::isOn;
        break;
        
        case 'C':
        case 'c':
            Player::firstPerson=!Player::firstPerson;
        break;
        
        case 'F':
        case 'f':
            if(Map::fillWalls&&!Map::useWallTexture)
                Map::useWallTexture=true;
            else if(!Map::fillWalls&&Map::useWallTexture){
                Map::fillWalls=true;
                Map::useWallTexture=false;
            }else Map::fillWalls=false;
            
        break;
        
        break;
        
        case 13://enter
            
        break;
        
        case 8://backSpace
            
        break;
    }
}

void update(int n){
    if(GLSpecReleaseKey!=0){
        
    }
            
    GL::mousePos.x=GL::rawMousePos.x*(float)GL::defaultSize.x/(float)GL::currentSize.x+Scenes::camera.x.movedCam;
    GL::mousePos.y=GL::rawMousePos.y*(float)GL::defaultSize.y/(float)GL::currentSize.y+Scenes::camera.y.movedCam;
    if(glCallbacksReleaseZOffSet){
        glCallbacksReleaseZOffSet=false;
        if(!GL::isPaused){

        }
    }
    if(glCallbacksReleaseMouseOffSet){
        GL::leftMouseReleased=false;
        glCallbacksReleaseMouseOffSet=false;
    }
    if(GL::leftMouseReleased)glCallbacksReleaseMouseOffSet=true;
    if(glCallbacksReleaseMouseROffSet){
        GL::rightMouseReleased=false;
        glCallbacksReleaseMouseROffSet=false;
    }
    if(GL::rightMouseReleased)glCallbacksReleaseMouseROffSet=true;
    glutPostRedisplay();
    glutTimerFunc(GL::getMs(), update, 0);
}
void mousePress(int button,int state,int x,int y){
    if(button==GLUT_LEFT_BUTTON){
        if(state==GLUT_DOWN){
            GL::leftMouseClicked=true;
            GL::leftMouseReleased=false;
          }
      if(state==GLUT_UP){
            GL::leftMouseClicked=false;
            GL::leftMouseReleased=true;
        }
    }
    if(button==GLUT_RIGHT_BUTTON){
        if(state==GLUT_DOWN){
            GL::rightMouseClicked=true;
            GL::rightMouseReleased=false;
          }
      if(state==GLUT_UP){
            GL::rightMouseClicked=false;
            GL::rightMouseReleased=true;
        }
    }
    if(button==GLUT_MIDDLE_BUTTON){
        if(state==GLUT_DOWN){
            
          }
        if(state==GLUT_UP){   

        }
    }
}
void mousePassiveMotion(int x,int y){
    nTPoint offSet;
    offSet.setPoint((GL::rawMousePos.x-x)/-GL::currentSize.x,(GL::rawMousePos.y-y)/GL::currentSize.y,0);
    GL::cam.rotateYaxis(offSet.x*nTCamera::mouseX);
    GL::cam.rotateZaxis(offSet.y*nTCamera::mouseY);
    GL::rawMousePos.setPoint(x,y,0);
    if(nTCamera::putMouseOnMiddle){
        glutWarpPointer(GL::currentSize.x/2,GL::currentSize.y/2);
        GL::rawMousePos.setPoint(GL::currentSize.x/2,GL::currentSize.y/2,0);
    }
}
void mouseActiveMotion(int x,int y){
    nTPoint offSet;
    offSet.setPoint((GL::rawMousePos.x-x)/-GL::currentSize.x,(GL::rawMousePos.y-y)/GL::currentSize.y,0);
    GL::cam.rotateYaxis(offSet.x*nTCamera::mouseX);
    GL::cam.rotateZaxis(offSet.y*nTCamera::mouseY);
    GL::rawMousePos.setPoint(x,y,0);
    if(nTCamera::putMouseOnMiddle){
        glutWarpPointer(GL::currentSize.x/2,GL::currentSize.y/2);
        GL::rawMousePos.setPoint(GL::currentSize.x/2,GL::currentSize.y/2,0);
    }
}
