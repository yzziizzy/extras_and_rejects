#!/bin/bash


function yes_no() {
	while true; do
		read -p "$* [y/n]: " yn
		case $yn in
			[Yy]*) return 0 ;;  
			[Nn]*) return 1 ;;
		esac
	done
}


# there is a better version of this somewhere 
here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
echo "we are here <$here>"

sed_files=( _build.c _build.inc.c build.sh debug.sh profiling.sh valgrind.sh valgrind valgrind_noleaks.sh valgrind_nosuppress.sh )
code_file="main.c"

# name of the "main.c" file to be auto-generated
dest_name_code="main.c"

# the executable file
dest_name_exe=""

# build dir
dest_path_build="build"

# where the executable file should be generated at
dest_path_exe="."

# root directory for the whole project
dest_path_root=""

# relative path of the source directory within the root
dest_path_source="src"

arg_project_name=""
arg_root_dir=""
arg_source_dir=""
arg_build_dir=""
arg_source_file_name="main.c"
arg_exe_file_path=""


while getopts ":b:c:e:r:s:x:" opt; do
	case $opt in
		p)
			arg_project_name="$OPTARG"
			;;
		b)
			arg_build_dir="$OPTARG"
			;;
		r)
			arg_path_dir="$OPTARG"
			;;
		S)
			arg_source_dir="$OPTARG"
			;;
		s)
			arg_source_file_name="$OPTARG"
			;;
		x)
			arg_exe_file_path="$OPTARG"
			;;
		\?)
			echo "Invalid option: -$OPTARG"
			exit 1
			;;
		:)
			echo "Option -$OPTARG requires an argument"
			exit 1
			;;
	esac
done

shift $((OPTIND-1))


read -e -p "Enter project name: " -i "$arg_project_name"  project_name


if [ "$arg_root_dir" = "" ] ; then
	root_dir="./$project_name"
fi
read -e -p "Enter target directory: " -i "$root_dir" root_dir


if [ "$arg_source_dir" = "" ] ; then
	source_dir="$root_dir/src"
fi
read -e -p "Enter source directory: " -i "$source_dir" source_dir


if [ "$arg_build_dir" = "" ] ; then
	build_dir="$root_dir/build"
fi
read -e -p "Enter build directory: " -i "$build_dir" build_dir


if [ "$arg_source_file_path" = "" ] ; then
	arg_source_file_path="$source_dir/main.c"
fi
read -e -p "Enter source file name: " -i "$arg_source_file_path"  source_file_path


if [ "$arg_exe_file_path" = "" ] ; then
	arg_exe_file_path="$root_dir/$project_name"
fi
read -e -p "Enter executable name: " -i "$arg_exe_file_path"  exe_file_path


# TODO: vulkan support optional

echo ""

echo "Creating project '$project_name':"
echo "  Root directory:   $root_dir"
echo "  Source directory: $source_dir"
echo "  Source File:      $source_file_path"
echo "  Build directory:  $build_dir"
echo "  Executable:       $exe_file_path"

if ! yes_no "Continue?" ; then
	echo "Aborted"
	exit 1
fi



mkdir -p "$root_dir"
mkdir -p "$source_dir"
mkdir -p "$build_dir"
mkdir -p `basename "$exe_file_path"`

cp "$here/scripts/$code_file" "$source_file_path"

for f in "${sed_files[@]}"; do
	cp "$here/scripts/$f" "$root_dir/$f"
done


exe_name=`basename "$exe_file_path"`
exe_dir=`dirname "$exe_file_path"`
exe_rel_dir=`realpath --relative-to="$root_dir" "$exe_dir" `
exe_rel_path="$exe_rel_dir/$exe_name"
build_rel_dir=`realpath --relative-to="$root_dir" "$build_dir" `
source_rel_dir=`realpath --relative-to="$root_dir" "$source_dir" `
source_rel_path=`realpath --relative-to="$root_dir" "$source_file_path" `
source_file_name=`basename "$source_file_path" `

#echo "exe_rel_path = $exe_rel_path"
#echo "build_rel_dir = $build_rel_dir"
#echo "source_rel_dir = $source_rel_dir"

cd $root_dir
#pwd
sed -i -E -e "s;__SED_TOKEN_EXE_NAME;$exe_file_path;" "${sed_files[@]}" "$source_rel_path"
sed -i -E -e "s;__SED_TOKEN_EXE_REL_PATH;$exe_rel_path;" "${sed_files[@]}" "$source_rel_path"
sed -i -E -e "s;__SED_TOKEN_BUILD_REL_PATH;$build_rel_dir;" "${sed_files[@]}" "$source_rel_path"
sed -i -E -e "s;__SED_TOKEN_SOURCE_FILE_NAME;$source_file_name;" "${sed_files[@]}" "$source_rel_path"
sed -i -E -e "s;__SED_TOKEN_SOURCE_REL_PATH;$source_rel_dir;" "${sed_files[@]}" "$source_rel_path"


echo "._build" >> ".gitignore"
echo "$exe_file_path" >> ".gitignore"
echo "$build_rel_dir" >>  ".gitignore"



