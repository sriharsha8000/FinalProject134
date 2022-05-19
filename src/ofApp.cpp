
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name: Koshi Huynh and Sriharsha Edukulla
//  Date:05/18/2022


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {
	fuel = 20000;

	font.load("OpenSans-Bold.ttf", 40);

	thrustSound.load("thrust.mp3");
	bgMusic.load("bg.mp3");
	bgMusic.setVolume(0.25);

	background.loadImage("background.jpg");
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(80);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	cam.setPosition(glm::vec3(0, 5, 10));
	cam.lookAt(lander.getPosition());
	ofEnableSmoothing();
	ofEnableDepthTest();
	currentCam = &cam;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/Terrain.obj");
	mars.setScaleNormalization(false);

	lander.loadModel("geo/Lander.obj");
	lander.setPosition(0, 0, 0);
	lander.setScale(0.005, 0.005, 0.005);
	bLanderLoaded = true;

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load the shader
	//
	shader.load("shaders/shader.frag");
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(thrustSlider.setup("Thrust", 8, 0, 20));
	bHide = false;

	//  Create Octree for testing.
	//
	octree.create(mars.getMesh(0), 20);

	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	glm::vec3 landerPos = lander.getPosition();

	trackingCam.setPosition(10, 15, 15);
	trackingCam.lookAt(glm::vec3(landerPos.x, landerPos.y, landerPos.z));
	trackingCam.setNearClip(.1);

	bottomCam.setPosition(glm::vec3(landerPos.x, landerPos.y, landerPos.z));
	bottomCam.lookAt(glm::vec3(0, -1, 0));
	bottomCam.setNearClip(.1);

	topCam.setPosition(glm::vec3(landerPos.x, 1000, landerPos.z));
	topCam.lookAt(glm::vec3(0, 1, 0));
	topCam.setNearClip(.1);

	currentCam = &cam;

	ImpulseRadialForce* radialForce = new ImpulseRadialForce(1000.0);
	GravityForce* gravityForce = new GravityForce(glm::vec3(0, -10, 0));
	TurbulenceForce* turbForce = new TurbulenceForce(glm::vec3(-5, -5, -5), glm::vec3(5, 5, 5));
	explosion.setLifespan(10);
	explosion.setParticleRadius(0.5);
	explosion.setOneShot(true);
	explosion.setGroupSize(50);
	explosion.setEmitterType(SphereEmitter);
	explosion.sys->addForce(turbForce);
	explosion.sys->addForce(gravityForce);
	explosion.sys->addForce(radialForce);

	GravityForce* gravityForce2 = new GravityForce(glm::vec3(0, -5, 0));
	TurbulenceForce* turbForce2 = new TurbulenceForce(glm::vec3(-30, -10, -30), glm::vec3(30, 0, 30));
	exhaust.setLifespan(.2);
	exhaust.setParticleRadius(0.1);
	exhaust.setOneShot(true);
	exhaust.setGroupSize(10);
	exhaust.setEmitterType(SphereEmitter);
	exhaust.sys->addForce(gravityForce2);
	exhaust.sys->addForce(turbForce2);

	dynamicLight.setup();
	dynamicLight.enable();
	dynamicLight.setSpotlight();
	dynamicLight.setScale(1);
	dynamicLight.setSpotlightCutOff(10);
	dynamicLight.setAttenuation(.2, .001, .001);
	dynamicLight.setAmbientColor(ofFloatColor(1, 1, 1));
	dynamicLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	dynamicLight.setSpecularColor(ofFloatColor(1, 1, 1));
	dynamicLight.rotate(180, ofVec3f(0, 1, 0));
	dynamicLight.setPosition((ofVec3f)(landerPos.x, landerPos.y + 10, landerPos.z));
	dynamicLight.rotate(90, ofVec3f(1, 0, 0));
	

}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	
	if (fuel <= 0) {
		thrustSound.stop();
		gameplay = 5;
	}

	glm::vec3 landerPos = lander.getPosition();
	exhaust.setPosition(landerPos );
	explosion.setPosition(landerPos);
	exhaust.update();
	explosion.update();

	thrust = thrustSlider;
	if (!bgMusic.isPlaying()) bgMusic.play();

	//CAMERAS
	trackingCam.lookAt(landerPos);
	bottomCam.setPosition(landerPos);
	topCam.setPosition(glm::vec3(landerPos.x, landerPos.y + 20, landerPos.z));


	dynamicLight.setPosition((ofVec3f)(landerPos.x, landerPos.y + 20, landerPos.z));
	//COLLISION
	checkCollision();

	//ALTITUDE CHECKER
	Ray altitudeRay = Ray(Vector3(landerPos.x, landerPos.y, landerPos.z), Vector3(landerPos.x, landerPos.y - 200, landerPos.z));
	TreeNode node;
	if (octree.intersect(altitudeRay, octree.root, node))
	{
		altitude = glm::length(octree.mesh.getVertex(node.points[0]) - lander.getPosition());
	}

	//HERE, CHANGE COORDINATES TO EACH LANDER SPOT
	if (gameplay == 0 && bGrounded && withinCircle(glm::vec3(5, 5, -4))) {
		gameplay = 1;
	}
	else if (gameplay == 1 && bGrounded && withinCircle(glm::vec3(20, 20, -4))) {
		gameplay = 2;
	}
	else if (gameplay == 2 && bGrounded && withinCircle(glm::vec3(40, 50, -4))) {
		gameplay = 3;
	}


	//GAME IS IN PLAY
	if (gameplay > -1 && gameplay < 3) {
		if (bGrounded && !keymap[OF_KEY_UP] && gameplay != 4)
		{
			velocity = glm::vec3(0, 0, 0);
			acceleration = glm::vec3(0, 0, 0);
			force = glm::vec3(0, 0, 0);
		}
		if (keymap[OF_KEY_UP]) {
			force += thrust * ofVec3f(0, 1, 0);
			fuel -= 10;
			if (!thrustSound.isPlaying()) thrustSound.play();
			exhaust.sys->reset();
			exhaust.start();
			bGrounded = false;
		}
		if (keymap[OF_KEY_LEFT] && !bGrounded) {
			force += -thrust * ofVec3f(1, 0, 0);
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
		if (keymap[OF_KEY_RIGHT] && !bGrounded) {
			force += thrust * ofVec3f(1, 0, 0);
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
		if (keymap[OF_KEY_DOWN] && !bGrounded) {
			force += -thrust * ofVec3f(0, 1, 0);
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
		if (keymap['q'] && !bGrounded) {
			force += thrust * ofVec3f(0, 0, 1);
			if (!thrustSound.isPlaying()) thrustSound.play();

		}
		if (keymap['w'] && !bGrounded) {
			force += -thrust * ofVec3f(0, 0, 1);
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
		if (keymap['e'] && !bGrounded) {
			angularForce += 50;
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
		if (keymap['r'] && !bGrounded) {
			angularForce += -50;
			if (!thrustSound.isPlaying()) thrustSound.play();
		}
	}

	if (gameplay == 0 || gameplay == 1 || gameplay == 2 || gameplay == 4) {
		integrate();
		force = glm::vec3(0, 0, 0);
		angularForce = 0;
	}
}
//--------------------------------------------------------------
void ofApp::draw() {

	loadVbo();
	ofBackground(ofColor::black);
	glDepthMask(false);
	background.draw(0, 0, -2.41437);
	if (!bHide) gui.draw();
	glDepthMask(true);

	currentCam->begin();
	ofPushMatrix();
	if (gameplay == 0 || gameplay == -1) {
		makeLanding(glm::vec3(5, 5, -4));
	}
	else if (gameplay == 1) {
		makeLanding(glm::vec3(20, 20, -4));
	}
	else if (gameplay == 2) {
		makeLanding(glm::vec3(40, 50, -4));
	}

	if (bWireframe) {                    // wireframe mode  (include landerBoundsaxis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}
	// this makes everything look glowy :) 
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	// begin drawing in the camera
	shader.begin();

	// draw particle emitter here..
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)exhaust.sys->particles.size());
	exhaust.draw();

	particleTex.unbind();

	//  end drawing in the camera
	// 
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	glDepthMask(GL_TRUE);
	ofEnableAlphaBlending();






	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
	}
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}
	explosion.draw();
	ofPopMatrix();
	currentCam->end();
	drawText();
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	keymap[key] = true;
	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'z':
		toggleWireframeMode();
		break;
	case '1':
		currentCam = &trackingCam;
		break;
	case '2':
		currentCam = &bottomCam;
		break;
	case '3':
		currentCam = &topCam;
		break;
	case ' ':
		if (gameplay == -1) {
			gameplay = 0;
			starttime = ofGetElapsedTimeMillis();
		}
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
	keymap[key] = false;
	switch (key) {
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {


}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f& pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {
	// Setup 3 - Light System
		
	keyLight.setup();
	keyLight.enable();
	keyLight.setAreaLight(1, 1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));

	keyLight.rotate(45, ofVec3f(0, 1, 0));
	keyLight.rotate(-45, ofVec3f(1, 0, 0));
	keyLight.setPosition(5, 5, 5);

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(00);
	fillLight.setAttenuation(2, .001, .001);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	fillLight.rotate(-10, ofVec3f(1, 0, 0));
	fillLight.rotate(-45, ofVec3f(0, 1, 0));
	fillLight.setPosition(-5, 5, 5);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(.05);
	rimLight.setSpotlightCutOff(00);
	rimLight.setAttenuation(.2, .001, .001);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	rimLight.rotate(180, ofVec3f(0, 1, 0));
	rimLight.setPosition(0, 5, -7);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
		//		lander.setScale(.1, .1, .1);
			//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

void ofApp::checkCollision()
{
	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();

	Box roverBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	colBoxList.clear();
	if (octree.intersect(roverBounds, octree.root, colBoxList))
	{
		glm::vec3 temp = force + velocity;
		if (temp.y < -4) {
			cam.lookAt(lander.getPosition());
			currentCam = &cam;
			velocity = glm::vec3(0, 200, 0);
			explosion.setPosition(lander.getPosition());
			explosion.sys->reset();
			explosion.start();
			gameplay = 4;
		}
		else {
			bGrounded = true;
			thrustSound.stop();
		}
	}

}

void ofApp::drawText()
{
	ofSetColor(ofColor::white);
	string alt = "Altitude: " + std::to_string(altitude);
	string fue = "Fuel: " + std::to_string(fuel);
	int framerate = ofGetFrameRate();
	string fps = "Frame Rate:" + std::to_string(framerate);
	string start = "PRESS SPACE TO START";
	string controls = "THESE ARE THE CONTROLS"; 
	string up = "UP ARROW TO MOVE UPWORDS";
	string rightleft = "RIGHT ARROW FOR MOVING RIGHT AND LEFT ARROW FOR MOVING LEFT";
	string rotate = "E AND R TO ROTATE";
	string zmovement = "Q AND W TO MOVE ALONG Z AXIS";	
	string end = "GAME OVER";
	string explode = "YOU EXPLODED\n  GAME OVER";
	string outOfFuel = "YOU RAN OUT OF FUEL\n    GAME OVER";
	


	ofDrawBitmapString(alt, ofGetWindowWidth() - 170, 55);
	ofDrawBitmapString(fue, ofGetWindowWidth() - 170, 35);
	ofDrawBitmapString(fps, ofGetWindowWidth() - 120, 15);

	if (gameplay == -1) {
		font.drawString(start, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2);
		ofDrawBitmapString(controls, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2 + 50);
		ofDrawBitmapString(up, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2 + 70);
		ofDrawBitmapString(rightleft, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2 + 90);
		ofDrawBitmapString(rotate, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2 + 110);
		ofDrawBitmapString(zmovement, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2 + 140);
	}
	else if (gameplay == 3) {

		font.drawString(end, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2);

	}
	else if (gameplay == 4) {

		font.drawString(explode, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2);
	}
	else if (gameplay == 5) {
		font.drawString(outOfFuel, ofGetWindowWidth() / 2 - 250, ofGetWindowHeight() / 2-50);

	}
}

void ofApp::makeLanding(glm::vec3 pos) {
	ofPushMatrix();
	ofRotateX(100);
	ofSetColor(ofColor::blue);
	ofNoFill();
	ofDrawEllipse(pos, 10, 10);
	ofPopMatrix();
}

bool ofApp::withinCircle(glm::vec3 pos) {
	glm::vec3 landerPos = lander.getPosition();
	return ((landerPos.x > pos.x - 5 && landerPos.x < pos.x + 5)
		&& (landerPos.z > pos.y - 5 && landerPos.z < pos.y + 5));
}



void ofApp::loadVbo()
{
	if (exhaust.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < exhaust.sys->particles.size(); i++)
	{
		points.push_back(exhaust.sys->particles[i].position);
		sizes.push_back(ofVec3f(20));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

