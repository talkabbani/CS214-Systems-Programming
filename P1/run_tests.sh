#!/bin/bash
# Script to run all tests for mymalloc/myfree implementation

echo "===== Running Validation Tests ====="
./validation_test
echo

echo "===== Running Error Detection Tests ====="
echo "Note: Each error test will cause program termination"

echo
echo "----- Test 1: Freeing non-malloc address -----"
./error_test 1
echo

echo "----- Test 2: Freeing offset pointer -----"
./error_test 2
echo

echo "----- Test 3: Double-free detection -----"
./error_test 3
echo

echo "===== Running memgrind Performance Tests ====="
./memgrind
echo

echo "All tests completed."