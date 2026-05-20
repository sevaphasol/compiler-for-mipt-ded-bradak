#!/bin/bash

RELDIR=examples/link_example

STDLIB_OBJ=build/splobj/libstd.splobj

FIRST_SRC=${RELDIR}/1st.lang
FIRST_FRONT=${RELDIR}/1st.front
FIRST_MID=${RELDIR}/1st.middle
FIRST_OBJ=${RELDIR}/1st.splobj

SECOND_SRC=${RELDIR}/2nd.lang
SECOND_FRONT=${RELDIR}/2nd.front
SECOND_MID=${RELDIR}/2nd.middle
SECOND_OBJ=${RELDIR}/2nd.splobj

MERGED_OBJ=${RELDIR}/1and2.splobj
MERGED_WITH_STDLIB_OUT=${RELDIR}/1and2_with_stdlib.splobj
MERGED_OUT=${RELDIR}/1and2.out

build/bin/frontend ${FIRST_SRC} ${FIRST_FRONT}
build/bin/midend -i ${FIRST_FRONT} -o ${FIRST_MID}
build/bin/backend --emit-obj -i ${FIRST_MID} -o ${FIRST_OBJ}

build/bin/frontend ${SECOND_SRC} ${SECOND_FRONT}
build/bin/midend -i ${SECOND_FRONT} -o ${SECOND_MID}
build/bin/backend --emit-obj -i ${SECOND_MID} -o ${SECOND_OBJ}

./build/bin/spl_link -o ${MERGED_OBJ} ${FIRST_OBJ} ${SECOND_OBJ}
./build/bin/spl_link -o ${MERGED_WITH_STDLIB_OUT} ${MERGED_OBJ} ${STDLIB_OBJ}
./build/bin/backend --convert-splobj -i ${MERGED_WITH_STDLIB_OUT} -o ${MERGED_OUT}
