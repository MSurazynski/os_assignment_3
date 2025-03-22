/*
 * Operating Systems (2INC0) Practical Assignment
 * Threading
 *
 * Intersection Part [REPLACE WITH PART NUMBER]
 * 
 * Michał Surażyński (1967665)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 * STUDENT_NAME_3 (STUDENT_NR_3)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "arrivals.h"
#include "intersection_time.h"
#include "input.h"

// TODO: Global variables: mutexes, data structures, etc...
// Mutex for the critical section
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int num_lanes = 4;
static int num_directions = 4;
static int num_lights = 16;
typedef struct 
{
  int direction;
  int lane;
} light_loc;




/* 
 * curr_car_arrivals[][][]
 *
 * A 3D array that stores the arrivals that have occurred
 * The first two indices determine the entry lane: first index is Side, second index is Direction
 * curr_arrivals[s][d] returns an array of all arrivals for the entry lane on side s for direction d,
 *   ordered in the same order as they arrived
 */
static Car_Arrival curr_car_arrivals[4][4][20];

/*
 * car_sem[][]
 *
 * A 2D array that defines a semaphore for each entry lane,
 *   which are used to signal the corresponding traffic light that a car has arrived
 * The two indices determine the entry lane: first index is Side, second index is Direction
 */
static sem_t car_sem[4][4];

/*
 * supply_cars()
 *
 * A function for supplying car arrivals to the intersection
 * This should be executed by a separate thread
 */
static void* supply_cars()
{
  fprintf(stderr, "Supplier thread: Started.\n");    

  int t = 0;
  int num_curr_arrivals[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

  // for every arrival in the list
  for (int i = 0; i < sizeof(input_car_arrivals)/sizeof(Car_Arrival); i++)
  {
    // get the next arrival in the list
    Car_Arrival arrival = input_car_arrivals[i];
    // wait until this arrival is supposed to arrive
    sleep(arrival.time - t);
    t = arrival.time;
    // store the new arrival in curr_arrivals
    curr_car_arrivals[arrival.side][arrival.direction][num_curr_arrivals[arrival.side][arrival.direction]] = arrival;
    num_curr_arrivals[arrival.side][arrival.direction] += 1;
    // increment the semaphore for the traffic light that the arrival is for
    sem_post(&car_sem[arrival.side][arrival.direction]);
  }

  fprintf(stderr, "Supplier thread: Closed.\n");
  return(0);
}


/*
 * manage_light(void* arg)
 *
 * A function that implements the behaviour of a traffic light
 */
static void* manage_light(void* arg)
{
  // Getting the function parameter
  light_loc * argi;           // pointer to the function paramter value 
  light_loc i;                // function paramter
  argi = (light_loc *) arg;   // proper casting before dereferencing (could also be done in one statement)
  i = *argi;                  // get the integer value of the pointer
  free (arg);                 // we retrieved the value, so now the pointer can be deleted
  
  int direction = i.direction;
  int lane = i.lane;
  fprintf(stderr, "Thread Dir %d x Lane %d: Started.\n", direction, lane);
  

  // TODO:
  // while not all arrivals have been handled, repeatedly:
  //  - wait for an arrival using the semaphore for this traffic light
  //  - lock the right mutex(es)
  //  - make the traffic light turn green
  //  - sleep for CROSS_TIME seconds
  //  - make the traffic light turn red
  //  - unlock the right mutex(es)


  fprintf(stderr, "Thread Dir %d x Lane %d: Closed.\n", direction, lane);
  pthread_exit(0);
}


int main(int argc, char * argv[])
{
  // create semaphores to wait/signal for arrivals
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_init(&car_sem[i][j], 0, 0);
    }
  }

  // start the timer
  start_time();
  
  // TODO: create a thread per traffic light that executes manage_light
  pthread_t light_ids [num_lights];

  for (int d = 0; d < num_directions; d++) {
    for (int l = 0; l < num_lanes; l++) {
      light_loc * parameter;
      parameter = malloc (sizeof (int));  // memory will be freed by the child-thread
      light_loc loc = {d, l};
      *parameter = loc;        // assign an arbitrary value...   

      if (!pthread_create (&light_ids[d * num_directions + l], NULL, manage_light, parameter) == 0) {
        fprintf(stderr, "Thread Dir %d x Lane %d: Failed to create.\n", d, l);      // in case of failure
      } else {
        fprintf(stderr, "Dir %d x Lane %d: Created.\n", d, l);               // in case of correct execution
      }
    }
  }


  // TODO: create a thread that executes supply_cars()
  pthread_t supplier_id;
  if (!pthread_create (&supplier_id, NULL, supply_cars, NULL) == 0) {
    fprintf(stderr, "Supplier thread: Failed to create.\n");      // in case of failure
  } else {
    fprintf(stderr, "Supplier thread: Created.\n");               // in case of correct execution
  }


  // TODO: wait for all threads to finish
  for (int i = 0; i < num_lights; i++) {
    pthread_join (light_ids[i], NULL);
  }
  pthread_join (supplier_id, NULL);


  // destroy semaphores
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_destroy(&car_sem[i][j]);
    }
  }
}
