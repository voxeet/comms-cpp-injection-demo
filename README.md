[![CodeQL for C++](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-cpp.yml/badge.svg)](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-cpp.yml)[![CodeQL for Python](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-python.yml/badge.svg)](https://github.com/dolbyio-samples/comms-cpp-injection-demo/actions/workflows/codeql-analysis-python.yml)

# Communications API C++ SDK Media Injection Demo

## Prerequisites
All Sytems:
- CMake 3.23
- Python 3
- A Dolby.io account and access to the Dolby.io dashboard

Windows Only:
- msys2
- unzip (inside msys2 run "pacman -S unzip")

## Supported platforms
This project relies on the Communications API C++ SDK, see this [link](https://api-references.dolby.io/comms-sdk-cpp/other/supported_platforms.html) for supported platforms.

## Building
If building on Windows make sure you have installed msys2 shell and unzip inside of it.

```
1. git clone git@github.com:dolbyio-samples/comms-cpp-injection-demo.git 
2. cd cpp-injection-demo
3. git checkout main 
```
Building on MacOS or Linux:
```
4. bash setup.sh
```
Building on Windows:
```
4. setup.bat
```

## Getting Started
The `demo.py` script will scan the `injection-input.json` file to determine which conversations it will inject into the conference. Based on the content of the `injection-input.json`, the `demo.py` script will take conversations in m4a or mp4 format from the respective folder and inject them at locations/rotations specified in the `def.json` files in the conversation folders.

The environment set by the cpp application is as follows:
```cpp
dolbyio::comms::spatial_position right{1, 0, 0};
dolbyio::comms::spatial_position up{0, 1, 0};
dolbyio::comms::spatial_position forward{0, 0, -1};
dolbyio::comms::spatial_scale scale{1, 1, 1};
```

Run the demo.py script and then follow the prompts: 
```
cd build/
python3 demo.py 
```

On MacOS/Windows the application will just run in terminal so entering **q** on command line will exit. Remember that each injector is running in its own process so you will have to **q+enter** for each spawned injector
instance.
When running on Ubuntu the process will run as daemon and is to be stopped using the python script (**when prompted by script make sure to provide same ALIAS and conversation as when you ran it**). 
```
python3 demo.py -stop yes

  Enter conference alias: c++sdk
  About to inject media into "c++sdk"
  injecting 0 bots
```
