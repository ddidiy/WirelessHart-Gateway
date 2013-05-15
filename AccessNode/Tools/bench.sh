#! /bin/sh -e

# usage bench.sh test_file flashfs tmpfs

bench() {
        echo "****************"
        echo "$1 -> $2"
        eval "/usr/bin/time $2" 2>&1 | grep sys | cut -f 2
        echo "****************"
        echo ""
        if [ "$?" != "0" ] ; then
                echo "$2 failed. Exiting"
                exit
        fi
}
fork_stress() {
        echo "Generating stress script"
        i=0
        while [ $i -lt 100 ]; do
                echo "echo exit | sh & " >> /tmp/fork_stress.sh
                i=`expr $i + 1`
        done
        echo "Running stress script"
        chmod +x /tmp/fork_stress.sh
        /tmp/fork_stress.sh
        rm -rf /tmp/fork_stress.sh
}


if [ $# -lt 3 ]; then
        echo "Usage: bench.sh  test_file  flashfs  tmpfs"
        exit
fi

test_file_path=$1
test_file="`basename ${test_file_path}`"
flashfs=$2
tmpfs=$3

cp ${test_file_path} ${tmpfs}
if [ "$?" != "0" ]; then
        echo "Unable to copy ${test_file_path} -> ${tmpfs}"
        exit 0
fi
cd ${tmpfs}

sync; echo 3 > /proc/sys/vm/drop_caches

bench "Z->M" "dd if=/dev/zero of=${tmpfs}/zero bs=1M count=1"
rm -rf ${tmpfs}/zero

bench "M->N" "cat ${tmpfs}/${test_file} > /dev/null"
bench "M->M" "cp  ${tmpfs}/${test_file}   ${tmpfs}/${test_file}2"
bench "M->F" "cp  ${tmpfs}/${test_file}   ${flashfs}/${test_file}"
rm -rf ${tmpfs}/${test_file} ${tmpfs}/${test_file}2

bench "F->N" "cat ${flashfs}/${test_file} > /dev/null"
bench "F->F" "cp  ${flashfs}/${test_file}   ${flashfs}/${test_file}2"
bench "F->M" "cp  ${flashfs}/${test_file}   ${tmpfs}/${test_file}"
rm -rf ${flashfs}/${test_file} ${flashfs}/${test_file}2 ${tmpfs}/${test_file}
cd -

#fork_stress
