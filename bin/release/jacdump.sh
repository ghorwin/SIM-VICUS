#!/bin/bash

NandradSolverJacDump $1
NandradSolverJacDump $1 --les-solver=Dense --integrator=ImplicitEuler

basename=$1
basename=${basename%.nandrad}
mv jacobian_dense.bin ${basename}_jacobian_dense.bin
mv jacobian_sparse.bin ${basename}_jacobian_sparse.bin

echo 'Created '${basename}_jacobian_dense.bin
echo 'Created '${basename}_jacobian_sparse.bin
