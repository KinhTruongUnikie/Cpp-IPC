# Cpp-IPC
- [Cpp-IPC](#cpp-ipc)
  - [Summary](#summary)
  - [How to run](#how-to-run)
    - [Build the program](#build-the-program)
    - [Run the program](#run-the-program)
      - [Pipe](#pipe)
      - [Message queue](#message-queue)
      - [Shared memory](#shared-memory)
## Summary
This is a file transfering program using different IPC methods including pipe, message queue and shared memory region.  
Test cases are implemented using googletest under test and testReceive folders. 
## How to run 
### Build the program
`./environment_setup.sh` to set up bazel environment  
`./build_and_test.sh` to build the programs and run all tests
### Run the program
#### Pipe
```
./bazel-bin/src/send --pipe pipeName --file filename
./bazel-bin/src/receive --pipe pipeName --file filename
```
#### Message queue
```
./bazel-bin/src/send --queue queueName --file filename
./bazel-bin/src/receive --queue queueName --file filename
```
#### Shared memory
```
./bazel-bin/src/send --shm shmName --file filename
./bazel-bin/src/receive --shm shmName --file filename
```