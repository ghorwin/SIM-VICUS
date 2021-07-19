#!/bin/bash

NandradSolverJacDump $1
NandradSolverJacDump $1 --les-solver=Dense

basename=$1
basename=${basename%.nandrad}
mv jacobian_dense_cvode.txt ${basename}_jacobian_dense.txt
mv jacobian_sparse.bin ${basename}_jacobian_sparse.bin

echo 'Created '${basename}_jacobian_dense.txt
echo 'Created '${basename}_jacobian_sparse.bin
