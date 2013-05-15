#!/bin/bash

#expects to be run from folder AccessNode (an_src base)
# looking for config/ folder FW_${DIST}_HW_${HW}
#	if is not found try FW_${DIST}_HW_any
#	Let this dir be called FW_HW_DIR

#	if a RELEASE is requested, all files that exist in ${FW_HW_DIR}/release_${RELEASE} will be copy over main ${FW_HW_DIR} files



DIST=$1
HW=${2:-"any"}
RELEASE=$3
EXCLUDE_MODULES=${4:-""}



AN_SRC_DIR=`pwd`
AN_CONFIG_DIR=${AN_CONFIG_TOP_DIR:-$AN_SRC_DIR"/config"}/
AN_BIN_DIR=${AN_DIR:-$AN_SRC_DIR"/an"}/


AN_VERSION=`cat ${AN_BIN_DIR}version`
BUILD_DIR=an_bin_${AN_VERSION}_lg



usage()
{
	echo ""
	echo "Usage: $0 <dist> [hw] [release] [dist_alias]"
	echo "  Knows <dist>: "
	echo "			mesh"
	echo ""
	echo "		<hw>: "
	echo "			vr900"
	echo "			i386"
	echo ""
	echo "		<release>: "
	echo "			isa"
	echo "			whart"
	echo ""

}

#$1 -- hw
get_hw_no()
{
	case $1 in
		vr900 ) echo 7 ;;
		*) echo 0 ;;
	esac
}

echo ""
echo "********************************************************"
echo "*"
echo "*  Building for dist=$DIST hw=$HW release=$RELEASE EXCLUDE_MODULES=$EXCLUDE_MODULES"
echo "*"
echo "********************************************************"
echo ""

echo "Build Dir = ${BUILD_DIR} AN_CONFIG_DIR=$AN_CONFIG_DIR AN_BIN_DIR=$AN_BIN_DIR AN_DIR=$AN_DIR"
#exit

# $1 - release folder
# $2 - build folder
apply_release_folder()
{
	local dir_release=$1
	local dir_build=$2
	# if a specific RELEASE is requested, folders and files for RELEASE folder should be copy in dist folder
	# 	release files overwrite common files

	echo "apply_release_folder: $dir_release"

	[ ! -d $dir_release ] && return 1

	if [ -f  ${dir_release}/profile_templ/config.ini  ]; then
		[ ! -d ${dir_build}/profile_templ/ ] && mkdir ${dir_build}/profile_templ/
		mv -f ${dir_release}/profile_templ/config.ini ${dir_build}/profile_templ/config.ini
	fi

	if [ -f  ${dir_release}/profile_templ/config.ini_up_set ]; then
		echo "config.ini_up_set"
		${AN_CONFIG_DIR}/ini_update --command=set -s ${dir_release}/profile_templ/config.ini_up_set -d ${dir_build}/profile_templ/config.ini > /dev/null
		rm ${dir_release}/profile_templ/config.ini_up_set
	fi

	echo "cp -f -R ${dir_release}/* ${dir_build}"
	cp -f -R ${dir_release}/* ${dir_build}
	rm -fR ${dir_release}/
}

#erase previous dist
rm -f ${AN_BIN_DIR}an_*.tgz


if [ -d ${AN_BIN_DIR}${BUILD_DIR} ]; then
	rm -f -R ${AN_BIN_DIR}${BUILD_DIR}
fi



mkdir -p ${AN_BIN_DIR}${BUILD_DIR}

FW_HW_DIR="FW_${DIST}_HW_any/"

echo "Config Dir base = ${FW_HW_DIR}"
if [ -d ${AN_CONFIG_DIR}${FW_HW_DIR} ]; then
	echo "Copy base folder "${FW_HW_DIR}
	cp -f -R ${AN_CONFIG_DIR}${FW_HW_DIR}* ${AN_BIN_DIR}${BUILD_DIR}
fi


if [ ! $HW = "any" ]; then

	FW_HW_DIR="FW_${DIST}_HW_${HW}/"
	echo "Config Dir main = ${FW_HW_DIR}"
	if [ -d ${AN_CONFIG_DIR}${FW_HW_DIR} ]; then
		echo "Copy main folder "${FW_HW_DIR}
		cp -f -R ${AN_CONFIG_DIR}${FW_HW_DIR}* ${AN_BIN_DIR}${BUILD_DIR}
	fi
fi

if [ `ls -1 | wc -l` -eq 0 ]; then
	echo "Failed to find base or main config folder "
	exit 1
fi

#copy binaries and shared libs; copy only shared libs (.so), do not copy static libs (.a)
cp -f ${AN_BIN_DIR}* ${AN_BIN_DIR}${BUILD_DIR}/
cp -f -R ${AN_BIN_DIR}"/lib" ${AN_BIN_DIR}${BUILD_DIR}
rm -f ${AN_BIN_DIR}${BUILD_DIR}/lib/lib*.a
[ -d ${AN_BIN_DIR}"/www" ] && cp -f -R ${AN_BIN_DIR}"/www" ${AN_BIN_DIR}${BUILD_DIR}

apply_release_folder ${AN_BIN_DIR}${BUILD_DIR}/release_${RELEASE} ${AN_BIN_DIR}${BUILD_DIR}

echo "$EXCLUDE_MODULES" | grep -q -v SysManager
[ $? -eq 0 ] &&	apply_release_folder ${AN_BIN_DIR}${BUILD_DIR}/SysManager ${AN_BIN_DIR}${BUILD_DIR}

apply_release_folder ${AN_BIN_DIR}${BUILD_DIR}/whart_aliens ${AN_BIN_DIR}${BUILD_DIR}

# erase folder for other configurations
rm -f -R ${AN_BIN_DIR}${BUILD_DIR}/release_*
#rm -f -R ${AN_BIN_DIR}${BUILD_DIR}/SysManager
#rm -f -R ${AN_BIN_DIR}${BUILD_DIR}/whart_aliens

echo "dist=$DIST" > ${AN_BIN_DIR}${BUILD_DIR}/build_info
echo "dist_date=\"`date \"+%Y/%m/%d %T\"`\"" >> ${AN_BIN_DIR}${BUILD_DIR}/build_info
echo "dist_date_raw=`date +%s`" >> ${AN_BIN_DIR}${BUILD_DIR}/build_info
echo "hw=$HW" >> ${AN_BIN_DIR}${BUILD_DIR}/build_info
echo "hw_no=`get_hw_no $HW`" >> ${AN_BIN_DIR}${BUILD_DIR}/build_info
echo "release=$RELEASE" >> ${AN_BIN_DIR}${BUILD_DIR}/build_info

#erase CVS folders
find ${AN_BIN_DIR}${BUILD_DIR} -name CVS -type d -print | xargs /bin/rm -f -r

#make shure that all .sh have execution rigths
find ${AN_BIN_DIR}${BUILD_DIR} -name "*.sh" -type f -print | xargs chmod a+x 

#build archive
cd ${AN_BIN_DIR}

#MCS Appliance has special needs because it is not an AccessNode
if [ "$DIST" = "mesh" -a "$HW" = "i386" ]; then
	cp $AN_SRC_DIR/../AuxLibs/lib/i386/libaxtls.so ${AN_BIN_DIR}${BUILD_DIR}/lib
	ln -s libaxtls.so ${AN_BIN_DIR}${BUILD_DIR}/lib/libaxtls.so.1
fi

tar czf "${BUILD_DIR}.tgz" ${BUILD_DIR}
rm -f ER_FW.tgz
ln -s ${BUILD_DIR}.tgz ER_FW.tgz

echo ""
ls -lh ${BUILD_DIR}.tgz
echo ""
echo "distribution an/${BUILD_DIR}.tgz is done"

exit 0







