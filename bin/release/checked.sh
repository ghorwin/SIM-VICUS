#!/bin/bash

basename=$1
basename=${basename%.nandrad}
rm ${basename}_jacobian_dense.bin
rm ${basename}_jacobian_sparse.bin

touch ${basename}.jac_checked
