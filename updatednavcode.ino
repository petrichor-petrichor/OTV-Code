#include <math.h>
#include "Enes100.h"

/*
* DISCLAIMER: We used GPT-4 to generate the code that the distance sensor uses to read distance (getDistance)
*/

const double NORTH = M_PI/2;
const double SOUTH = -(M_PI/2);
const double MARGIN = M_PI/20;

// pins for the distance sensors (PWM)

// left
const int TRIGGER_1 = 2;
const int ECHO_1 = 3;

// right
const int TRIGGER_2 = 4;
const int ECHO_2 = 5;

// pins for motor control (PWM)

// left motor
const int EN_A = 8;
const int IN_1 = 9;
const int IN_2 = 10;

// right motor
const int EN_B = 13;
const int IN_3 = 11;
const int IN_4 = 12;

/* The code inside void setup() runs only once, before the code in void loop(). */
void setup() {

	Enes100.begin("Trial By Fire", FIRE, 209, 50, 51); // param: name, mission, aruco, tx, rx
  Serial.begin(9600); // DO NOT CHANGE, ESP8266 BAUD IS 9600

  pinMode(EN_A, OUTPUT);
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(EN_B, OUTPUT);
  pinMode(IN_3, OUTPUT);
  pinMode(IN_4, OUTPUT);
  pinMode(TRIGGER_1, OUTPUT);
  pinMode(TRIGGER_2, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(ECHO_2, INPUT);

}

/* The code in void loop() runs repeatedly forever */ 
void loop() { 
	
  double missionLocation;

  getY();
	
 /*
 * face the mission site
 */

	if (getY() > 1.0) {
		
    Enes100.println(getY());
		faceDir(SOUTH);
    Enes100.println("facing direction");

  /*
  the aruco is offset from the body of the OTV, and mission site y 
  is the exact y coordinate of the center of the site, offset accordingly
  */

    missionLocation = Enes100.missionSite.y + 0.11;
		
	} else {
		
    Enes100.println(getY());
		faceDir(NORTH);
    Enes100.println("facing direction");
    missionLocation = Enes100.missionSite.y - 0.11;
		
	}

  Enes100.println("done");

  /*
  * move to the mission site
  */
	
	moveToLocation(false, missionLocation);
	Enes100.updateLocation();
	
	if (topStart) {
		
		faceDir(Enes100.location.theta + NORTH);
		
	} else {
		
		faceDir(Enes100.location.theta + SOUTH);
		
	}
	Enes100.updateLocation();
	
	/* Obstacle avoidance logic */

  /*

  update the condition of this while loop to be 
  when the OTV is outside of the obstacle zone
  instead of while(1)

  */

	while (1) {
		
		Enes100.updateLocation();
		
		if (getDistance(TRIGGER_1, ECHO_1) <= 30 || getDistance(TRIGGER_2, ECHO_2) <= 0.3) {
			
      setMotorSpeed(0);

			if (Enes100.location.y < 1.0) {
				faceDir(Enes100.location.theta + NORTH);
				moveToLocation(false, 1.0);
				faceDir(Enes100.location.theta - NORTH);
			} else {
				faceDir(Enes100.location.theta + SOUTH);
				moveToLocation(false, 1.0);
				faceDir(Enes100.location.theta - SOUTH);
			}

		} else if (Enes100.location.x > 3) {
			
			if (Enes100.location.y <= 1.2) {
				faceDir(Enes100.location.theta + NORTH);
				while (Enes100.location.y < 1.2) {
					Enes100.updateLocation();
					setMotorSpeed(100);
				}
				faceDir(Enes100.location.theta - NORTH);
				setMotorSpeed(100);
			} else {
				setMotorSpeed(100);
			}
			
		} else {
			setMotorSpeed(100);
		}
		
	}

  /*

  need to add logic here to move OTV to top half of the arena and face the log and go through to end

  */
	
	while(1);  // Circumvent the loop and ensure the above statements only get run once.
	
}

/* This is an example function to make both motors drive
 * at the given power.
 */

double getX() {

  Enes100.updateLocation();

  return Enes100.location.x;

}

double getY() {

  Enes100.updateLocation();

  return Enes100.location.y;

}

double getHeading() {

  Enes100.updateLocation();

  return Enes100.location.theta;

}

void setMotorSpeed(int PWMspeed) {

  
  if (PWMspeed == 0) {

    digitalWrite(EN_A, LOW);
    digitalWrite(EN_B, LOW);
    return;

  }

  analogWrite(EN_A, PWMspeed);
  analogWrite(EN_B, PWMspeed);

}

void setMotorDir(int motor, bool forward) {


  // left motor
  if (motor == 1) {

    if (forward) {

      digitalWrite(IN_1, LOW);
      digitalWrite(IN_2, HIGH);

    } else {

      digitalWrite(IN_1, HIGH);
      digitalWrite(IN_2, LOW);

    }

  // right motor
  } if (motor == 2) {

    if (forward) {

      digitalWrite(IN_3, LOW);
      digitalWrite(IN_4, HIGH);

    } else {

      digitalWrite(IN_3, HIGH);
      digitalWrite(IN_4, LOW);

    }
    
  }

}

void rotateRight(int PWMspeed) {

  setMotorDir(1, true);
  setMotorDir(2, false);
  setMotorSpeed(PWMspeed);

}

void rotateLeft(int PWMspeed) {

  setMotorDir(1, false);
  setMotorDir(2, true);
  setMotorSpeed(PWMspeed);

}

void faceDir(double limit) {

  bool oriented = false;

	do {

    Enes100.println(getHeading());

    if (getHeading() <= limit + MARGIN && getHeading() >= limit - MARGIN) {

      setMotorSpeed(0);
      oriented = true;

    }
	
		if (getHeading() < limit - MARGIN) {
	
			rotateLeft(120);
		
		}
	
		if (getHeading() > limit + MARGIN) {
		
			rotateRight(120);
		
		}

    delay(50);
		
	} while (!oriented);

  setMotorSpeed(0);

	
}

void moveToLocation(bool xDir, double limit) {

  setMotorDir(1, true);
  setMotorDir(2, true);
	
	if (xDir) {
		
		moveX(limit);	
		
	} else {
		
		moveY(limit);
		
	}
	
}

void moveX(double limit) {
	
	bool done = false;
	
	do {
		
		Enes100.updateLocation();
		
		if (Enes100.location.x <= limit + MARGIN && Enes100.location.x >= limit - MARGIN) {
			
			setMotorSpeed(0);
			done = true;
			return;
			
		} 
		
		setMotorSpeed(90);
		
	} while (!done);
	
}
	
void moveY(double limit) {
	
	bool done = false;
	
	do {
		
		Enes100.updateLocation();
		
		if (Enes100.location.y <= limit + MARGIN && Enes100.location.y >= limit - MARGIN) {
			
			setMotorSpeed(0);
			done = true;
			return;
			
		} 
		
		setMotorSpeed(90);
		
	} while (!done);
	
}

int getDistance(int trigger_pin, int echo_pin) {

  // Send ultrasonic pulse
  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);

  // Receive echo and calculate distance in cm
  long duration = pulseIn(echo_pin, HIGH);
  int distance = (duration * 0.0343) / 2;
  return distance;

}
