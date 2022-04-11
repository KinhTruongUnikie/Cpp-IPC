#include <iostream>
#include <string>
#include <vector>
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

std::map<std::string, std::shared_ptr<Ipc_method>> input_ipc_pointer = {
    {"pipe", std::make_shared<Ipc_pipe>("gpipe", "pipeGtest")},
    {"queue", std::make_shared<Ipc_queue>("/gqueue", "queueGtest")}
    //{"shm", std::make_shared<Ipc_shm>("/gshm", "shmGtest")}
};

class PolymorphismInput : public::testing::TestWithParam<std::pair<const std::string, std::shared_ptr<Ipc_method>>> {};

INSTANTIATE_TEST_SUITE_P(MultipleIPCMethods, PolymorphismInput, 
testing::ValuesIn(input_ipc_pointer), 
[](const testing::TestParamInfo<PolymorphismInput::ParamType> &info) {
    return info.param.first;
});

TEST_P(PolymorphismInput, ReceiveData) {
    std::string expected = "Write to file from ";
    expected += GetParam().first;
    expected +=  " successfully, exiting the program..\n";

    std::stringstream strm;
    auto cout_stream_buf = std::cout.rdbuf(); // save cout stream buffer
    // redirect cout to string stream strm
    std::cout.rdbuf(strm.rdbuf());
    GetParam().second->receive();

    // reset cout stream buffer to original
    std::cout.rdbuf(cout_stream_buf);

    std::string file = GetParam().first + "Gtest";
    EXPECT_THAT(GetParam().second->getFileSize(file), testing::Eq(174536)); //174536 is the file size of test2 (sent by PipeSendTest)
    EXPECT_THAT(strm.str(), testing::EndsWith(expected)); 
    unlink(file.c_str());
}