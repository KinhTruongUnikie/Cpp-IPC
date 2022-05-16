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
#include <thread>
#include "testlib/PolymorphismInput.h"
#include "testlib/CaptureStdOut.h"
#include "testlib/Prog_initTest.h"

std::map<std::string, std::shared_ptr<Ipc_method>> input_ipc_pointer = {
    {"pipe", std::make_shared<Ipc_pipe>("gpipe", "test2")},
    {"pipeBigfile", std::make_shared<Ipc_pipe>("gpipe2", "bigfile")},
    {"queue", std::make_shared<Ipc_queue>("/gqueue", "test2")},
    {"queueBigfile", std::make_shared<Ipc_queue>("/gqueue2", "bigfile")},
    {"shm", std::make_shared<Ipc_shm>("/gshm", "test2")},
    {"shmBigfile", std::make_shared<Ipc_shm>("/gshm2", "bigfile")}, 
    {"timeoutPipe", std::make_shared<Ipc_pipe>("ptimeout", "test2", 1000)},
    {"timeoutQueue", std::make_shared<Ipc_queue>("/qtimeout", "test2", 1000)},
    {"timeoutShm", std::make_shared<Ipc_shm>("/stimeout", "test2", 1000)}       
};

INSTANTIATE_TEST_SUITE_P(MultipleIPCMethods, PolymorphismInput, 
testing::ValuesIn(input_ipc_pointer), 
[](const testing::TestParamInfo<PolymorphismInput::ParamType> &info) {
    return info.param.first;
});

TEST_P(PolymorphismInput, GetFileSize) {
    off_t size;
    if (GetParam().first.find("timeout") == std::string::npos) {
        if (GetParam().first.find("Bigfile") != std::string::npos) {
            size = GetParam().second->getFileSize("bigfile");
            EXPECT_THAT(size, testing::Eq(bigSize)); // size of bigfile file
        } else {
            size = GetParam().second->getFileSize("test2");
            EXPECT_THAT(size, testing::Eq(smallSize));
        }
        //unexist file test
        std::string emptyFile = "empty";
        unlink(emptyFile.c_str());
        size = GetParam().second->getFileSize(emptyFile);
        EXPECT_THAT(size, testing::Eq(-1));
    }
}

TEST_P(PolymorphismInput, SendData) {
    std::string coutString;
    if (GetParam().first.find("timeout") == std::string::npos) { 
        size_t pos;
        std::string expected = "File delivered by ";
        expected += GetParam().first;
        if ((pos = expected.find("Bigfile")) != std::string::npos) {
            expected = expected.substr(0, pos); 
        } 
        expected +=  " successfully, exiting the program..\n";

        {
            CaptureStdOut capture;
            GetParam().second->send();
            coutString = capture.getString();
        }
        EXPECT_THAT(coutString, testing::EndsWith(expected));
    } else {
        CaptureStdOut capture;
        EXPECT_THROW(GetParam().second->send(), std::runtime_error);
    }
}

TEST_F(Prog_initTest, PrintInstruction) {
    std::string expected = "This program is a file data transfer program in IPC environment, it is for IPC methods practice.\n";
    expected += "The program shall receive an argument starts with \"--\"\n";
    expected += "The options below are available for different file transfer usages(may require argument), --file option is mandatory:\n\n";
    expected += "--help(-h): display all commands instruction and description(no arguments)\n";
    expected += "--queue(-q): uses queue as IPC method(argument required)\n";
    expected += "--pipe(-p): uses name pipe(FIFO) as IPC method(argument required)\n";
    expected += "--shm(-s): uses shared memory buffer as IPC method(argument required)\n";
	expected += "--file(-f): file used for read/write data(argument required)\n\n";

    CaptureStdOut capture;
    init.printInstruction();
    EXPECT_THAT(capture.getString(), testing::Eq(expected));
}

std::map<std::string, Methods> input_methods = {
    {"--pipe", Methods::PIPES},
    {"--queue", Methods::QUEUE},
    {"--shm", Methods::SHM}
};

std::map<std::string, std::vector<std::string>> optionsCombination = {
    {"LongOptionSucceed1", {"programName", "--file", "file", "method", "argument"}},
    {"LonngOptionSucceed2", {"programName", "method", "argument", "--file", "file"}},
    {"ShortOptionSucceed1", {"programName", "-f", "file", "method", "argument"}},
    {"ShortOptionSucceed2", {"programName", "method", "argument", "-f", "file"}},
    {"NoFileOptionFail", {"programName","file", "method", "argument"}},
    {"NoMethodOptionFail", {"programName", "argument", "-f", "file"}},
    {"DuplicateFileOptionFail", {"programName", "--file", "file", "method", "argument", "--file", "file1"}},
    {"DuplicateMethodOptionFail", {"programName", "method", "argument", "--file", "file", "method", "argument1"}},
    // runIPC call is required to test the existence of file
    {"SendNonExistingFile", {"programName", "-f", "noexist", "method", "argument"}},
    {"SendDirectoryNameFile1", {"programName", "-f", "src", "method", "argument"}},
    {"SendDirectoryNameFile2", {"programName", "-f", "lib", "method", "argument"}},
    {"ReceiveDirectoryNameFile1", {"programName", "-f", "src", "method", "argument"}},
    {"ReceiveDirectoryNameFile2", {"programName", "-f", "lib", "method", "argument"}}
};
// Parameterized test with class fixture

INSTANTIATE_TEST_SUITE_P(MethodSelectionVariousInputs, Prog_initParamTest, 
testing::Combine(testing::ValuesIn(input_methods),testing::ValuesIn(optionsCombination)), 
[](const testing::TestParamInfo<Prog_initParamTest::ParamType> &info) {
    return std::get<0>(info.param).first.substr(2) + "_" + std::get<1>(info.param).first;
});

TEST_P(Prog_initParamTest, OptionBehavior) {
    auto method_map = std::get<0>(GetParam()); 
    auto option_map = std::get<1>(GetParam());
    auto expected = Ipc_info("file", "argument", method_map.second); 
    // Simulate options and argument passed in terminal
    std::vector<std::string> v = option_map.second;
    // Set up method flag e.g --pipe
    replace(v.begin(), v.end(), std::string("method"), method_map.first);
    char *p[v.size()];
    for (int i = 0; i < static_cast<int>(v.size()); i++) {
        p[i] = &v[i][0];
    }
    //Start checking test result
        
    if (option_map.first.find("Succeed") != std::string::npos) {
        auto actual = init.checkOptions(v.size(), p);
        EXPECT_THAT(expected.getFilename(), testing::Eq(actual.getFilename()));
        EXPECT_THAT(expected.getArgument(), testing::Eq(actual.getArgument()));
        EXPECT_THAT(expected.getMethod(), testing::Eq(actual.getMethod()));
    } else if (option_map.first.find("Fail") != std::string::npos) {
        EXPECT_THROW(init.checkOptions(v.size(), p), std::runtime_error);
    } else if (option_map.first.find("Send") != std::string::npos) {
        init.checkOptions(v.size(), p);
        EXPECT_THROW(init.run_IPC(Send_or_receive::SEND), std::runtime_error);
    } else {
        init.checkOptions(v.size(), p);
        EXPECT_THROW(init.run_IPC(Send_or_receive::RECEIVE), std::runtime_error);
    }
}

class ThreadParamTest : public::testing::TestWithParam<std::pair<const std::string, std::shared_ptr<Ipc_method>>> {
protected:
    static void SetUpTestSuite() {
        std::ofstream out("same");
        out.seekp(DATA_SIZE*20);
        out << "end";
        out.close();
    }

    static void TearDownTestSuite() {
        unlink("same");
    }
};

std::map<std::string, std::shared_ptr<Ipc_method>> ipc_pointer1 = {
    {"pipe", std::make_shared<Ipc_pipe>("threadpipe", "same", 1000)},
    {"queue", std::make_shared<Ipc_queue>("/threadqueue", "same", 1000)},
    {"shm", std::make_shared<Ipc_shm>("/threadshm", "same", 1000)}
};

INSTANTIATE_TEST_SUITE_P(ThreadTest, ThreadParamTest,
testing::ValuesIn(ipc_pointer1), 
[](const testing::TestParamInfo<ThreadParamTest::ParamType> &info) {
    return info.param.first;
});

TEST_P(ThreadParamTest, SameName) {
    CaptureStdOut capture;
    auto f = [](std::shared_ptr<Ipc_method> m, Send_or_receive side) {
        if (side == Send_or_receive::SEND) {
            EXPECT_THROW(m->send(), std::runtime_error);
        } else {
            EXPECT_THROW(m->receive(), std::runtime_error);
        }
    };
    std::thread send(f, GetParam().second, Send_or_receive::SEND); 
    std::thread receive(f, GetParam().second, Send_or_receive::RECEIVE);
    send.join();
    receive.join();
}