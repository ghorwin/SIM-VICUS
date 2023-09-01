#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./SIM-VICUS  > valgrind_log.txt

