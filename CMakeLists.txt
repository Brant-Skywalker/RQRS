cmake_minimum_required(VERSION 3.16)
project(CS225_SP22)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

add_executable(RQRS
        main.cpp
        fibonacciHeap.h
        fibonacciHeap.cpp
        registrationRecord.h
        registrationRecord.cpp
        utilities.h
        utilities.cpp
        eventDriver.h
        eventDriver.cpp
        recordProcessor.h
        recordProcessor.cpp
        databaseSchema.h
        databaseSchema.cpp
        centralizedQueue.cpp
        centralizedQueue.h
        BTree.h
        BTree.cpp
        BPlusTree.h
        BPlusTree.cpp
        queue.cpp
        queue.h
        config.h
        )