#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "pcg.h"
 
#define GRID_SIZE 10240
#define NSTEPS    32000
#define NMARKERS  160000



#pragma omp declare target
int stepMarker(float *markers_integral, int *markers_location, uint64_t *markers_rng_state, uint64_t *markers_rng_inc, float *grid, int i);
#pragma omp end declare target

int stepMarker(float *markers_integral, int *markers_location, uint64_t *markers_rng_state, uint64_t *markers_rng_inc, float *grid, int i){

  int location;
  float integral = markers_integral[i];
  uint64_t state = markers_rng_state[i];
  uint64_t inc = markers_rng_inc[i];
  uint32_t threshold = -GRID_SIZE % GRID_SIZE;
  
  for(int j=0;j<NSTEPS;j++){
      //stepMarker( &(markers[i]), grid );
      // 

    //location = (int) pcg32_boundedrand_r( &(state), &(inc), GRID_SIZE, threshold);

    //uint32_t threshold = -GRID_SIZE % GRID_SIZE;

    for (;;) {
      //uint32_t r = pcg32_random_r(rng_state, rng_inc);
      uint64_t oldstate = state;
      state = oldstate * 6364136223846793005ULL + inc;
      uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
      uint32_t rot = oldstate >> 59u;
      uint32_t r = (int)((xorshifted >> rot) | (xorshifted << ((-rot) & 31)));

      if (r >= threshold)
        location = r % GRID_SIZE;
        break;
      }
      

      integral += grid[location];
  }
  markers_location[i] = location;
  markers_integral[i] = integral;
  markers_rng_state[i] = state;
  return 0;
}

int initField(int gridSize, float *grid){
  int i;
  for(i=0;i<gridSize;i++){
    grid[i] = 1.0;
  }
  return 0;
}


int initMarkers(float *markers_integral, int *markers_id, uint64_t *markers_rng_state, uint64_t *markers_rng_inc, int nMarks){
  int i;
  for(i=0;i<nMarks;i++){
    pcg32_srandom_r( &(markers_rng_state[i]), &(markers_rng_inc[i]),  0x853c49e6748fea9bULL, i);
    markers_id[i]=i;
    markers_integral[i]=0;
  }

  return 0;
}



int main(void){

  int i;
  //mrk_t *markers;
  const int nMarks = NMARKERS;
  float *grid;

  //markers = (mrk_t *) malloc( nMarks * sizeof(mrk_t));

  //pcg32_random_t *markers_rng = (pcg32_random_t*) malloc(nMarks * sizeof(pcg32_random_t));

  int *markers_id = (int*) malloc(nMarks * sizeof(int));
  int *markers_thread = (int*) malloc(nMarks * sizeof(int));
  int *markers_team = (int*) malloc(nMarks * sizeof(int));
  uint64_t *markers_rng_state = (uint64_t*) malloc(nMarks * sizeof(uint64_t));
  uint64_t *markers_rng_inc = (uint64_t*) malloc(nMarks * sizeof(uint64_t));
  int *markers_location = (int*) malloc(nMarks * sizeof(int));
  float *markers_integral = (float*) malloc(nMarks * sizeof(float));

  grid    = (float *) malloc( GRID_SIZE * sizeof(float));
 

  initMarkers(markers_integral, markers_id, markers_rng_state, markers_rng_inc, nMarks);
  initField(GRID_SIZE,grid);

  double start = omp_get_wtime();

  /*Move all the data to the target on one go*/
  #pragma omp target data map(tofrom:markers_id[0:nMarks], markers_thread[0:nMarks], markers_team[0:nMarks], markers_rng_state[0:nMarks], markers_rng_inc[0:nMarks], markers_location[0:nMarks], markers_integral[0:nMarks]) map(to:grid[0:GRID_SIZE])
  {



    if ( omp_is_initial_device() ) {
    	  printf("Running on host.\n");
         } else {
          printf("Running on target\n");
         }

    double startNoData = omp_get_wtime();
    #pragma omp target teams distribute parallel for   private(i) 
    for(i=0;i<nMarks;i++){
    
      /*
      if ( omp_is_initial_device() ) {
	printf("Host\n");
      }
      else {
	printf("Target\n");
      }
      */
      markers_thread[i] = omp_get_thread_num();
      markers_team[i]   = omp_get_team_num();
      stepMarker(markers_integral, markers_location, markers_rng_state, markers_rng_inc, grid, i);

    }
    printf("runtime no data transfers %f sec \n", omp_get_wtime() - startNoData);

  }
  printf("runtime with data transfers %f sec \n", omp_get_wtime() - start);


  
  for (i=0; i<20; i++){
    //printMarker(markers[i]);
    printf("M%05d: @%06d by tm%04d:th%04d integral %f\n", 
	    markers_id[i],    markers_location[i], 
	    markers_team[i],  markers_thread[i],
	    markers_integral[i]);
  }


}
