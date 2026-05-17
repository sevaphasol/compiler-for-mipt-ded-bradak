#!/bin/bash
set -e

rm -f ./back-end/lib/stdlib.bin
rm -f ./local/test/test.out

pushd stdlib
echo "Running build_stdlib.sh"
./build_stdlib.sh   # output visible
mv stdlib.bin ../back-end/lib/stdlib.bin
popd

# make
make > /dev/null 2>&1

echo "Running compiler driver"
./run.sh ./local/test/test.lang ./local/test/test.out 
./local/test/test.out 

echo "build success"
