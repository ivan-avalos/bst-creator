# Fakemile.sh - compile and link libgvc.

if [ -z $1 ]; then
	# iterate C files and compile them
	for file in *.c; do
		echo Compiling $file...
		gcc "$file" -lgvc -lcgraph -lcdt -o $(basename "$file" ".c")
	done
	exit
fi

# compile only specified file
echo Compiling $1...
gcc "$1" -lgvc -lcgraph -lcdt -o $(basename "$1" ".c");
