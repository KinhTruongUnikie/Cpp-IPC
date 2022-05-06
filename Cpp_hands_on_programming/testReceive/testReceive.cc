#include <iostream>
#include <string>
#include <vector>
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include "testlib/PolymorphismInput.h"
#include "testlib/CaptureStdOut.h"

std::map<std::string, std::shared_ptr<Ipc_method>> input_ipc_pointer = {
    {"pipe", std::make_shared<Ipc_pipe>("gpipe", "pipeGtest")},
    {"pipeBigfile", std::make_shared<Ipc_pipe>("gpipe2", "pipeBigfileGtest")},
    {"queue", std::make_shared<Ipc_queue>("/gqueue", "queueGtest")},
    {"queueBigfile", std::make_shared<Ipc_queue>("/gqueue2", "queueBigfileGtest")},
    {"shm", std::make_shared<Ipc_shm>("/gshm", "shmGtest")},
    {"shmBigfile", std::make_shared<Ipc_shm>("/gshm2", "shmBigfileGtest")},
    {"timeoutPipe", std::make_shared<Ipc_pipe>("ptimeoutReceive", "file1", 1000)},
    {"timeoutQueue", std::make_shared<Ipc_queue>("/qtimeoutReceive", "file2", 1000)},
    {"timeoutShm", std::make_shared<Ipc_shm>("/stimeoutReceive", "file3", 1000)}

};

INSTANTIATE_TEST_SUITE_P(MultipleIPCMethods, PolymorphismInput, 
testing::ValuesIn(input_ipc_pointer), 
[](const testing::TestParamInfo<PolymorphismInput::ParamType> &info) {
    return info.param.first;
});

TEST_P(PolymorphismInput, ReceiveData) {
    std::string coutString;
    if (GetParam().first.find("timeout") == std::string::npos) {
        size_t pos;
        std::string expected = "Write to file from ";
        expected += GetParam().first;
        if ((pos = expected.find("Bigfile")) != std::string::npos) {
            expected = expected.substr(0, pos); 
        } 
        expected +=  " successfully, exiting the program..\n";
        {
            CaptureStdOut capture;
            GetParam().second->receive();
            coutString = capture.getString();
        }
            std::string file = GetParam().first + "Gtest";
            if (GetParam().first.find("Bigfile") != std::string::npos) {
            EXPECT_THAT(GetParam().second->getFileSize(file), testing::Eq(bigSize));
        } else {
            EXPECT_THAT(GetParam().second->getFileSize(file), testing::Eq(smallSize)); 
        }
        EXPECT_THAT(coutString, testing::EndsWith(expected)); 
        unlink(file.c_str());
        
    } else {
        CaptureStdOut capture;
        EXPECT_THROW(GetParam().second->receive(), std::runtime_error);
    }
}