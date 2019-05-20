#include "main.h"

enum CONTROL_STYLE { ARCADE_CONTROL, TANK_CONTROL, LUDICROUS_MODE };

enum DR4B_STATE { MOVE_MAX, MOVE_MIN, MOVE_MANUAL };
/*
    Maps from (-127) -> 127 to itself using the function
    (((128 * pow(4, ((abs(x)-50)/12.5)))/(pow(4, ((abs(x)-50)/12.5))+1))) - 1) * sign(x)

    Used to ease in/out joystick movement for more precise control.

    If the horizontal input is at 25%, it only has 5% of the power, reducing the amount
   that the robot will slowly veer off course fron an imperfect control stick.

    A lookup table of pre-computed values is used to reduce computation.
*/
int sigmoidMap[255] = {
    -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127,
    -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127,
    -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125,
    -124, -124, -124, -123, -123, -123, -122, -122, -121, -120, -119, -119, -118, -117,
    -116, -114, -113, -112, -110, -108, -107, -105, -103, -100, -98,  -95,  -93,  -90,
    -87,  -84,  -80,  -77,  -74,  -70,  -67,  -63,  -59,  -56,  -52,  -49,  -46,  -42,
    -39,  -36,  -33,  -31,  -28,  -26,  -23,  -21,  -19,  -18,  -16,  -14,  -13,  -12,
    -10,  -9,   -8,   -7,   -7,   -6,   -5,   -4,   -4,   -3,   -3,   -3,   -2,   -2,
    -2,   -1,   -1,   -1,   -1,   0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    1,
    1,    1,    2,    2,    2,    3,    3,    3,    4,    4,    5,    6,    7,    7,
    8,    9,    10,   12,   13,   14,   16,   18,   19,   21,   23,   26,   28,   31,
    33,   36,   39,   42,   46,   49,   52,   56,   59,   63,   67,   70,   74,   77,
    80,   84,   87,   90,   93,   95,   98,   100,  103,  105,  107,  108,  110,  112,
    113,  114,  116,  117,  118,  119,  119,  120,  121,  122,  122,  123,  123,  123,
    124,  124,  124,  125,  125,  125,  125,  126,  126,  126,  126,  126,  126,  126,
    126,  126,  126,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
    127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,  127,
    127,  127,  127};

const int LU_FLAME_INTENSITY = 5; // How on fire is the robot?
void luInit() {
	srand(time(0)); // Initialize random number generator.
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
const double FULL_DR4B_EXTENSION_DEG = 40;

void opcontrol() {
	CONTROL_STYLE controlMode = TANK_CONTROL;
	DR4B_STATE fourBarState = MOVE_MANUAL;
	short switchCooldown = 0;

	int luModL = 0;
	int luModR = 0;
	long counter;

	while (true) {
#pragma region 'Drive'
		/*
		  Tank control - Default control style.

		  Left joystick controls left wheels, right joystick controls right wheels.
		 */
		if (controlMode == TANK_CONTROL) {
			int left = sigmoidMap[master.get_analog(ANALOG_LEFT_Y) + 127];
			int right = sigmoidMap[master.get_analog(ANALOG_RIGHT_Y) + 127];

			leftFrontDrive = left;
			leftBackDrive = left;

			rightFrontDrive = right;
			rightBackDrive = right;
		}
		/*
		  Arcade control

		  Left joystick controls the entire robot. Up/down for forward/backwards,
		  left/right for turning.
		*/
		else if (controlMode == ARCADE_CONTROL) {
			int forward = sigmoidMap[master.get_analog(ANALOG_LEFT_Y) + 127];
			int steer = sigmoidMap[master.get_analog(ANALOG_LEFT_X) + 127];

			leftFrontDrive = forward + steer;
			leftBackDrive = forward + steer;

			rightFrontDrive = forward - steer;
			rightBackDrive = forward - steer;
		}
		/*
		  Training mode

		  In this mode the robot does tank drive, but changes the speed on each side by
		  random amounts. (Imagine robot is on fire)
		*/
		else if (controlMode == LUDICROUS_MODE) {
			int left = sigmoidMap[master.get_analog(ANALOG_LEFT_Y) + 127];
			int right = sigmoidMap[master.get_analog(ANALOG_RIGHT_Y) + 127];

			luModL += (rand() % LU_FLAME_INTENSITY * 2) - LU_FLAME_INTENSITY;
			luModR += (rand() % LU_FLAME_INTENSITY * 2) - LU_FLAME_INTENSITY;

			// Keep modifier within half the speed so robot isn't impossible to control.
			if (luModL > 127) {
				luModL = 127;
			} else if (luModL < -127) {
				luModL = -127;
			}
			if (luModR > 127) {
				luModR = 127;
			} else if (luModR < -127) {
				luModR = -127;
			}

			// Apply fire modifier to robot.
			leftFrontDrive = left + luModL;
			leftBackDrive = left + luModL;

			rightFrontDrive = left - luModL;
			rightBackDrive = right - luModR;
		}
		/*
		  Control style switching logic.

		  Switches from tank to arcade and vice versa when all four shoulder
		  buttons are held down.

		  0.5 second cooldown between switches.
		*/
		switchCooldown--;
		if (master.get_digital(DIGITAL_L1) && master.get_digital(DIGITAL_L2) &&
		    master.get_digital(DIGITAL_R1) && master.get_digital(DIGITAL_R2) &&
		    switchCooldown <= 0) {
			// Toggle modes
			if (controlMode = TANK_CONTROL) {
				controlMode = ARCADE_CONTROL;
			} else if (controlMode = ARCADE_CONTROL) {
				controlMode = LUDICROUS_MODE;
				luInit();
			} else if (controlMode = LUDICROUS_MODE) {
				controlMode = TANK_CONTROL;
			}
			switchCooldown = 25;
		}
#pragma endregion

#pragma region 'DR4B'
		/*
		    Double reverse four bar control.

		    X - Raise
		    B - Lower

		    ↑ - All the way up unless L1 is pressed
		    ↓ - All the way down unlesss L2 is pressed
		    ← - Move up + outtake
		*/

		// Manual movement
		if (fourBarState == MOVE_MANUAL) {
			if (master.get_digital(DIGITAL_X)) {
				reverseFourLeft = 127;
				reverseFourRight = 127;
			} else if (master.get_digital(DIGITAL_B)) {
				reverseFourLeft = -127;
				reverseFourRight = -127;
			} else {
				reverseFourLeft = 0;
				reverseFourRight = 0;
			}
		} else {
			if (reverseFourLeft.get_position() > FULL_DR4B_EXTENSION_DEG - 5 &&
			    reverseFourRight.get_position() > FULL_DR4B_EXTENSION_DEG - 5 &&
			    fourBarState == MOVE_MAX) {
				fourBarState = MOVE_MANUAL;
				reverseFourLeft = 0;
				reverseFourRight = 0;
			} else if (reverseFourLeft.get_position() > 5 &&
			           reverseFourRight.get_position() > 5 && fourBarState == MOVE_MIN) {
				fourBarState = MOVE_MANUAL;
				reverseFourLeft = 0;
				reverseFourRight = 0;
			}
		}
		// Move to highest
		if (master.get_digital(DIGITAL_UP)) {
			fourBarState = MOVE_MAX;

			// TODO: actually measure this
			reverseFourLeft.move_absolute(FULL_DR4B_EXTENSION_DEG, 127);
			reverseFourRight.move_absolute(FULL_DR4B_EXTENSION_DEG, 127);
		}
		// Move to lowest
		else if (master.get_digital(DIGITAL_DOWN)) {
			fourBarState = MOVE_MIN;

			reverseFourLeft.move_absolute(0, -127);
			reverseFourRight.move_absolute(0, -127);
		}
#pragma endregion

#pragma region "Intake/Claw"
		/*
		    Y - Intake/Close
		    A - Outtake/Open
		*/
		if (master.get_digital(DIGITAL_Y)) {
			intakeLeft = 127;
			intakeRight = 127;
		} else if (master.get_digital(DIGITAL_A)) {
			intakeLeft = -127;
			intakeRight = -127;
		} else {
			intakeLeft = 0;
			intakeRight = 0;
		}
#pragma endregion
		counter++;
		if (counter % 25 == 0) {
			master.print(0, 0, "%f, %f", reverseFourLeft.get_position(),
			             reverseFourRight.get_position());
		}
		pros::delay(20);
	}
}
