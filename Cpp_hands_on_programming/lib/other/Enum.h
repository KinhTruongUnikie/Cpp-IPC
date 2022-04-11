#ifndef ENUM
#define ENUM

#define DATA_SIZE 4096

enum class Methods{
	PIPES,
	QUEUE,
	SHM,
	NONE
};

enum class Send_or_receive{
    RECEIVE,
    SEND
};

#endif