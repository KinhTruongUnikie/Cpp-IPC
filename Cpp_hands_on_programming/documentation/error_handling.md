# Possible errors
- [Possible errors](#possible-errors)
  - [Summary](#summary)
  - [Option combination](#option-combination)
    - [Invalid option](#invalid-option)
    - [Invalid argument](#invalid-argument)
      - [IPC methods](#ipc-methods)
      - [File](#file)
  - [Timeout](#timeout)
    - [Only one process launched](#only-one-process-launched)
    - [Two different IPC methods launched](#two-different-ipc-methods-launched)
    - [One process terminated during run time](#one-process-terminated-during-run-time)
## Summary
This program is a file transfering program. There are two processes required to be launched in parallel so that the transfering is possible, i.e sender and receiver.   
Program throws an error and exit the program if one of the below scenarios happen .
## Option combination
### Invalid option
- No `--file/-f` option is provided
- No or more than one method option is provided, e.g `--pipe --queue` 
- Duplicate option, e.g `--shm --shm` 
### Invalid argument
#### IPC methods
The argument is the name of IPC method object being used, e.g `--shm /shmName`
- Argument is not provided for argument required option, e.g pipe, queue, shm
- Invalid name according to IPC method, e.g [message queue naming rule](https://man7.org/linux/man-pages/man7/mq_overview.7.html)

#### File
The argument is the name of the file being used, e.g `--file filename`
- Sender program provide nonexist file name
- Receiver program provide already exist file name and overwrite is not possible
- File name is the name of an already exist directory name
- Identical names(sender and receiver)

## Timeout
### Only one process launched
One of the two parallel processes is launched but the other is not, i.e sender is launched but receiver is not launched and vice versa. The running process will be terminated with an error message when its timer runs out. 
### Two different IPC methods launched
Two parallel processses are launched with different IPC methods, no connection can be established, both processes will be terminated with an error message when timer runs out.  
*E.g*
```
./bazel-bin/src/send --pipe pipe1 --file myfile
./bazel-bin/src/receive --queue /queue1 --file newfile
```
### One process terminated during run time
If one process is terminated while both are running, the remaining process will be terminated with an error message. 
