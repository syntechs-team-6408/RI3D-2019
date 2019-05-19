#include "main.h"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

pros::Motor leftFrontDrive(1, pros::E_MOTOR_GEARSET_18, false,
                           pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor rightFrontDrive(2, pros::E_MOTOR_GEARSET_18, false,
                            pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor leftBackDrive(3, pros::E_MOTOR_GEARSET_18, false,
                          pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor rightBackDrive(4, pros::E_MOTOR_GEARSET_18, false,
                           pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor reverseFourLeft(5, pros::E_MOTOR_GEARSET_18, true,
                            pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor reverseFourRight(6, pros::E_MOTOR_GEARSET_18, false,
                             pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor intakeLeft(7, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor intakeRight(8, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);
/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	leftFrontDrive.set_brake_mode(MOTOR_BRAKE_COAST);
	leftBackDrive.set_brake_mode(MOTOR_BRAKE_COAST);
	rightFrontDrive.set_brake_mode(MOTOR_BRAKE_COAST);
	rightBackDrive.set_brake_mode(MOTOR_BRAKE_COAST);

	reverseFourRight.set_brake_mode(MOTOR_BRAKE_HOLD);
	reverseFourLeft.set_brake_mode(MOTOR_BRAKE_HOLD);

	intakeLeft.set_brake_mode(MOTOR_BRAKE_COAST);
	intakeRight.set_brake_mode(MOTOR_BRAKE_COAST);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}
