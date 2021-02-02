#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "pcg.h"
 
#define GRID_SIZE 10240
#define NSTEPS    32000
#define NMARKERS  160000

struct mrk{
  int id;
  int thread;
  int team;
  pcg32_random_t rng;
  int location;
  float integral;
};
typedef struct mrk mrk_t;


#pragma omp declare target
int stepMarker(mrk_t *, float *);
#pragma omp end declare target

int stepMarker(mrk_t *marker, float *grid){
  marker->location = (int) pcg32_boundedrand_r( &(marker->rng), GRID_SIZE);
  marker->integral += grid[marker->location];
  return 0;
}

int initField(int gridSize, float *grid){
  int i;
  for(i=0;i<gridSize;i++){
    grid[i] = 1.0;
  }
  return 0;
}


int initMarkers(mrk_t *markers, int nMarks){
  int i;
  for(i=0;i<nMarks;i++){
    pcg32_srandom_r( &(markers[i].rng),  0x853c49e6748fea9bULL, i);
    markers[i].id=i;
    markers[i].integral=0;
  }

  return 0;
}

int printMarker(mrk_t marker){
  printf("M%05d: @%06d by tm%04d:th%04d integral %f\n", 
	 marker.id,    marker.location, 
	 marker.team,  marker.thread,
	 marker.integral);
  return 0;
}



int main(void){

  int i;
  mrk_t *markers;
  const int nMarks = NMARKERS;
  float *grid;

  int *nFinished; /* This variable is here to test atomic/critical pragmas */
  int N;


  markers = (mrk_t *) malloc( nMarks * sizeof(mrk_t));
  grid    = (float *) malloc( GRID_SIZE * sizeof(float));
 

  initMarkers(markers,nMarks);
  initField(GRID_SIZE,grid);

  nFinished = &N;
  *nFinished = -1; /* Just mark this with something non-default*/
  printf("Finished %d/%d markers.\n",*nFinished,nMarks);


  /*Move all the data to the target on one go*/

  #pragma omp target data map(tofrom:markers[0:nMarks]) map(to:grid[0:GRID_SIZE]) map(tofrom:nFinished[0:1])
  {

    /*
    if ( omp_is_initial_device() ) {
    	  printf("Running on host (target data map).\n");
         } else {
          printf("Running on target (target data map)\n");
         }
   
    */


    #pragma omp atomic write
    (*nFinished) = 0;   /* Test the atomic save.*/
 
    #pragma omp target teams distribute parallel for  /* shared(nFinished) */
    for(i=0;i<nMarks;i++){

      if(i==0){
	if ( omp_is_initial_device() ) {
	  printf("Running on host (ottdpf)\n");
	}
	else {
	  printf("Running on arget (ottdpf)\n");
	}
      }

      markers[i].thread = omp_get_thread_num();
      markers[i].team   = omp_get_team_num();
      int j;
      for(j=0;j<NSTEPS;j++){
        stepMarker( &(markers[i]), grid );

      }
    #pragma omp atomic update
      (*nFinished)++;
      /* printf("i=%5d ready=%5d\n",i,*nFinished); */
    }
    /* printf("** i=----- ready=%5d **\n",*nFinished); */
  }

  printf("Finished %d/%d markers.\n",*nFinished,nMarks);
  
  for (i=0; i<nMarks; i+=19331){
    printMarker(markers[i]);

  }


}
