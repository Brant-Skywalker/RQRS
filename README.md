# Registration Queueing Reporting System
This is a toy C++ project! Please don't take it seriously!

![alt text](https://github.com/Brant-Skywalker/RQRS/blob/main/sys_architecture.png?raw=true)


### How to build and run?

**Please build and run this program on Linux or macOS.**

If you **have to** use Windows to compile this program (*Hold on, are you sure about this?*), please remove the compiler
flags in `CMakeLists.txt` since `-Werror`
will prevent you from compiling. Also, please replace all the color codes in `config.h` with empty strings since they
will not
display as excepted on Windows.

Navigate to the build directory and run **CMake (version >= 3.16)** to configure the project and generate a native build
system. Run `sudo apt install cmake` or `brew install cmake` to install CMake.

```bash
cd build
cmake ..
cmake --build .
./RQRS
```

### Project Features

- [x] *Beautiful* Color Scheme (may not work correctly in Windows)
- [x] STL-style data structure interfaces
- [x] Passing by value boosted with move semantics
- [x] Comparing and hashing of customized objects achieved by specializations of `std::hash` and `std::less` injected
  into `namespace std`.
- [x] Fair IO exception handling design
- [x] Uniform naming convention
- [x] Function headers and detailed documentation.
- [x] Code optimized with modern C++ features
- [x] Memory leaks resolved by using smart pointer.
- [x] ... and so on! :)

### Functionalities

Please enter integers between 0 and 9 to pick the next operation.
The prompt below is also shown in the program.

    1. Move 12 hours forward.
    2. Move a few days forward.
    3. Create new registrations.
    4. Withdraw a registration.
    5. Recover a registration.
    6. Update the profession category for a record.
    7. Update the risk status for a record.
    8. Add a treatment deadline for a record.
    9. Print this prompt again.
    10. Retrieve a Database record by ID.     <- Implemented with B+-tree.
    11. Retrieve a Database record by NAME.   <- Implemented with B-tree.
    12. Remove a Database record by ID.       <- Implemented with B+-tree.
    13. Remove a Database record by NAME.     <- Implemented with B-tree.
    0. Exit!

### Important IO Information

* All input records will be written into the `data/reg_x.csv` that you specified!
* All treatment records will be written into `data/appointment.csv` (will be generated if not exists).
* All reports generated (weekly or monthly) will be output to console *and* written into `data/report.txt` (will be
  generated
  if not exists) in the sorted order you just specified. *Magic?*

### Assumptions

We assume that there are **5** local registries and **5** appointment locations. This is the default configuration
in `config.h`, but it's possible to adapt this program to an arbitrary number of local registries and appointment
locations.

But to make sure the program works correctly, csv data files must be provided correctly. **They must be put in a `data`
directory and `data` must be in the same directory as executable `RQRS`.** For `m` local registries, `m`
csv files containing registration records must be named from `reg_1.csv` to
`reg_m.csv`. They **must** contain rows in format

| ID  | Name | Address | Phone | WeChat | Email        | Profession ID | Date of Birth | Risk Status | Registry ID |
|-----|------|---------|-------|--------|--------------|---------------|---------------|-------------|-------------|
| 666 | KD   | ZJUI    | 911   | N/A    | kd@gmail.com | 1             | 1900-01-01    | 0           | 1           |

For `n` appointment locations, a `location_preference.csv` **must** be provided with rows in the following format

| Registries ID | Preference #1 | Preference #2 | ...... | Preference #n |
|---------------|---------------|---------------|--------|---------------|
| 3             | 9             | 8             | ...... | 1             |

Otherwise, the program will not work correctly. *(May or may not add error detection functions later.)*

To simplify the code, records are compared and hashed only based on their **ID**s and **Name**s. In other words, two
records with the same **ID** and **Name** will get the same hash values, and they will be considered equal in *
searching* operations. **However, they will be assigned different priorities in the centralized queue as expected.**


### Project File Structure

```
project
|   main.cpp
|   config.h
|   fibonacciHeap.h
|   fibonacciHeap.cpp
|   centralizedQueue.h
|   centralizedQueue.cpp
|   queue.h
|   queue.cpp
|   registrationRecord.h
|   registrationRecord.cpp
|   eventDriver.h
|   eventDriver.cpp
|   recordProcessor.h
|   recordProcessor.cpp
|   databaseSchema.h
|   databaseSchema.cpp
|   BPlusTree.h
|   BPlusTree.cpp
|   BTree.h
|   BTree.cpp
|   utilities.h
|   utilities.cpp
|
└───build
  └───data
    │   reg_1.csv
    │   reg_2.csv
    │   reg_3.csv
    │   reg_4.csv
    │   reg_5.csv
    │   location_preferences.csv
```

### Other Notes

If you are having difficulties compiling with **CMake**, please use the `Makefile` below.
Before execution, please run `cp -r build/data .` to make sure `RQRS` executable and the `data` folder are under the
same directory.

```makefile
outname := RQRS

CXX := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -Werror -std=c++17 -g

srcfiles := $(shell find . -maxdepth 1 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(outname)

$(outname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(outname) $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(objects)

clear: clean
	rm -f *~ .depend $(outname)

include .depend
```
