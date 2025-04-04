/*
 * Operating Systems (2INC0) Practical Assignment
 * Threading
 *
 * Intersection Part 1
 * 
 * Michał Surażyński (1967665)
 * Bram van Waes (2001624)
 * Mateusz Rozmyslowicz (1966243)
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
static pthread_mutex_t intersection_mutex = PTHREAD_MUTEX_INITIALIZER;      // Mutex for the critical section
typedef enum {RED = 0, GREEN = 1} Colors;                                   // Colors of the traffic lights
typedef struct                                                              // Defines the traffic light
{
  int side;
  int direction;
  int color;
  int cars_serviced;
} Traffic_Light;

const static int num_directions = 4;                                        // Number of lanes per direcion
const static int num_sides = 4;                                             // Number of directions
                              
static Traffic_Light traffic_lights[4][4];                                  // Static array of all traffic lights 



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
  Traffic_Light * traffic_light_ptr;            // Pointer to a member of traffic_lights global array       
  traffic_light_ptr = (Traffic_Light *) arg;    // Casting before dereferencing              
  
  // Pointers to the attributes of the traffic light
  int *side_ptr = &traffic_light_ptr->side;
  int *direction_ptr = &traffic_light_ptr->direction;
  int *color_ptr = &traffic_light_ptr->color;
  int *cars_serviced_ptr = &traffic_light_ptr->cars_serviced;

  fprintf(stderr, "Light thread side %d x dir %d: Started.\n", *side_ptr, *direction_ptr);


  // TODO:
  // while not all arrivals have been handled, repeatedly:
  //  - wait for an arrival using the semaphore for this traffic light
  //  - lock the right mutex(es)
  //  - make the traffic light turn green
  //  - sleep for CROSS_TIME seconds
  //  - make the traffic light turn red
  //  - unlock the right mutex(es)
  while (true) {
    // Either triggered from supplier when car arrived or from main when END_TIME is over
    sem_wait(&car_sem[*side_ptr][*direction_ptr]);

    // Check if END_TIME is over
    if (get_time_passed() >= END_TIME) {
      fprintf(stderr, "Light thread side %d x dir %d: Closed.\n", *side_ptr, *direction_ptr);
      free(arg);
      return(0);
    } 
    // If not over continue
    

    // Request access to the critical section
    pthread_mutex_lock(&intersection_mutex);

    // Check if END_TIME is over agian (thread could have been waiting on mutex for some time after the last check of END_TIME on the semaphore)
    if (get_time_passed() >= END_TIME) {
      fprintf(stderr, "Light thread side %d x dir %d: Closed.\n", *side_ptr, *direction_ptr);
      free(arg);
      pthread_mutex_unlock(&intersection_mutex);  
      return(0);
    } 
    // If not over continue
    
    // Turn green for the car with some ID
    int car_id = curr_car_arrivals[*side_ptr][*direction_ptr][*cars_serviced_ptr].id;
    printf("traffic light %d %d turns green at time %d for car %d\n", *side_ptr, *direction_ptr, get_time_passed(), car_id);
    *color_ptr = GREEN;

    sleep(CROSS_TIME);
    
    // Turn red after the car
    *color_ptr = RED;
    printf("traffic light %d %d turns red at time %d\n", *side_ptr, *direction_ptr, get_time_passed());
    
    // Remember that the car was serviced
    (*cars_serviced_ptr)++;

    // End of critical section
    pthread_mutex_unlock(&intersection_mutex);
  }
}


int main(int argc, char * argv[])
{
  // Create semaphores to wait/signal for arrivals
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      sem_init(&car_sem[i][j], 0, 0);
    }
  }

  // Start the timer
  start_time();
  
  
  // TODO: create a thread per traffic light that executes manage_light
  pthread_t light_ids [num_sides][num_directions];

  for (int s = 0; s < num_sides; s++) {
    for (int d = 0; d < num_directions; d++) {
      // Create a traffic light with side s, direction d, color red and 0 serviced cars
      traffic_lights[s][d] = (Traffic_Light){s, d, RED, 0};      

      Traffic_Light * parameter;
      parameter = malloc (sizeof (Traffic_Light));    // Memory will be freed by the child-thread
      *parameter = traffic_lights[s][d];        
      
      // Initialize the traffic light thread
      if (!pthread_create (&light_ids[s][d], NULL, manage_light, parameter) == 0) {
        fprintf(stderr, "Light thread side %d x dir %d: Failed to create.\n", s, d);     
      } else {
        fprintf(stderr, "Light thread side %d x dir %d: Created.\n", s, d);               
      }
    }
  }


  // TODO: create a thread that executes supply_cars()
  pthread_t supplier_id;
  // Initialize the supplier thread
  if (!pthread_create (&supplier_id, NULL, supply_cars, NULL) == 0) {
    fprintf(stderr, "Supplier thread: Failed to create.\n");      
  } else {
    fprintf(stderr, "Supplier thread: Created.\n");               
  }


  // TODO: wait for all threads to finish
  while (get_time_passed() < END_TIME) {        // Wait until END_TIME
    continue;
  } 

  for (int s = 0; s < num_sides; s++) {
    for (int d = 0; d < num_directions; d++) {
      sem_post(&car_sem[s][d]);                 // Send terminating signal to the lights after time > 40
      pthread_join (light_ids[s][d], NULL);     // Wait for that thead to terminate
    }
  }
  pthread_join (supplier_id, NULL);             // Wait for the termiantion of supplier thread


  // Destroy semaphores
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_destroy(&car_sem[i][j]);
    }
  }

  return(0);
}
