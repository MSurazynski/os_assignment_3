#ifndef INPUT_H
#define INPUT_H

#include "arrivals.h"

// the time in seconds it takes for a car to cross the intersection
#define CROSS_TIME 5

// the time in seconds the traffic lights and railway manager should be alive
#define END_TIME 40

// the array of car arrivals for the intersection
const Car_Arrival input_car_arrivals[] = { {0, EAST, LEFT, 2}, {1, WEST, STRAIGHT, 4}, {2, SOUTH, RIGHT, 6} };

// the array of train arriavls for the intersection
const Train_Arrival input_train_arrivals[] = {{0, 5, 3}, {1, 3, 6}};

#endif
