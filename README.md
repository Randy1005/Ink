![example](https://github.com/Randy1005/QSat/actions/workflows/cmake.yml/badge.svg)
# QSat

## Features
+ CDCL SAT solver
+ activity-based variable selection 

## Current Progress
- [x] basic solver functionaliy (boolean constraint propagation + conflict analysis)
- [x] regression / integration test for comparing performance with minisat
- [x] geometric solver restart strategy

## TODO
- [ ] modify SYCL to CUDA
  - [ ] cuda memory pool
  - [ ] configure kernels with num_blocks that fit each problem (instead of fixed blocks)

