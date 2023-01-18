#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "pcg.h"
 
#define GRID_SIZE 10240
#define NSTEPS    320000
#define D_NSTEPS  0
#define NMARKERS  1000000

#define TOLERANCE 0.01

struct mrk{
  int id;
  int thread;
  int team;
  int stopAt;
  pcg32_random_t rng;
  int location;
  float integral;
};
typedef struct mrk mrk_t;


#pragma omp declare target
int stepMarker(mrk_t *, float *, int);
#pragma omp end declare target

int stepMarker(mrk_t *marker, float *grid, int gridsize){
  marker->location = (int) pcg32_boundedrand_r( &(marker->rng), gridsize);
  marker->integral += grid[marker->location];
  return 0;
}

int initField(int gridSize, float *grid){
  int i;

  /* Init the field s.t. the average = 1.0  */
  for(i=0;i<gridSize;i++){
    grid[i] = 2.0 *  ( (float) i ) / (  (float) gridSize ) ;
  }
  return 0;
}


int initMarkers(mrk_t *markers, int nMarks, int nsteps, int d_nsteps){
  int i;
  for(i=0;i<nMarks;i++){
    pcg32_srandom_r( &(markers[i].rng),  0x853c49e6748fea9bULL, i);
    markers[i].id=i;
    markers[i].integral=0;
    markers[i].stopAt = nsteps + rand() / (RAND_MAX / (2*d_nsteps + 1) + 1) - d_nsteps;
  }

  return 0;
}

int printMarker(mrk_t marker){
  if ( fabs(marker.integral - 1.000 ) < TOLERANCE ) {
    printf("M%05d: @%06d by tm%04d:th%04d s%d integral %f\n",
	   marker.id,    marker.location,
	   marker.team,  marker.thread,
	   marker.stopAt, marker.integral);
  } else {
    printf("M%05d: @%06d by tm%04d:th%04d s%d integral %f !!!!!!!!!!!!\n",
	   marker.id,    marker.location,
	   marker.team,  marker.thread,
	   marker.stopAt, marker.integral);

  }
    return 0;
}


int parse_arguments( int argc, char *argv[], int *gridsize, int *nsteps, int *d_nsteps){

  char *endptr;
  
  /* Get the gridsize as input */
  if(argc >= 2 ){
    *gridsize = strtol(argv[1],&endptr,10);
    if(errno == EINVAL || *gridsize <= 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable gridsize.\n", argv[1], *gridsize);
      return 1;
    }
  } else {
    *gridsize = GRID_SIZE;
  }

  /* Get the number of steps and its variability as input */
  if(argc >= 3 ){
    *nsteps =  strtol(argv[2],&endptr,10);
    if(errno == EINVAL || *nsteps <= 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable gridsize.\n", argv[2], *nsteps);
      return 2;
    }
  } else {
    *nsteps = NSTEPS;
  }

  if(argc >= 4 ){
    *d_nsteps =  strtol(argv[3],&endptr,10);
    if(errno == EINVAL || *d_nsteps < 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable variability of steps.\n", argv[3], *nsteps);
      return 3;
    }
  } else {
    *d_nsteps = D_NSTEPS;
  }


  

  return 0;
}


int main( int argc, char *argv[] ){

  int i;
  int resval;
  mrk_t *markers;
  const int nMarks = NMARKERS;
  float *grid;
  int gridsize;

  int nsteps, d_nsteps;
  
  int maxTeam,maxThread;

  struct timespec  wc_begin,wc_end,cpu_begin,cpu_end;

#ifdef USE_ATOMIC
  int *nFinished; /* This variable is here to test atomic/critical pragmas */
#endif
  int N;

  int initialDevice[1];

  resval =  parse_arguments( argc, argv, &gridsize, &nsteps, &d_nsteps);
  if(resval != 0) {
    return resval;
  }
  
  printf("Markers %d; Steps %d pm %d; Gridsize %d.\n", nMarks, nsteps, d_nsteps, gridsize );

  markers = (mrk_t *) malloc( nMarks * sizeof(mrk_t));
  grid    = (float *) malloc( gridsize * sizeof(float));


  
  initMarkers(markers,nMarks,nsteps,d_nsteps);
  initField(gridsize,grid);

#ifdef USE_ATOMIC
  nFinished = &N;
  *nFinished = -1; /* Just mark this with something non-default*/
  printf("Finished %d/%d markers.\n",*nFinished,nMarks);
#endif
  
  clock_gettime( CLOCK_REALTIME,           &wc_begin  );
  clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &cpu_begin );

  /*Move all the data to the target on one go*/
  
#ifdef USE_ATOMIC
  #pragma omp target data map(tofrom:markers[0:nMarks]) map(to:grid[0:gridsize]) map(tofrom:nFinished[0:1])
#else
  #pragma omp target data map(tofrom:markers[0:nMarks]) map(to:grid[0:gridsize])
#endif
  {

    /*
    if ( omp_is_initial_device() ) {
    	  printf("Running on host (target data map).\n");
         } else {
          printf("Running on target (target data map)\n");
         }
   
    */

#pragma omp target map(from:initialDevice[0:1])
    {
      initialDevice[0] = omp_is_initial_device();
    }

    if ( initialDevice[0]  ) {
      printf("Running on host\n");
    }
    else {
      printf("Running on target\n");
    }
    fflush(stdout);


    #pragma omp target
    {
#ifdef USE_ATOMIC
    #pragma omp atomic write
    (*nFinished) = 0;   /* Test the atomic save.*/
#endif
    }

#ifdef USE_ATOMIC
    #pragma omp target teams distribute parallel for  shared(nFinished) 
#else
    #pragma omp target teams distribute parallel for
#endif
    for(i=0;i<nMarks;i++){


      markers[i].thread = omp_get_thread_num();
      markers[i].team   = omp_get_team_num();
      int j;
      for(j=0;j<markers[i].stopAt;j++){
        stepMarker( &(markers[i]), grid, gridsize );
      }

      /* Scale the integral by number of steps */
      markers[i].integral /= (float) markers[i].stopAt;

#ifdef USE_ATOMIC
    #pragma omp atomic update
      (*nFinished)++;
      /* printf("i=%5d ready=%5d\n",i,*nFinished); */
#endif
    }
    /* printf("** i=----- ready=%5d **\n",*nFinished); */
  }

  clock_gettime( CLOCK_REALTIME,           &wc_end  );
  clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &cpu_end );
    
#ifdef USE_ATOMIC
  printf("Finished %d/%d markers.\n",*nFinished,nMarks);
#endif
  
  printf("Wall clock time: %lf s\n",  (double) ( wc_end.tv_sec- wc_begin.tv_sec) + ((double) ( wc_end.tv_nsec- wc_begin.tv_nsec ))*1.0e-9 ); 
  printf("       CPU time: %lf s\n",  (double) (cpu_end.tv_sec-cpu_begin.tv_sec) + ((double) (cpu_end.tv_nsec-cpu_begin.tv_nsec ))*1.0e-9 ); 
  
  for (i=0; i<nMarks; i+=19331){
    printMarker(markers[i]);
  }

  maxThread = -1;
  maxTeam = -1;
  for (i=0; i<nMarks; i++){
    if ( markers[i].team   > maxTeam  ) maxTeam   =  markers[i].team;
    if ( markers[i].thread > maxThread) maxThread =  markers[i].thread;
  }

  printf("Teams %d Threads %d\n", maxTeam+1, maxThread+1);

  return 0;
}
