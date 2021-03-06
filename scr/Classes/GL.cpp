#include "GL.hpp"
#include "../Libs/GL.Callbacks.h"
#include "nTCamera.hpp"
#include "nTMap.hpp"
#include "Scenes.hpp"
#include "Light.hpp"

GL::GL(string name,float FPS,GLbitfield displayMode,nTPoint defaultsize,bool blend,vector<GLenum>&enables,int argc, char** argv) {
    this->FPS=FPS;
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(displayMode);
    defaultSize.setPoint(defaultsize.x,defaultsize.y,defaultsize.z);
    glutInitWindowSize(defaultsize.x, defaultsize.y);
    currentSize=defaultsize;
    glutInitWindowPosition(0, 0);
    glutCreateWindow(name.c_str());
    glewInit();
    for(GLenum en:enables)
        glEnable(en);
    if(blend)
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutSpecialUpFunc(specialKeyboardUp);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyboard);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, -1);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutMotionFunc(mouseActiveMotion);
    glutMouseFunc(mousePress);
    glClearColor(clearcolor.R, clearcolor.G, clearcolor.B, clearcolor.A);
    nTCamera camera;
    cam=camera;
    nTMap::scale.setPoint(1,3,1);
};

GL::GL(const GL& orig) {
}

GL::~GL() {
}

float GL::FPS;
nTColor GL::currentColor=Util::nTColorSet(1,1,1,1);
void* GL::currentFont=GLUT_BITMAP_TIMES_ROMAN_24;
vector<GLuint> GL::textures;
vector<string> GL::textureNames;
nTPoint GL::defaultSize;
nTPoint GL::currentSize;
bool GL::isPaused=false;
bool GL::leftMouseClicked=false;
bool GL::leftMouseReleased=false;
bool GL::rightMouseClicked=false;
bool GL::rightMouseReleased=false;
nTColor GL::clearcolor;
unsigned long int GL::framesInGame=0;
nTPoint GL::mousePos;
nTPoint GL::rawMousePos;
GLuint GL::GLlist=glGenLists(1);
nTCamera GL::cam;
nTColor GL::bindColor;
bool GL::is3D=true;
bool GL::fullScreen=false;


void GL::setFPS(float FPS){
    GL::FPS=FPS;
}
float GL::getFPS(){
    return FPS;
}
float GL::getMs(){
    return 1000/FPS;
}
void GL::start(){
    glutMainLoop();
}
void GL::setND(int width, int height){
   GL::currentSize.x=width;
   GL::currentSize.y=height;
   glViewport (0, 0,width,height);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   Scenes::putCameraOnOrigin();
}

void GL::set3D(int width, int height){
  is3D=true;
  setND(width,height);
  gluPerspective(65.0,width/height,1,GL::defaultSize.z);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GL::set2D(int width, int height){
  is3D=false;
  setND(width,height);
  glOrtho(Scenes::camera.x.movedCam, GL::defaultSize.x+Scenes::camera.x.movedCam,GL::defaultSize.y+Scenes::camera.y.movedCam, Scenes::camera.y.movedCam, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GL::setFullScreen(bool fs){
    GL::fullScreen=fs;
    if(fs){
        glutFullScreen();
        GL::currentSize.setPoint(glutGet(GLUT_SCREEN_WIDTH),glutGet(GLUT_SCREEN_HEIGHT),GL::currentSize.z);
    }else{
        glutReshapeWindow(GL::defaultSize.x, GL::defaultSize.y);
        glutPositionWindow(20,20);
        glutPostRedisplay();
    }
}
bool GL::getFullScreen(){
    return fullScreen;
}

nTMesh GL::loadObjCplx(string path){
  nTMesh mesh;
  string err;
  std::ifstream ifs(Util::newPath((char*)path.c_str()));
  if(ifs.fail()){
    cout << "file not found "<<path<<endl;
    return mesh;
  }
  
  tinyobj::callback_t cb;
  cb.vertex_cb = vertex_cb;
  cb.normal_cb = normal_cb;
  cb.texcoord_cb = texcoord_cb;
  cb.index_cb = index_cb;
  cb.usemtl_cb = usemtl_cb;
  cb.mtllib_cb = mtllib_cb;
  cb.group_cb = group_cb;
  cb.object_cb = object_cb;
  string folder="";
  bool condition=false;
  for(int i=path.size()-1;i>=0;i--){
      if(path[i]=='/'&&!condition){
          condition=true;
      }
      if(condition){
          for(int j=0;j<i;j++)
              folder+=path[j];
          i=-1;
          break;
      }
  }
  tinyobj::MaterialFileReader mtlReader(folder);
  if(!err.empty()){
    cout<<err<<endl;
  }  
  if(!tinyobj::LoadObjWithCallback(ifs, cb, &mesh, &mtlReader, &err)){
    cout<< "Failed to parse"<<path<<endl;
    return mesh;
  }  
  return mesh;
}

nTShape GL::loadObj(string path){
    nTShape out;
  tinyobj::attrib_t attrib;
  vector<tinyobj::shape_t> shapes;
  vector<tinyobj::material_t> materials;
  string err;
  if (tinyobj::LoadObj(&attrib, &shapes, &materials, &err, Util::newPath((char*)path.c_str()), NULL)) {
    cout<< "Failed to load " << path << endl;
  }
  if (!err.empty()) {
    cout << err << endl;
  }
  out.attrib=attrib;
  out.materials=materials;
  out.shapes=shapes;
  
  return out;
}

void GL::drawShapeObj(nTShape obj,nTColor color){
    nTColor tmp=getColor();
    setColor(color);
     
     for(int i=0;i<obj.shapes.size();i++){
         glBegin(GL_LINE_LOOP);
         for(int j=0;j<obj.shapes[i].mesh.indices.size();j++){
             glVertex3f(obj.attrib.vertices[obj.shapes[i].mesh.indices[j].vertex_index],
                     obj.attrib.vertices[obj.shapes[i].mesh.indices[j].vertex_index+1],
                     obj.attrib.vertices[obj.shapes[i].mesh.indices[j].vertex_index+2]);
         }
            
         glEnd(); 
     }
     
     
    setColor(tmp);
}

void GL::drawObj(nTMesh obj,nTColor color, string name, nTPoint pos, GLint way, int ang, nTPoint pointRotation, nTPoint coord){
    nTColor tmp=getColor();
    setColor(color);
//    glPolygonMode(GL_FRONT,GL_FILL);
    if(name=="player"){
      glPushMatrix();

        glTranslatef(Player::pos.x, Player::pos.y, Player::pos.z);
        glRotatef(Player::life,0,1,0);
        glTranslatef(-Player::pos.x, -Player::pos.y, -Player::pos.z);
  
    for(int f=0;f<obj.vertices.size();f+=3) {
      glBegin(way);
      glPointSize(GL_POINT_SIZE_MAX);
        //glVertex3f(obj.vertices[obj.v_indices[f]],obj.vertices[obj.v_indices[f]+1],obj.vertices[obj.v_indices[f]+2]);
        if(Player::pos.y<=0){
          glTranslatef(-pointRotation.x, -pointRotation.y, -pointRotation.z);
          glRotatef(ang,coord.x,coord.y,coord.z);
          glTranslatef(pointRotation.x, pointRotation.y, pointRotation.z);
        glVertex3f(obj.vertices[f]+Player::pos.x,obj.vertices[f+1]-5+pos.y,obj.vertices[f+2]+Player::pos.z);
        if(f%9==0)glVertex3f(obj.vertices[f]+Player::pos.x,obj.vertices[f+1]-5+pos.y,obj.vertices[f+2]+Player::pos.z);
      }else{
        glTranslatef(-pointRotation.x, -pointRotation.y, -pointRotation.z);
        glRotatef(ang,coord.x,coord.y,coord.z);
        glTranslatef(pointRotation.x, pointRotation.y, pointRotation.z);
        glVertex3f(obj.vertices[f]+Player::pos.x,obj.vertices[f+1]+cam.pos.y-2+pos.y,obj.vertices[f+2]+Player::pos.z);
        if(f%9==0)glVertex3f(obj.vertices[f]+Player::pos.x,obj.vertices[f+1]+cam.pos.y-2+pos.y,obj.vertices[f+2]+Player::pos.z);
      }
      glPopMatrix();
    }
  }else{
    glPushMatrix();
    for(int f=0;f<obj.vertices.size();f+=3) {
      glBegin(way);
      glPointSize(GL_POINT_SIZE_MAX);
      glTranslatef((pointRotation.x), (pointRotation.y), (pointRotation.z));
      glRotatef(ang,coord.x,coord.y,coord.z);
      glTranslatef(-(pointRotation.x), -(pointRotation.y), -(pointRotation.z));
      glutSolidCube(1);
        //glVertex3f(obj.vertices[obj.v_indices[f]],obj.vertices[obj.v_indices[f]+1],obj.vertices[obj.v_indices[f]+2]);
        glVertex3f(obj.vertices[f]+pos.x,obj.vertices[f+1]+pos.y,obj.vertices[f+2]+pos.z);
        glVertex3f(obj.vertices[f]+pos.x,obj.vertices[f+1]+pos.y,obj.vertices[f+2]+pos.z);
        if(f%9==0)glVertex3f(obj.vertices[f]+pos.x,obj.vertices[f+1]+pos.y,obj.vertices[f+2]+pos.z);
    }
    glPopMatrix();
  }
glEnd();
    setColor(tmp);
}


GLuint GL::loadTexture(string name,char* path){
    GLuint temp=SOIL_load_OGL_texture(Util::newPath(path),SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
    if(temp==0){
        if(Util::DEBUG) cout<<"falha ao carregar textura "<<Util::newPath(path)<<endl;
    }else{
        int i=0;
        for(string n:textureNames)
            if(n==name){
                textures[i]=temp;
                return temp;
            }else
                i++;
        textures.push_back(temp);
        textureNames.push_back(name);
    }
    return temp;
}
vector<GLuint> GL::loadTextures(string name,int nOfTex,char* path){
    vector<GLuint> tex;
    GLuint tmp;
    for(int i=0;i<nOfTex;i++){
        tmp=SOIL_load_OGL_texture(Util::newPath(Util::getDinamicPath(path,i,".png")),SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
        if(tmp){
            tex.push_back(NULL);
            tex[i]=tmp;
            textures.push_back(tex[i]);
            textureNames.push_back(Util::getDinamicPath((char*)name.c_str(),i,""));
        }else{if(Util::DEBUG)cout<<"erro em:"<<path<<endl;}
    }
    return tex;
}
void GL::setColor(nTColor color){
    GL::currentColor=color;
    glColor4f(color.R,color.G,color.B,color.A);
}
nTColor GL::getColor(){
    return GL::currentColor;
}
void GL::drawRectangle(nTRectangle pos,nTColor color){
    nTColor tmp=getColor();
    setColor(color);
    glBegin(GL_POLYGON);
        glVertex3f(pos.p0.x, pos.p0.y, pos.p0.z);
        glVertex3f(pos.p1.x, pos.p0.y, pos.p0.z);
        glVertex3f(pos.p1.x, pos.p1.y, pos.p0.z);
        glVertex3f(pos.p0.x, pos.p1.y, pos.p0.z);
    glEnd();
    setColor(tmp);
}

void GL::drawBox(nTRectangle pos,nTColor color){
    nTColor tmp=getColor();
    setColor(color);
    glBegin(GL_QUADS);
    glVertex3f(pos.p1.x,pos.p1.y,pos.p0.z); 
    glVertex3f(pos.p0.x,pos.p1.y,pos.p0.z);  
    glVertex3f(pos.p0.x,pos.p1.y,pos.p1.z);   
    glVertex3f(pos.p1.x,pos.p1.y,pos.p1.z);    
    glVertex3f(pos.p1.x,pos.p0.y,pos.p1.z);    
    glVertex3f(pos.p0.x,pos.p0.y,pos.p1.z); 
    glVertex3f(pos.p0.x,pos.p0.y,pos.p0.z);    
    glVertex3f(pos.p1.x,pos.p0.y,pos.p0.z);  
    glVertex3f(pos.p1.x,pos.p1.y,pos.p1.z);   
    glVertex3f(pos.p0.x,pos.p1.y,pos.p1.z);    
    glVertex3f(pos.p0.x,pos.p0.y,pos.p1.z);    
    glVertex3f(pos.p1.x,pos.p0.y,pos.p1.z);    
    glVertex3f(pos.p1.x,pos.p0.y,pos.p0.z);    
    glVertex3f(pos.p0.x,pos.p0.y,pos.p0.z);    
    glVertex3f(pos.p0.x,pos.p1.y,pos.p0.z);  
    glVertex3f(pos.p1.x,pos.p1.y,pos.p0.z);    
    glVertex3f(pos.p0.x,pos.p1.y,pos.p1.z);   
    glVertex3f(pos.p0.x,pos.p1.y,pos.p0.z);    
    glVertex3f(pos.p0.x,pos.p0.y,pos.p0.z);    
    glVertex3f(pos.p0.x,pos.p0.y,pos.p1.z); 
    glVertex3f(pos.p1.x,pos.p1.y,pos.p0.z);    
    glVertex3f(pos.p1.x,pos.p1.y,pos.p1.z);   
    glVertex3f(pos.p1.x,pos.p0.y,pos.p1.z);   
    glVertex3f(pos.p1.x,pos.p0.y,pos.p0.z); 
    glEnd();
    setColor(tmp);
}
void GL::drawTexture(nTRectangle pos,nTColor color,GLuint tex,int Orientation){
    nTColor tmp=getColor();
    setColor(color);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    if(Orientation<=0){
      glTexCoord2f(1, 0); glVertex3f(pos.p0.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(0, 0); glVertex3f(pos.p1.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(0, 1); glVertex3f(pos.p1.x, pos.p1.y, pos.p0.z);
      glTexCoord2f(1, 1); glVertex3f(pos.p0.x, pos.p1.y, pos.p0.z);
    }else if(Orientation==1){
      glTexCoord2f(0, 0); glVertex3f(pos.p0.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(1, 0); glVertex3f(pos.p1.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(1, 1); glVertex3f(pos.p1.x, pos.p1.y, pos.p0.z);
      glTexCoord2f(0, 1); glVertex3f(pos.p0.x, pos.p1.y, pos.p0.z);
    }else if(Orientation==2){
      glTexCoord2f(0, 0); glVertex3f(pos.p0.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(0, 1); glVertex3f(pos.p1.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(1, 1); glVertex3f(pos.p1.x, pos.p1.y, pos.p0.z);
      glTexCoord2f(1, 0); glVertex3f(pos.p0.x, pos.p1.y, pos.p0.z);
    }else if(Orientation==3){
      glTexCoord2f(1, 0); glVertex3f(pos.p0.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(1, 1); glVertex3f(pos.p1.x, pos.p0.y, pos.p0.z);
      glTexCoord2f(0, 1); glVertex3f(pos.p1.x, pos.p1.y, pos.p0.z);
      glTexCoord2f(0, 0); glVertex3f(pos.p0.x, pos.p1.y, pos.p0.z);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    setColor(tmp);
}
nTColor GL::getColorByName(string name){
  float R=1,G=1,B=1;
  if(name=="black"){
      R=0;G=0;B=0;
  }else if(name=="red"){
      R=1;G=0;B=0;
  }else if(name=="green"){
      R=0;G=1;B=0;
  }else if(name=="blue"){
      R=0;G=0;B=1;
  }else if(name=="yellow"){
      R=1;G=1;B=0;
  }else if(name=="magenta"){
      R=1;G=0;B=1;
  }else if(name=="cyan"){
      R=0;G=1;B=1;
  }else if(name=="grey"){
      R=0.6;G=0.6;B=0.6;
  }else if(name=="violet"){   //mish 153,112,205 ou 99,70,CD
      R=0.6;G=0.4392156;B=0.80392156;
  }else if(name=="white"){
      R=1;G=1;B=1;
  }else if(name=="mouseSelected"){
      R=0.3;G=0.3;B=0.3;
  }
  return Util::nTColorSet(R,G,B,1);
}
void GL::setFont(void* font){
    currentFont=font;
}
void GL::setFontByIndex(int idx){
    switch(idx){
        default:
            setFont(GLUT_BITMAP_TIMES_ROMAN_24);
        break;
        case 1:
            setFont(GLUT_BITMAP_8_BY_13);
        break;
        case 2:
            setFont(GLUT_BITMAP_9_BY_15);
        break;
        case 3:
            setFont(GLUT_BITMAP_TIMES_ROMAN_10);
        break;
        case 4:
            setFont(GLUT_BITMAP_HELVETICA_10);
        break;
        case 5:
            setFont(GLUT_BITMAP_HELVETICA_12);
        break;
        case 6:
            setFont(GLUT_BITMAP_HELVETICA_18);
        break;
    }
}
void GL::drawText(nTPoint point,char* text,nTColor color){
    nTColor tmp2=getColor();
    setColor(color);
    string tmp(text);
    glRasterPos3f(point.x,point.y,point.z);
    for (int i = 0;i<tmp.size();i++) {
       glutBitmapCharacter(currentFont, text[i]);
    }
    setColor(tmp2);
}
void GL::drawPolygon(nTPoint point,float radius,int edges){
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(point.x,point.y,point.z);
    for(int i=0;i<=edges;i++){
        glVertex3f(cos(2*M_PI*i/edges)*radius+point.x,sin(2*M_PI*i/edges)*radius+point.y,point.z);
    }
    glEnd();
}
GLuint GL::getTextureByName(string name){
    int i=0;
    for(string n:textureNames)
        if(n==name){
            return textures[i];
        }else
            i++;
    return 0;
}

string GL::getNameByTexture(GLuint tex){
    int i=0;
    for(GLuint n:textures)
        if(n==tex){
            return textureNames[i];
        }else
            i++;
    return "";
}

void GL::bindTexture(string name,bool setWhite){
    bindColor=getColor();
    if(setWhite)setColor(GL::getColorByName("white"));
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, getTextureByName(name));
}

void GL::unbindTexture(){
    glDisable(GL_TEXTURE_2D);
    setColor(bindColor);
}

vector<GLuint> GL::getTexturesByName(string name,int nOfTex){
    vector<GLuint> out;
    GLuint tmp;
    for(int i=0;i<nOfTex;i++){
        tmp=getTextureByName(Util::getDinamicPath((char*)name.c_str(),i,""));
        if(tmp)
            out.push_back(tmp);
    }
    return out;
}
bool GL::buttonBehave(nTRectangle collision,nTColor pressedColor,GLuint tex,void(*clickFunction)(int,int),void(*releaseFunction)(int,int),void(*RclickFunction)(int,int),void(*RreleaseFunction)(int,int)){
    if(GL::mousePos.x>=collision.p0.x&&GL::mousePos.x<=collision.p1.x&&((GL::mousePos.y>=collision.p0.y&&GL::mousePos.y<=collision.p1.y)||(GL::mousePos.y>=collision.p1.y&&GL::mousePos.y<=collision.p0.y))){
        if(tex){
            if(tex==GL::getTextureByName("LIDs"))
                GL::drawTexture(collision,GL::getColorByName("white"),GL::getTextureByName("LIDg"),1);
            else
                GL::drawTexture(collision,pressedColor,tex,1);
        }else
            GL::drawRectangle(collision,pressedColor);
        if(GL::leftMouseReleased||GL::rightMouseReleased)
            al->playSoundByName("mouse");
        if(GL::leftMouseClicked){
            if(*clickFunction!=NULL){
                clickFunction((int)GL::mousePos.x,(int)GL::mousePos.y);
            }
            return true;
        }else if(GL::leftMouseReleased){
            if(*releaseFunction!=NULL){
                 releaseFunction((int)GL::mousePos.x,(int)GL::mousePos.y);
            }
            GL::leftMouseReleased=0;
            return true;
        }
        if(GL::rightMouseClicked){
            if(*RclickFunction!=NULL){
                RclickFunction((int)GL::mousePos.x,(int)GL::mousePos.y);
            }
            return false;
        }else if(GL::rightMouseReleased){
            if(*RreleaseFunction!=NULL){
                 RreleaseFunction((int)GL::mousePos.x,(int)GL::mousePos.y);
            }
            GL::rightMouseReleased=0;
            return false;
        }
    }else{
        if(tex)
            GL::drawTexture(collision,GL::getColorByName("white"),tex,1);
    }
    return false;
}
ostream& operator<<(ostream &strm, const GL &gl) {
    if(Util::DEBUG)
        return strm <<"GL:["<<"FPS("<<gl.FPS<<"),"<<"CurrentColor("<<"R:"<<gl.currentColor.R<<" G:"<<gl.currentColor.G<<" B:"<<gl.currentColor.B<<" A:"<<gl.currentColor.A<<"),"<<
                "Loaded Texutres("<<gl.textures.size()<<"),"<<"ScreenSize("<<"x:"<<gl.currentSize.x<<" y:"<<gl.currentSize.y<<"),"<<"Is paused("<<gl.isPaused<<"),"<<"]\n";
    return strm;
}

void modifySound(int a,int b){
  AL::setSoundState(!AL::getSoundState());
  saveSettings();
}

void modifyMusic(int a,int b){
  AL::setMusicState(!AL::getMusicState());
  if(Scenes::current==Scenes::menu||Scenes::current==Scenes::options){
      al->playSoundByName("menu");
  }else if(Scenes::current==Scenes::game){
    char bff[10];
    snprintf(bff,10, "%d", rand()%4);
    string idM(bff);
    al->playSoundByName("music"+idM);
  }
  saveSettings();
}

vector<vector<float> > GL::getRotateMatrix(float angle){
    float rad=Util::angleToRad(angle);
    vector<vector<float> >out;
    out.resize(4,vector<float>(4,0));
    out[0][0]=cos(rad);
    out[0][1]=-sin(rad);
    out[1][1]=cos(rad);
    out[1][0]=sin(rad);
    out[2][2]=1;
    out[3][3]=1;
    return out;
}

vector<vector<float> > GL::getTranslateMatrix(nTPoint point){
    vector<vector<float> >out;
    out.resize(4,vector<float>(4,0));
    out[0][0]=1;
    out[1][1]=1;
    out[2][2]=1;
    out[3][3]=1;

    out[0][3]=point.x;
    out[1][3]=point.y;
    out[2][3]=point.z;
    return out;
}

nTPoint GL::rotatePoint(nTPoint point,nTPoint center, float angle){
    vector<vector<float> >Mpoint;
    Mpoint.resize(4,vector<float>(1,0));
    Mpoint[0][0]=point.x;
    Mpoint[1][0]=point.y;
    Mpoint[2][0]=point.z;
    Mpoint[3][0]=1;

    center.setPoint(-center.x,-center.y,-center.z);
    Mpoint=Util::multiplyMatrix(GL::getTranslateMatrix(center),Mpoint);
    Mpoint=Util::multiplyMatrix(GL::getRotateMatrix(angle),Mpoint);
    center.setPoint(-center.x,-center.y,-center.z);
    Mpoint=Util::multiplyMatrix(GL::getTranslateMatrix(center),Mpoint);
    point.setPoint(Mpoint[0][0],Mpoint[1][0],Mpoint[2][0]);

    return point;
}

nTPoint GL::getModelViewPoint(nTPoint point){
    GLfloat matrixf[16];
    vector<vector<float> >Mpoint;
    vector<vector<float> >out;

    Mpoint.resize(4,vector<float>(1,0));
    out.resize(4,vector<float>(4,0));

    glGetFloatv(GL_MODELVIEW_MATRIX, matrixf);

    Mpoint[0][0]=point.x;
    Mpoint[1][0]=point.y;
    Mpoint[2][0]=point.z;
    Mpoint[3][0]=1;

    out[0][0]=(float)matrixf[0];
    out[0][0]=(float)matrixf[1];
    out[0][0]=(float)matrixf[2];
    out[0][0]=(float)matrixf[3];
    out[0][0]=(float)matrixf[4];
    out[0][0]=(float)matrixf[5];
    out[0][0]=(float)matrixf[6];
    out[0][0]=(float)matrixf[7];
    out[0][0]=(float)matrixf[8];
    out[0][0]=(float)matrixf[9];
    out[0][0]=(float)matrixf[10];
    out[0][0]=(float)matrixf[11];
    out[0][0]=(float)matrixf[12];
    out[0][0]=(float)matrixf[13];
    out[0][0]=(float)matrixf[14];
    out[0][0]=(float)matrixf[15];
    Mpoint=Util::multiplyMatrix(out,Mpoint);

    point.setPoint(Mpoint[0][0],Mpoint[1][0],Mpoint[2][0]);
    return point;
}

float GL::getGameMs(){
    return GL::framesInGame*1000/GL::FPS;
}

void GL::drawCube(GLfloat size, GLenum type){
  static GLfloat n[6][3] =
  {
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0}
  };
  static GLint faces[6][4] =
  {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };
  GLfloat v[8][3];
  GLint i;
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;
  for (i = 5; i >= 0; i--) {
    glBegin(type);
     glNormal3fv(&n[i][0]);
     if(i==4)glTexCoord2f(0, 0);else if(i==0)glTexCoord2f(1, 0);else if(i==2)glTexCoord2f(0, 1);else glTexCoord2f(1, 1);glVertex3fv(&v[faces[i][0]][0]);
     if(i==4)glTexCoord2f(0, 1);else if(i==0)glTexCoord2f(0, 0);else if(i==2)glTexCoord2f(1, 1);else glTexCoord2f(1, 0);glVertex3fv(&v[faces[i][1]][0]);
     if(i==4)glTexCoord2f(1, 1);else if(i==0)glTexCoord2f(0, 1);else if(i==2)glTexCoord2f(1, 0);else glTexCoord2f(0, 0);glVertex3fv(&v[faces[i][2]][0]);
     if(i==4)glTexCoord2f(1, 0);else if(i==0)glTexCoord2f(1, 1);else if(i==2)glTexCoord2f(0, 0);else glTexCoord2f(0, 1);glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

void GL::drawSkyBox(string name,nTPoint center, float renderDist){
    
    nTColor tmp=getColor();
    setColor(GL::getColorByName("white"));
    glEnable(GL_TEXTURE_2D);    
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"B"));
    glBegin(GL_QUADS);      
        glTexCoord2f(1, 0); glVertex3f(center.x+renderDist,center.y,center.z);
        glTexCoord2f(1, 1); glVertex3f(center.x+renderDist,center.y+renderDist,center.z); 
        glTexCoord2f(0, 1); glVertex3f(center.x,center.y + renderDist,center.z);
        glTexCoord2f(0, 0); glVertex3f(center.x,center.y,center.z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"F"));
    glBegin(GL_QUADS);  
        glTexCoord2f(1, 0); glVertex3f(center.x,center.y,center.z+renderDist);
        glTexCoord2f(1, 1); glVertex3f(center.x,center.y+renderDist,center.z+renderDist);
        glTexCoord2f(0, 1); glVertex3f(center.x+renderDist,center.y+renderDist,center.z+renderDist); 
        glTexCoord2f(0, 0); glVertex3f(center.x+renderDist,center.y,center.z +renderDist);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"D"));
    glBegin(GL_QUADS);      

        glTexCoord2f(1, 0); glVertex3f(center.x,center.y,center.z);
        glTexCoord2f(1, 1); glVertex3f(center.x,center.y,center.z+renderDist);
        glTexCoord2f(0, 1); glVertex3f(center.x+renderDist,center.y,center.z+renderDist); 
        glTexCoord2f(0, 0); glVertex3f(center.x+renderDist,center.y,center.z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"U"));
    glBegin(GL_QUADS);      
        glTexCoord2f(0, 0); glVertex3f(center.x+renderDist,center.y+renderDist,center.z);
        glTexCoord2f(1, 0); glVertex3f(center.x+renderDist,center.y+renderDist,center.z+renderDist); 
        glTexCoord2f(1, 1); glVertex3f(center.x,center.y + renderDist,center.z+renderDist);
        glTexCoord2f(0, 1); glVertex3f(center.x,center.y+renderDist,center.z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"L"));
    glBegin(GL_QUADS);      
        glTexCoord2f(1, 1); glVertex3f(center.x,center.y+renderDist,center.z); 
        glTexCoord2f(0, 1); glVertex3f(center.x,center.y+renderDist,center.z+renderDist); 
        glTexCoord2f(0, 0); glVertex3f(center.x,center.y,center.z+renderDist);
        glTexCoord2f(1, 0); glVertex3f(center.x,center.y,center.z);     

    glEnd();
    glBindTexture(GL_TEXTURE_2D, GL::getTextureByName(name+"R"));
    glBegin(GL_QUADS);  
        glTexCoord2f(0, 0); glVertex3f(center.x+renderDist,center.y,center.z);
        glTexCoord2f(1, 0); glVertex3f(center.x+renderDist,center.y,center.z+renderDist);
        glTexCoord2f(1, 1); glVertex3f(center.x+renderDist,center.y+renderDist,center.z+renderDist); 
        glTexCoord2f(0, 1); glVertex3f(center.x+renderDist,center.y+renderDist,center.z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    setColor(tmp);
}


void GL::drawHUD(){
  if(!Scenes::cheatMode)return;
  GL::set2D(GL::currentSize.x, GL::currentSize.y);
  float sizeOfEachBlock=nTMap::scale.x*nTMap::scale.x;
  float sizeCol=sizeOfEachBlock+nTMap::scale.x/2;
  int playerInMapX=floor(Player::pos.x/sizeOfEachBlock);
  int playerInMapZ=floor(Player::pos.z/sizeOfEachBlock);
    vector<vector <int> > map = nTMap::current;
    nTColor tmp, color;
    int x0=0, x1=3, y0=3, y1=0, cont=0, cont2=0;
    nTPoint pa, pb;
    pa.setPoint(x0,y0,0);
    pb.setPoint(x1,y1,0);
    nTRectangle rect;
    rect.setRec(pa,pb);
    tmp=getColor();
    for(int i=0;i<map.size();i++){
        for(int j=0, x0=0, x1=3;j<map[0].size();j++){
          if(map[i][j]==0){
            color.setColor(1,1,1,1);
            setColor(color);
          }
          if(map[i][j]==1){
            color.setColor(0,0,0,1);
            setColor(color);
          }
          pa.setPoint(x0,y0,0), pb.setPoint(x1,y1,0);
          rect.setRec(pa,pb);
          drawRectangle(rect, color);
          x0+=3;
          x1+=3;
        }
        y0+=3;
        y1+=3;
    }
    color.setColor(0,1,0,1);
    setColor(color);
    for(int i=0; i<nTMap::wayOut.size(); i++){
      y0=nTMap::wayOut[i].i*3;
      y1=y0+3;
      x0=nTMap::wayOut[i].j*3;
      x1=x0+3;
      pa.setPoint(x0,y0,0.5);
      pb.setPoint(x1,y1,0.5);
      rect.setRec(pa,pb);
      drawRectangle(rect, color);
    }
    x0=Player::pos.x/3*sizeOfEachBlock;
    x1=x0+3;
    y0=Player::pos.z/3*sizeOfEachBlock;
    y1=y0+3;
    pa.setPoint(x0,y0,1);
    pb.setPoint(x1,y1,1);
    rect.setRec(pa,pb);
    color.setColor(0,1,1,1);
    setColor(color);
    drawRectangle(rect, color);

    for(int i=0, y0=3, y1=0;i<map.size();i++){
        for(int j=0, x0=0, x1=3;j<map[0].size();j++){
    if(map[i][j]==2 && cont==0){
      color.setColor(1,0,0,1);
      setColor(color);
      cont++;
      pa.setPoint(x0,y0,1), pb.setPoint(x1,y1,1);
      rect.setRec(pa,pb);
      drawRectangle(rect, color);
    }
    if(map[i][j]==3 && cont2==0){
      color.setColor(0,0,1,1);
      setColor(color);
      cont2++;
      pa.setPoint(x0,y0,1), pb.setPoint(x1,y1,1);
      rect.setRec(pa,pb);
      drawRectangle(rect, color);
    }
    x0+=3;
    x1+=3;
  }
  y0+=3;
  y1+=3;
}
    setColor(tmp);
    GL::set3D(GL::currentSize.x, GL::currentSize.y);
}

void GL::drawPause(){
    GL::set2D(GL::currentSize.x,GL::currentSize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSetCursor(GLUT_CURSOR_INHERIT);
    glNewList(GLlist,GL_COMPILE_AND_EXECUTE);
     if(AL::getSoundState()){
        GL::buttonBehave(Util::nTRectangleSet(700+Scenes::camera.x.movedCam,50+Scenes::camera.y.movedCam,740+Scenes::camera.x.movedCam,10+Scenes::camera.y.movedCam,1,1),Util::nTColorSet(0,0,0,1),GL::getTextureByName("soundOn"),NULL,*modifySound,NULL,NULL);
      }else{
        GL::buttonBehave(Util::nTRectangleSet(700+Scenes::camera.x.movedCam,50+Scenes::camera.y.movedCam,740+Scenes::camera.x.movedCam,10+Scenes::camera.y.movedCam,1,1),Util::nTColorSet(0,0,0,1),GL::getTextureByName("soundOff"),NULL,*modifySound,NULL,NULL);
      }
      if(AL::getMusicState()){
        GL::buttonBehave(Util::nTRectangleSet(640+Scenes::camera.x.movedCam,50+Scenes::camera.y.movedCam,680+Scenes::camera.x.movedCam,10+Scenes::camera.y.movedCam,1,1),Util::nTColorSet(0,0,0,1),GL::getTextureByName("musicOn"),NULL,*modifyMusic,NULL,NULL);
      }else{
        GL::buttonBehave(Util::nTRectangleSet(640+Scenes::camera.x.movedCam,50+Scenes::camera.y.movedCam,680+Scenes::camera.x.movedCam,10+Scenes::camera.y.movedCam,1,1),Util::nTColorSet(0,0,0,1),GL::getTextureByName("musicOff"),NULL,*modifyMusic,NULL,NULL);
      }
        GL::setFontByIndex(0);
        string text="Pausado";
        GL::drawText(Util::nTPointSet(360+Scenes::camera.x.movedCam,Scenes::camera.y.movedCam+300,1),(char*)text.c_str(),GL::getColorByName("white"));
        if(GL::buttonBehave(Util::nTRectangleSet(GL::defaultSize.x/2+GL::defaultSize.x/6,300+220,GL::defaultSize.x/2+2*GL::defaultSize.x/6,200+220,0.4,0.4),Util::nTColorSet(0.4,0.4,0.4,1),GL::getTextureByName("sair"),NULL,NULL,NULL,NULL)){
            Light::disableAllLights(); 
            Scenes::current=Scenes::menu;
        }
    glEndList();
}

