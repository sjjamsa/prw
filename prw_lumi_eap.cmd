#!/bin/bash

#SBATCH --partition=eap
#SBATCH --account=project_462000079


#SBATCH --time=10:00


#SBATCH --ntasks=1


#SBATCH --cpus-per-task=1


#SBATCH --gpus-per-task=1



export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK #


export MPICH_GPU_SUPPORT_ENABLED=1 #


date
time srun ./parallel_random_walk 1024000   320000 0
date
