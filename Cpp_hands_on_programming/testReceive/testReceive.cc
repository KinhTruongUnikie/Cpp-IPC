#include <iostream>
#include <string>
#include <vector>
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

TEST(PipeReceiveTest, ReceiveData) { 
    auto p = Ipc_pipe("gpipe", "fileGtest");
    std::stringstream strm;
    auto cout_stream_buf = std::cout.rdbuf(); // save cout stream buffer
    // redirect cout to string stream strm
    std::cout.rdbuf(strm.rdbuf());
    p.receive();
    // reset cout stream buffer to original
    std::cout.rdbuf(cout_stream_buf);
    EXPECT_THAT(p.getFileSize("fileGtest"), testing::Eq(174536)); //174536 is the file size of test2 (sent by PipeSendTest)
    EXPECT_THAT(strm.str(), testing::EndsWith("Write to file from pipe successfully, exiting the program..\n")); 
    unlink("fileGtest");
} 