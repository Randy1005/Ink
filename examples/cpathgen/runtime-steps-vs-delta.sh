#!/bin/bash


# ldoor
./runtime-steps-vs-delta ../../benchmarks/ldoor.edges 1.0 0.5 5.0

# cage15
./runtime-steps-vs-delta ../../benchmarks/cage15.edges 2.0 2.0 10.0

# M6
./runtime-steps-vs-delta ../../benchmarks/M6.edges 0.1 0.1 1.0

# nlpkkt120
./runtime-steps-vs-delta ../../benchmarks/nlpkkt120.edges 0.1 0.1 1.0