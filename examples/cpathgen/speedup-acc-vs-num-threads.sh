#!/bin/bash

# netcard
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/netcard_iccad.edges netcard.golden

# leon2
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/leon2_iccad.edges leon2.golden

# leon3mp
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/leon3mp_iccad.edges leon3mp.golden

# ldoor
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/ldoor.edges ldoor.golden

# cage15
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/cage15.edges cage15.golden

# audikw1
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/audikw1.edges audikw1.golden

# M6
./speedup-acc-vs-num-threads 1000000 ../../benchmarks/M6.edges M6.golden