#include <iostream>
#include <CL/sycl.hpp>
#include "pcg-cpp-0.98/include/pcg_random.hpp"
#include <random>


#define TOLERANCE 0.01

struct mrk{
  int id;
  int thread;
  int team;
  int stopAt;
  pcg32 rng;
  int location;
  float integral;
};
typedef struct mrk mrk_t;


int parse_arguments( int argc, char *argv[], int *gridsize, int *nsteps, int *d_nsteps, bool &cpu){

  char *endptr;
  
  /* Get the gridsize as input */
  if(argc >= 2 ){
    *gridsize = strtol(argv[1],&endptr,10);
    if(errno == EINVAL || *gridsize <= 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable gridsize.\n", argv[1], *gridsize);
      return 1;
    }
  }

  /* Get the number of steps and its variability as input */
  if(argc >= 3 ){
    *nsteps =  strtol(argv[2],&endptr,10);
    if(errno == EINVAL || *nsteps <= 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable numbero of steps.\n", argv[2], *nsteps);
      return 2;
    }
  }

  if(argc >= 4 ){
    *d_nsteps =  strtol(argv[3],&endptr,10);
    if(errno == EINVAL || *d_nsteps < 0 ){
      printf("Interpreting '%s' as %d, which is not reasonable variability of steps.\n", argv[3], *d_nsteps);
      return 3;
    }
  }

  if(argc >= 5 ){
    cpu = false;
    if        ( !strcasecmp(argv[4],"cpu") ) {
      cpu = true;

    } else if ( !strcasecmp(argv[4],"gpu") ) {
      cpu = false;
    } else {
      printf("The 4th parameter '%s' is not 'cpu' or 'gpu'.\n", argv[4]);
      return 4;
    }
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


int initMarkers(mrk_t *markers, int nMarks, int nsteps, int d_nsteps){
  int i;
  //pcg_extras::seed_seq_from<std::random_device> seed_source;
  for(i=0;i<nMarks;i++){
    //pcg32_srandom_r( &(markers[i].rng),  0x853c49e6748fea9bULL, i);
    //markers[i].rng = pcg32( seed_source);
    markers[i].rng = pcg32( 0x853c49e6748fea9bULL, i);
    markers[i].id=i;
    markers[i].integral=0;
    markers[i].stopAt = nsteps + rand() / (RAND_MAX / (2*d_nsteps + 1) + 1) - d_nsteps;
    markers[i].location = -1;
    markers[i].team = -1;
    markers[i].thread = -1;
  }

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


//SYCL_EXTERNAL 
int stepMarker(mrk_t *marker, float *grid, int gridsize){
  //marker->location = (int) pcg32_boundedrand_r( &(marker->rng), gridsize);
  //marker->location = std::uniform_int_distribution
  marker->location   = (int)  marker->rng(gridsize);
  marker->integral += grid[marker->location];
  return 0;
}

class simple_test;

int main(int argc, char **argv){





  int nMarks, gridSize;
  int nsteps, d_nsteps;
  int i, imark, retval;
  bool cpu;

  nMarks = 1000000;
  gridSize = 500;
  nsteps = 10000;
  d_nsteps = 20;
  cpu = false;

  retval = parse_arguments( argc, argv, &gridSize, &nsteps, &d_nsteps, cpu);
  if (retval > 0){
    return retval;
  }


  mrk *markers = new mrk[nMarks];
  float *grid  = new float[gridSize];

  std::cout << "Markers:" << nMarks << " Steps:" << nsteps <<  "pm" << d_nsteps << " Gridsize " << gridSize << std::endl;


  initField(gridSize, grid);
  initMarkers(markers, nMarks, nsteps, d_nsteps);

  {  // SYCL SCOPE

    // accelerator job queue
    cl::sycl::queue queue;
    
  
    // selector to choose the accelerator
    if (cpu) { 
      cl::sycl::cpu_selector device_selector;
      queue = cl::sycl::queue(device_selector);
    } else {
      cl::sycl::default_selector device_selector;
      queue = cl::sycl::queue(device_selector);
    }



    std::cout   << "   prw_sycl running on "
                << queue.get_device().get_info<cl::sycl::info::device::name>()
                << std::endl;


    //                                dimensions=1
    cl::sycl::buffer<float, 1> grid_buff(grid, cl::sycl::range<1>(gridSize));
    cl::sycl::buffer<mrk, 1> markers_buff(markers, cl::sycl::range<1>(nMarks));
    
    
    queue.submit([&](sycl::handler& cgh)
    {

      // Transfer data
      auto grid_acc   =   grid_buff.get_access<cl::sycl::access::mode::read>(cgh);
      auto markers_acc=markers_buff.get_access<cl::sycl::access::mode::read_write>(cgh);
      //, mrk, 1,  cl::sycl::access::target::constant_buffer>();


      // device code here
      cgh.parallel_for<class simple_test>(sycl::range<1>(nMarks), [=](sycl::id<1> idx)
      {  
        int istep;
        mrk marker = markers_acc[idx[0]];
        float *grid_local = grid_acc.get_pointer();

        for(istep=0; istep < marker.stopAt; ++istep){

          stepMarker( &marker, grid_local, gridSize );

        }
      
        marker.integral /= (float) marker.stopAt;
        markers_acc[idx[0]] = marker;
      });
      
    });  // End of device code

  } // End of SYCL scope

  for(imark=0; imark<nMarks; imark+=19331){
    printMarker(markers[imark]);
  }

  

  std::cout   << "   prw_sycl finished. " << std::endl;

  return 0;
}