#ifndef ENUM
#define ENUM

typedef enum {
	PIPES,
	QUEUE,
	SHM,
	NONE
} methods;

typedef enum {
    RECEIVE,
    SEND
} send_or_receive;

typedef enum{
	DATA_SIZE = 4096
} buff_size;

#endif