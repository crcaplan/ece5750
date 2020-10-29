#!/bin/bash
#PBS -k eo
#PBS -m abe 
#PBS -M pac256@cornell.edu
#PBS -N group5_pa2
parr=(1 2 5 10 15 20 $n) 

for p in ${parr[*]};
do
  echo "Parallel Implementation, n=$n and p=$p"
  /home/pac256/ece5750/pa2/p-nqueens3 $n $p
  echo "Serial Implementation, n=$n"
  /home/pac256/ece5750/pa2/nqueens2 $n
  echo " "
done

