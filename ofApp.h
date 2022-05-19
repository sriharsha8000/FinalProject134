#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "../ParticleEmitter.h"



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent2(ofDragInfo dragInfo);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
	void toggleSelectTerrain();
	void setCameraTarget();
	bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point);
	bool raySelectWithOctree(ofVec3f& pointRet);
	glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p, glm::vec3 n);
	void loadVbo();
	void checkCollision();

	void drawText();

	void makeLanding(glm::vec3 pos);
	bool withinCircle(glm::vec3 pos);

	float thrust;
	glm::vec3 velocity = glm::vec3(0, 0, 0);
	glm::vec3 acceleration = glm::vec3(0, 0, 0);
	glm::vec3 force = glm::vec3(0, 0, 0);
	glm::vec3 gravity = glm::vec3(0, -2.5, 0);
	float rotation = 0.0;
	float angularForce = 0;
	float angularVelocity = 0.0;
	float angularAcceleration = 0.0;
	float altitude;
	map<int, bool> keymap;


	void ofApp::integrate() {
		float framerate = 60;	// workaround
		float dt = 1.0 / framerate;

		glm::vec3 pos = lander.getPosition();
		lander.setPosition(pos.x + velocity.x * dt, pos.y + velocity.y * dt, pos.z + velocity.z * dt);
		glm::vec3 accel = acceleration;
		accel += (force + gravity);
		velocity += accel * dt;
		velocity *= .999;

		rotation += (angularVelocity * dt);

		lander.setRotation(0, rotation, 0, 1, 0);
		float a = angularAcceleration;
		a += angularForce;
		angularVelocity += a * dt;
		angularVelocity *= .999;
	}
	ofTrueTypeFont font;
	int gameplay = -1;

	ofSoundPlayer thrustSound;
	ofSoundPlayer bgMusic;

	ofEasyCam* currentCam;
	ofEasyCam cam;
	ofEasyCam trackingCam;
	ofEasyCam bottomCam;
	ofEasyCam topCam;

	ofxAssimpModelLoader mars, lander;
	//ofLight light;
	Box boundingBox, landerBounds;
	Box testBox;
	vector<Box> colBoxList;
	bool bLanderSelected = false;
	Octree octree;
	TreeNode selectedNode;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;

	ofxIntSlider numLevels;
	ofxFloatSlider thrustSlider;
	ofxPanel gui;
	int fuel;

	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bWireframe;
	bool bDisplayPoints;
	bool bPointSelected;
	bool bHide;
	bool pointSelected = false;
	bool bDisplayLeafNodes = false;
	bool bDisplayOctree = false;
	bool bDisplayBBoxes = false;
	bool bGrounded = false;

	bool bLanderLoaded;
	bool bTerrainSelected;

	ofVec3f selectedPoint;
	ofVec3f intersectPoint;

	vector<Box> bboxList;

	const float selectionRange = 4.0;

	ParticleEmitter exhaust;
	ParticleEmitter explosion;

	ofVbo vbo;
	ofShader shader;
	ofTexture  particleTex;

	ofLight keyLight, fillLight, rimLight,dynamicLight;
	vector<ofLight*> Lights;
	ofImage background;

	float starttime;
};

