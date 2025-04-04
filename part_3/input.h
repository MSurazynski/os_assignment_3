#ifndef INPUT_H
#define INPUT_H

#include "arrivals.h"

// the time in seconds it takes for a car to cross the intersection
#define CROSS_TIME 5

// the time in seconds the traffic lights and railway manager should be alive
#define END_TIME 40

// the array of car arrivals for the intersection
const Car_Arrival input_car_arrivals[] = { {0, EAST, LEFT, 0}, {1, WEST, LEFT, 1}, {2, SOUTH, STRAIGHT, 8}, {3, SOUTH, UTURN, 10} };

// the array of train arriavls for the intersection
const Train_Arrival input_train_arrivals[] = {{0, 5, 2}};

#endif
