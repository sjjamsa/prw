# prw -- "parallel random walk"
mini-app for testing openMP offloading to GPUs

The ultimate goal of this mini-app is to have a small test-case for [ASCOT5-gpu](https://wiki.aalto.fi/display/ASCOTCo/Parallelization+and+offloading) testing.

The code basically does a 1D-montecarlo integral in parallel. For each timestep it does a random number generation and a random memory access.
