# benchmarks
declare -a benchmarks=("../benchmarks/leon2_iccad.edges" "../benchmarks/leon3mp_iccad.edges" "../benchmarks/netcard_iccad.edges")
runs=10
rm *out
mv report.csv report_old.csv

for b in "${benchmarks[@]}"
do
  for k in 1000000 2000000 3000000
  do
    for qs in 40 80 120 160
    do
      for ws in 4 8 12 16
      do
        for bs in 1 2 4
        do
          for node_distr in 0 1
          do 
            for policy in 0 1
            do
              # echo "command: $b $k $qs $ws $bs $node_distr $policy"
              echo "benchmark=$b"
              echo "num paths=$k"
              echo "num queues=$qs"
              echo "num workers=$ws"
              echo "bulk size=$bs"
              echo "enable node redistr=$node_distr"
              echo "partition policy=$policy"
              ./parallel $b out $k $qs $ws $bs $node_distr $policy 1.0 $runs 1   
            done
          done
        done
      done
    done
  done 


done


