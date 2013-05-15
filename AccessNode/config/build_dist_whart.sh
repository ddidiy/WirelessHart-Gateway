#!/bin/bash

link=dynamic
link_stack=static
b_no=""
procs=""

CVS_SERVER="10.32.0.12:/home/cvs"
SVN_SERVER="https://cljsrv01.nivis.com:8443/svn"

declare -a architectures
architectures=(vr900 i386 pc)

declare -a source_directories
source_directories=(AccessNode AuxLibs Gateway HostApplication Stack NetworkManager NetworkEngine cpplib MCSWebsite)

_tabcomplete_build_whart_ ()	# register tab completion
{
	WORD="${COMP_WORDS[COMP_CWORD]}"
	case $COMP_CWORD in
		( 1 )
		FIRSTPARAM="update checkout clean hw= help"
		COMPREPLY=($( compgen -W "$FIRSTPARAM" -- $WORD))
		;;
		( 2|3 )
		# hw=arch
		if [ $COMP_CWORD -eq 2 ]; then WORD=""; fi
		if [ "hw" = "${COMP_WORDS[1]}" -a "=" = "${COMP_WORDS[2]}" ]; then
			PARAM=${architectures[@]}
			COMPREPLY=($( compgen -W "$PARAM" -- $WORD))
		fi
		# clean all
		if [ "clean" = "${COMP_WORDS[1]}" ]; then
			PARAM=(all)
			COMPREPLY=($( compgen -W "$PARAM" -- $WORD))
		fi
		;;
		( 4 )
		mm="0"; for i in ${architectures}; do if [ "$i" = "${COMP_WORDS[3]}" ]; then mm="1"; fi;
		done
		if [ "${COMP_WORDS[1]}" = "hw" -a "${COMP_WORDS[2]}" = "=" -a "${mm}" = "1" ]; then
			COMPREPLY=($( compgen -W "silent fast" -- $WORD))
		fi
		;;
		( 5 )
		mm="0"; for i in ${architectures}; do if [ "$i" = "${COMP_WORDS[3]}" ]; then mm="1"; fi;
		done
		if [ "${COMP_WORDS[1]}" = "hw" -a "${COMP_WORDS[2]}" = "=" -a "${mm}" = "1" ]; then
			PARM="fast silent"
			if [ "${COMP_WORDS[4]}" = "silent" ]; then
				PARM="fast";
			fi
			if [ "${COMP_WORDS[4]}" = "fast" ]; then
				PARM="silent";
			fi
			COMPREPLY=($( compgen -W $PARM -- $WORD))
		fi
		;;
	esac
}

check_sourced_forked()			# when sourced, install tab completion
{
	# when sourced _only_ from the pwd of this script, install tab key
	if [ "x" = "x$(expr match "$0" ".*build_dist_whart.s\(h\)")" ] ; then
		complete -F _tabcomplete_build_whart_ "./build_dist_whart.sh"
		echo "I installed programmable tab completion"
		return 0
	fi
	return 1
}

_update_()
{
	for i in ${source_directories[@]}; do
		update_source $i
	done
}

#$1: folder to update
update_source()
{
	if [ ! -d $1 ]; then echo $1 does not exist. cannot update; return 1; fi
	echo "Update: $1"
	pushd $1 > /dev/null
	[ -d CVS  ] && cvs -q update -dP
	[ -d .svn ] && svn update 
	popd > /dev/null
}

#$1: folder to check-out
#$2: cvs user name
#$3: cvs branch for AccessNode
checkout_source()
{
	local svn_path=""
	echo "Checkout [$1]"
	case $1 in
		( AccessNode )	cvs -q -d:pserver:${2}@${CVS_SERVER} co -r ${3} $1; return $? ;;	#pushd AccessNode; cvs -q up -r ${3}; popd;
	    	( AuxLibs )		cvs -q -d:pserver:${2}@${CVS_SERVER} co $1; return $? ;;
		( cpplib )		svn_path="/$1/" ;;
		( Gateway|HostApplication|NetworkEngine|NetworkManager|Stack|MCSWebsite )
						svn_path="/svnroot/branches/opensource-developer-wh-1.5/$1" ;;
		( * ) echo "Unknown module [$1]"; return 1 ;;
	esac
	[ -n "$svn_path" ] && svn checkout ${SVN_SERVER}/$svn_path
}

check_symlink()					# only before a compilation
{
	# it is called only before a compilation

# this file should be a slink to the script from AccessNode/config
# directory, to be sure it is up to date. When not sourced, display a
# warning if this executable is not a slink, or check it is in
# repository
	if [ -h $0 ]; then
		if [ "AccessNode/config/build_dist_whart.sh" != "`readlink ./build_dist_whart.sh`" ]; then
			echo $0 should be a symlink to "AccessNode/config/build_dist_whart.sh"
		fi
	else
		if [ "$(echo $(pwd) | sed "s:/AccessNode/config:X:")" = "$(echo $(pwd))" ]; then
			echo $0 is not a slink and not called from AccessNode repo
		fi
	fi
}

init()						    # set the ACTION, using the 1st parameter
{
	case $1 in
		( update|checkout|clean )
		ACTION=$1
		;;
		( * )
		if [ "hw=" = "${1:0:3}" ] ; then
			case "${1:3}" in
				( i386|vr900|pc )
				ARCH="${1:3}"
				ACTION=compile
				case "$2$3" in
					( "fast" )
					fast="yes"
					silent=""
					;;
					( "silent" )
					fast="no"
					silent=-s;
					;;
					( "silentfast"|"fastsilent" )
					silent=-s;
					fast="yes"
					;;
					( * )
					fast="no"
					silent=""
					;;
				esac
				;;
				( * )
				_help_
				exit 1
			esac
		else
			_help_
			exit 1
		fi
		;;
	esac
}

check()							# all useful directories exist
{
	check_sources()
	{
		echo -n Check if the directory $1 exists:
		if [ -d $1 ]; then
			echo OK
			return 0
		else
			echo missing.
			return 1
		fi
	}

	local error
	error=0
	for i in ${source_directories[@]}; do
		check_sources $i
		if [ $? -ne 0 ]; then
			error=1;
		fi
	done
	return $error
}

_help_()
{
	if [ "xterm" = "$TERM" ]; then

		local black red green yellow blue magenta cyan white R
		local p1					# color of the first param
		local p2					# color of the second param
		local p3					# color of the third parameter
		local cc					# color of comments
		local R						# reset color
		local color_parm

		black="\E[30;1m"
		red="\E[31;1m"
		green="\E[32;1m"
		yellow="\E[33;1m"
		blue="\E[34;1m"
		magenta="\E[35;1m"
		cyan="\E[36;1m"
		white="\E[37;1m"
		R="\E[0;0m"					# reset color

		p1=$red
		p2=$magenta
		p3=$cyan
		cc=$black
		O=-e

		color_parm="${p1}first$R ${p2}second$R ${p3}third$R parameter"
	fi
	
	wrap="$(echo "`basename $0`" | sed 's,.,.,g')"
	command_line="`basename $0` [ ${p1}hw$R=${p2}<arch>$R ${p3}[compile-flags]$R | ${p1}update$R | ${p1}checkout$R ${p2}[<cvs_user> [<cvs_branch>]]$R | ${p1}clean$R (not implemented yet) | ${p1}help$R  ]"
	
	echo $O $command_line
	echo $O $wrap "${p2}arch$R: < vr900 | i386 | pc >"
	echo $O $wrap "${p2}cvs_user$R: cvs user"
	echo $O $wrap "${p2}cvs_branch$R: cvs branch to use for AccessNode"
	echo $O $wrap "${p3}compile-flags$R: <silent| fast>*"
	echo $O ${cc}source the file to install programmable tab completion$R
	
	if [ -n $O ] ; then tput sgr0; echo $O $color_parm; fi # reset colors
}

_checkout_()
{
	local start_sec end_sec cvs_user cvs_branch
	start_sec=`date +%s`
	cvs_branch=${2:-"opensource-developer-wh-1_5_6_g"}
	cvs_user=$1
	if [ -z $1 ]; then
		echo "Enter your CVS username"
		read cvs_user
	fi
	[ -z $cvs_user ] && exit 1	#must have a valid CVS used
	cvs -q -d:pserver:${cvs_user}@${CVS_SERVER} login	# do not check return code; maybe user is already logged
	for i in ${source_directories[@]}; do
		checkout_source $i ${cvs_user} ${cvs_branch} || { echo "Checkout failed"; return 1; }
    done
	[ ! -L boost ] && ln -sf cpplib/trunk/boost_1_36_0 boost
	end_sec=`date +%s`
	echo ""
	echo "AccessNode using tag/branch [$cvs_branch]"
	echo "Install OK in $((end_sec-start_sec)) seconds"
}

_clean_()
{
	local source_directories_for_clean
	declare -a source_directories_for_clean
	source_directories_for_clean=${source_directories[@]}

	if [ "all" = "$1" ]; then
		:
	else
		if [ -z $1 ] ; then
			source_directories_for_clean=(${source_directories_for_clean[@]/cpplib})
			source_directories_for_clean=(${source_directories_for_clean[@]/AuxLibs})
		else
			echo "invalid parameter for clean" && _help_ && exit 1
		fi
	fi

	echo will clean: ${source_directories_for_clean[@]}

	for i in ${source_directories_for_clean[@]}; do
		[ ! -d $i ] && echo "Skip $i" && continue
		pushd $i
		make clean
		popd
	done
}

_compile_()
{
    if [ -n "$DEBUG" ]; then
	    ODIR=debug
	else
	    ODIR=release
	fi

	case ${ARCH} in
		( vr900 )
		TOOLCHAIN=m68k-unknown-linux-uclibc
		host=m68k-unknown-linux-uclibc
		NLIB_OS_PATH=cpplib/trunk/nlib/lib/linux-m68k/libnlib-socket-gcc44-mt-0_3_1.so  
		NM_OS_PATH=NetworkManager/out/release/linux-m68k/NetworkManager/WHart_NM.o
		GW_OS_PATH=Gateway/out/release/linux-m68k/Gateway/WHart_GW.o
		HA_OS_PATH=HostApplication/out/release/linux-m68k/Host/MonitorHost
		;;
		( i386 | pc )
		TOOLCHAIN=gcc-linux-pc
		host=i386
		NLIB_OS_PATH=cpplib/trunk/nlib/lib/linux-pc/libnlib-socket-gcc44-mt-0_3_1.so  
		NM_OS_PATH=NetworkManager/out/release/linux-pc/NetworkManager/WHart_NM.o
		GW_OS_PATH=Gateway/out/release/linux-pc/Gateway/WHart_GW.o
		HA_OS_PATH=HostApplication/out/release/linux-pc/Host/MonitorHost
		fstree_arg="fs_tree=an"
		;;
 		( * )
		echo "hw=${ARCH} not supported"
		exit 1
		;;
	esac

	check_symlink
	check
	
	if [ $? -ne 0 ]; then
		echo must checkout
		exit 1
	fi

	if [ ! -e ~/default_home.mk ]; then
		echo ~/default_home.mk does not exist. Create a default. compile again.
#++++@			WHART_STACK_DIR=XXX/Stack
#++++@			AN_SRC=../AccessNode
#++++@			CPPLIB_PATH=../cpplib
		grep "^#++++@" $0  | sed -e 's:^#++++@[[:blank:]]*\(.*\):\1:' -e "s:XXX:$(pwd):" > ~/default_home.mk
		exit 0
	else
		(
			. ~/default_home.mk
			[ "xk" = "x${WHART_STACK_DIR#$(pwd)/Stac}" ] && exit 0
			exit 1
		)
		[ "1" = "$?" ] &&
		echo "removed ~/default_home.mk. old directory differs." &&
		rm ~/default_home.mk &&
		exit 1
	fi
	

	ln -sf cpplib/trunk/boost_1_36_0 boost

	pushd AccessNode/Shared
#	make ${silent} release=whart link=${link} hw=${ARCH} clean_module_exe
#	make ${silent} release=whart link=${link} hw=${ARCH} ${c_action} cfast=$c_fast $fstree_arg
    make ${silent} release=whart link=dynamic hw=${ARCH} clean_module_exe
    make ${silent} release=whart link=static hw=${ARCH} clean_module_exe
    make ${silent} release=whart link_shared=dynamic link=${link} hw=${ARCH} ${c_action} cfast=$c_fast $fstree_arg
    make ${silent} release=whart                     link=${link} hw=${ARCH} ${c_action} cfast=$c_fast $fstree_arg
	popd

	pushd AccessNode/Shared/SqliteUtil/
	make ${silent} release=whart link=dynamic hw=${ARCH} clean_module_exe
	if [  "${c_action}" = "clean" ]; then
		make ${silent} release=whart link=dynamic hw=${ARCH} ${c_action} cfast=$c_fast 
	else
		make ${silent} release=whart link=dynamic hw=${ARCH} compile exe_copy exe_strip cfast=$c_fast $fstree_arg
	fi	
	popd

	MAKE="make ${silent} hw=${ARCH} link_shared=${link} link_stack=${link_stack} ${c_action} $procs $fstree_arg"
	( cd cpplib/trunk/boost_1_36_0 && ${MAKE} )
	( cd cpplib/trunk/nlib && ${MAKE} )
	( cd cpplib/trunk/log4cplus_1_0_2 && ${MAKE} )
    ( cd NetworkEngine && ${MAKE} )

	pushd NetworkEngine
	${MAKE}
	[ $? -ne 0 ] && popd && exit 1
	popd

	pushd Stack
	find out -name  WHart_Stack -type f -print | xargs /bin/rm -f
	${MAKE}
	[ $? -ne 0 ] && popd && exit 1
	popd

	pushd NetworkManager
	find out -name  WHart_NM.o -type f -print | xargs /bin/rm -f
	echo ${MAKE}
	${MAKE}
	[ $? -ne 0 ] && popd && exit 1
	popd

	pushd Gateway
	find out -name  WHart_GW.o -type f -print | xargs /bin/rm -f
	find out -name  MainApp.o -type f -print | xargs /bin/rm -f
	${MAKE} 
	[ $? -ne 0 ] && popd && exit 1
	popd

	pushd HostApplication
	find out -name  MonitorHost -type f -print | xargs /bin/rm -f
	find out -name  MainApp.o -type f -print | xargs /bin/rm -f
	${MAKE} 
	[ $? -ne 0 ] && popd && exit 1
	popd

##  need to create a distribution for i386 either
###	[ "${ARCH}" = "i386" -o "${ARCH}" = "pc" ] && exit 0

	mkdir -p AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/lib/

	cp -f AccessNode/an/lib/libsqlitexx.so AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/lib/
	rm -f  AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/Monitor_Host.db3.fixture
	rm -f  AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/Monitor_Host.db3

	sqlite3  AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/Monitor_Host.db3.fixture < HostApplication/db/schema/sqlite/CreateSchema.sql
	sqlite3  AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/Monitor_Host.db3.fixture < HostApplication/db/schema/sqlite/CreateDefaultData.sql


	echo "cp -f $NM_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/"
	cp -f $NM_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/
	echo "cp -f $GW_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/"
	cp -f $GW_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/

	echo "cp -f $HA_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/"
	cp -f $HA_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/

	echo "cp -f $NLIB_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/lib/"
	cp -f $NLIB_OS_PATH AccessNode/config/FW_mesh_HW_${ARCH}/release_whart/whart_aliens/lib/

	pushd AccessNode
	make ${silent} cfast=$c_fast release=whart link=${link} hw=${ARCH} b_no=${b_no} $fstree_arg
	[ $? -ne 0 ] && popd && exit 1
	popd

}

{
	# when sourced, install the completion and quit
	check_sourced_forked $0 && return 0

# calling with no parameter, this calls help
	if [ "$#" -eq "0" ]; then
		_help_
		exit 0
	fi

	init $@

	case $ACTION in
		( update|checkout|clean )
			shift
			_${ACTION}_ $@
		;;
		( compile|help )
		echo $ACTION
		_${ACTION}_
		;;
	esac
}




