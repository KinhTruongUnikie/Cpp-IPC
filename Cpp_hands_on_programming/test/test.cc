#include <iostream>
#include <string>
#include <vector>
#include "lib/other/Prog_init.h"
#include "lib/pipe/Ipc_pipe.h"
#include "lib/queue/Ipc_queue.h"
#include "lib/shm/Ipc_shm.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <fcntl.h>
#include "lib/abstract/Ipc_method.h"


// std::streambuf * init_captureCout(std::streambuf * sbuf) {
//     // save cout stream buffer
//     auto cout_stream_buf = std::cout.rdbuf();
//     // redirect cout stream buffer to string stream buffer
//     std::cout.rdbuf(sbuf);
//     return cout_stream_buf;
// }
// void close_captureCout(std::streambuf *cout_stream_buf) {
//     std::cout.rdbuf(cout_stream_buf);
// }

class Prog_initTest : public::testing::Test {
protected:
    Prog_init init;
    
    static void SetUpTestSuite() {
        std::cout << "START PROG_INIT TESTS" << std::endl;
    }

    static void TearDownTestSuite() {
        std::cout << "END OF PROG_INIT TESTS" << std::endl;
    }

    // Prog_initTest() {
    //     std::cout << "Constructor is called" << std::endl;
    // }

    // void SetUp() override {
    //     std::cout << "SetUp is called" << std::endl;
    // }

    // void TearDown() override {
    //     std::cout << "TearDown is called" << std::endl;
    // }

    // ~Prog_initTest() {
    //     std::cout << "Destructor is called" << std::endl;
    // }

};

TEST_F(Prog_initTest, PrintInstruction) {
    std::stringstream strm;
    auto cout_stream_buf = std::cout.rdbuf();
    std::string expected = "This program is a file data transfer program in IPC environment, it is for IPC methods practice.\n";
    expected += "The program shall receive an argument starts with \"--\"\n";
    expected += "The options below are available for different file transfer usages(may require argument), --file option is mandatory:\n\n";
    expected += "--help(-h): display all commands instruction and description(no arguments)\n";
    expected += "--queue(-q): uses queue as IPC method(argument required)\n";
    expected += "--pipe(-p): uses name pipe(FIFO) as IPC method(argument required)\n";
    expected += "--shm(-s): uses shared memory buffer as IPC method(argument required)\n";
	expected += "--file(-f): file used for read/write data(argument required)\n\n";
    // redirect cout to string stream strm
    std::cout.rdbuf(strm.rdbuf());
    init.printInstruction();
    // reset cout back to original stream buffer
    std::cout.rdbuf(cout_stream_buf);
    EXPECT_THAT(expected, testing::Eq(strm.str()));
}

std::map<std::string, Methods> input_label = {
    {"--pipe", Methods::PIPES},
    {"--queue", Methods::QUEUE},
    {"--shm", Methods::SHM}
};

// Parameterized test with class fixture
class Prog_initParamTest : public Prog_initTest, public ::testing::WithParamInterface<std::pair<const std::string, Methods>> {};

TEST_P(Prog_initParamTest, CheckOptionsInfoValues) {
    auto expected = Ipc_info("file", "argument", GetParam().second); 
    // Simulate options and argument passed in terminal
    std::vector<std::string> v = {"program name", "--file", "file", GetParam().first, "argument"};
    char *p[5];
    for (int i = 0; i < static_cast<int>(v.size()); i++) {
        p[i] = &v[i][0];
    }
    auto actual = init.checkOptions(5, p);
    EXPECT_THAT(expected.getFilename(), testing::Eq(actual.getFilename()));
    EXPECT_THAT(expected.getArgument(), testing::Eq(actual.getArgument()));
    EXPECT_THAT(expected.getMethod(), testing::Eq(actual.getMethod()));
}

INSTANTIATE_TEST_SUITE_P(MethodSelectionVariousInputs, Prog_initParamTest, 
testing::ValuesIn(input_label), 
[](const testing::TestParamInfo<Prog_initParamTest::ParamType> &info) {
    return info.param.first.substr(2);
});

std::map<std::string, std::shared_ptr<Ipc_method>> input_ipc_pointer = {
    {"Pipe", std::make_shared<Ipc_pipe>("test", "dummy")},
    {"Queue", std::make_shared<Ipc_queue>("test", "dummy")},
    {"SHM", std::make_shared<Ipc_shm>("test", "dummy")}
};

class GetFileSizeParamTest : public::testing::TestWithParam<std::pair<const std::string, std::shared_ptr<Ipc_method>>> {};


TEST_P(GetFileSizeParamTest, MultipleIpcPointer) {
    auto size = GetParam().second->getFileSize("test/test2");
    EXPECT_THAT(size, testing::Eq(174536)); //174536 is the size of test2 file
}

INSTANTIATE_TEST_SUITE_P(PolymorphismInput, GetFileSizeParamTest, 
testing::ValuesIn(input_ipc_pointer), 
[](const testing::TestParamInfo<GetFileSizeParamTest::ParamType> &info) {
    return info.param.first;
});

TEST(PipeSendTest, SendData) { 
    auto p = Ipc_pipe("gpipe", "test/test2");
    std::stringstream strm;
    auto cout_stream_buf = std::cout.rdbuf(); 
    // redirect cout to string stream strm
    std::cout.rdbuf(strm.rdbuf());
    p.send();
    // reset cout back to original stream buffer
    std::cout.rdbuf(cout_stream_buf);
    EXPECT_THAT(strm.str(), testing::EndsWith("File delivered by pipe successfully, exiting the program..\n"));
} 

// TEST_F(Prog_initTest, MethodSelection) {
//     auto info = std::make_shared<Ipc_info>("random", "random", SHM); 
//     std::string expected = "Starting shmSend..\n";
//     testing::internal::CaptureStdout();
//     init.run_IPC(info, SEND);
//     std::string actual = testing::internal::GetCapturedStdout();
//     EXPECT_THAT(expected, testing::StartsWith(actual));
// }