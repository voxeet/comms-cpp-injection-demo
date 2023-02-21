#!/bin/bash

echo "If you want your own version and not default you must run script like:"
echo "bash setup.sh {sdk_version}" 

DEFAULT_SDK_VERSION="2.3.1"
WORK_DIR=$(pwd)

set_proper_sdk_version() {
	sdk_version=$1
	if [ -z $sdk_version ]; then
		sdk_version=${DEFAULT_SDK_VERSION}
	fi
}

should_fetch_new_sdk() {
	if [ -f sdk/version.txt ]; then
		current_version=$(cat sdk/version.txt)
	else
		current_version="none"
	fi

	if [ "$current_version" != "$sdk_version" ]; then
		# If we want to use a different package clear the old one
		if [ "$current_version" != "none" ]; then
			rm -rf sdk/
		fi

		true
		return
	else
		false
		return
	fi
}

fetch_sdk() {
	platform=$(uname)
	if [ $platform == "Darwin" ]; then
		system="macos"
	elif [ $platform == "Linux" ]; then
		system="linux"
	else
		system="windows"
	fi

	curl https://github.com/DolbyIO/comms-sdk-cpp/releases/download/${sdk_version}/cppsdk-${sdk_version}-${system}64.zip -O -J -L
}

unzip_sdk_package() {
	if [ ! -f cppsdk-${sdk_version}-${system}64.zip ]; then
		echo "There is not sdk zip package here!"
		exit 1
	fi

	package_name="sdk-release"
	mkdir sdk/
	unzip cppsdk-${sdk_version}-${system}64.zip
	if [ $system == "macos" ]; then
		if [ $(uname -m) == "arm64" ]; then
			package_name="sdk-release-arm"
		else
			package_name="sdk-release-x86"
		fi
	fi
	cp -r $package_name/*  sdk/

	rm -rf sdk-release*/
	rm *.zip

	echo ${sdk_version} > sdk/version.txt
}

build_cpp_injection_demo() {
	popd
	if [ ! -d $WORK_DIR/build/ ]; then
		mkdir $WORK_DIR/build
	fi

	cd $WORK_DIR/build/

	cmake ../
	cmake --build . -j 16
	cmake --install .
}

# Set the version of the SDK to use, default will be set if no arg
# passed in
set_proper_sdk_version

# Enter ext-lib to place all the dependencies
if [ ! -d $WORK_DIR/ext-lib ]; then
	mkdir $WORK_DIR/ext-lib/
fi
pushd $WORK_DIR/ext-lib/

# Fetch SDK package if hashes do not match
if should_fetch_new_sdk; then
	# Fetch the sdk-package and unzip it 
	fetch_sdk
	unzip_sdk_package
else
	echo "Not fetching new SDK packages as hashes match"
fi

# Build the injetion binary in cpp-injection-demo/build/ directory
build_cpp_injection_demo
