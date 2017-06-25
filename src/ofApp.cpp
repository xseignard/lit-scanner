#include "ofApp.h"

using namespace ofxCv;
using namespace cv;
using namespace std;

static const ofColor red = ofColor::fromHex(0xff0000);
static const ofColor green = ofColor::fromHex(0x00ff00);
static const ofColor white = ofColor::fromHex(0xffffff);


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	// camera and opencv settings
	finder.setup("haarcascade_frontalface_default.xml");
	finder.setPreset(ObjectFinder::Fast);
	#ifdef TARGET_OPENGLES
	camSettings.width = 1920;
	camSettings.height = 1080;
	camSettings.enableTexture = true;
	camSettings.doRecording = false;
	cam.setup(camSettings);
	#else
	cam.setup(800, 600);
	#endif
	// udp settings
	client.Create();
	client.Bind(8888);
	client.SetNonBlocking(true);
	// misc
	state = "idle";
	ofTrueTypeFont::setGlobalDpi(72);
	font.load("VT323-Regular.ttf", 60, true, true);
	font.setLineHeight(68.0);
	font.setLetterSpacing(1.035);
	// rpi stuff
	#ifdef TARGET_OPENGLES
	setupGPIOs();
	#endif
}

//--------------------------------------------------------------
#ifdef TARGET_OPENGLES
void ofApp::setupGPIOs(){
	// setup wiring pi
	if(wiringPiSetup() == -1){
		ofLogNotice(__func__) << "Error on wiringPi setup";
	}
	// relay pin
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH);
}
#endif

//--------------------------------------------------------------
void ofApp::update() {
	#ifndef TARGET_OPENGLES
	cam.update();
	if(cam.isFrameNew()) {
		finder.update(cam);
	}
	#endif
	char data[10];
	client.Receive(data, 10);
	string msg = data;
	if (msg == "0") {
		state = "idle";
		#ifdef TARGET_OPENGLES
		digitalWrite(RELAY_PIN, HIGH);
		#endif
		ofLog(OF_LOG_NOTICE, "Relay on");
	}
	else if (msg == "1") {
		state = "refused";
		#ifdef TARGET_OPENGLES
		digitalWrite(RELAY_PIN, HIGH);
		#endif
		ofLog(OF_LOG_NOTICE, "Relay on");
	}
	else if (msg == "2") {
		state = "ok";
		#ifdef TARGET_OPENGLES
		digitalWrite(RELAY_PIN, LOW);
		#endif
		ofLog(OF_LOG_NOTICE, "Relay off");
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	#ifdef TARGET_OPENGLES
	cam.draw();
	#else
	cam.draw(0, 0);
	#endif
	if (finder.size() > 0) {
		if (state == "refused") drawTracker(finder.getObjectSmoothed(0), "ACCÈS REFUSÉ", red);
		else if (state == "ok") drawTracker(finder.getObjectSmoothed(0), "ACCÈS AUTORISÉ", green);
	}
}

//--------------------------------------------------------------
void ofApp::drawTracker(ofRectangle rect, string text, ofColor color) {
	float top = rect.getTop();
	float scale = .85 * rect.width / font.stringWidth(text);
	ofSetColor(color);
	ofPushMatrix();
		ofTranslate(rect.x, rect.y);
		ofScale(scale, scale);
		font.drawString(text, 0, 0);
	ofPopMatrix();
	ofPushMatrix();
		ofTranslate(rect.x + rect.width + 10, rect.y + rect.height / 2 );
		ofScale(scale / 3, scale / 3);
		string data = to_string(rect.getCenter().x);
		data += ", ";
		data += to_string(rect.getCenter().y);
		data += "\n// ";
		data += to_string(rect.width);
		data += " // ";
		data += to_string(rect.height);
		font.drawString(data, 0, 0);
	ofPopMatrix();
	path.clear();
	path.setStrokeWidth(10);
	path.setFilled(false);
	path.setStrokeColor(color);
	path.rectangle(rect);
	path.draw();
	ofSetColor(white);

}
