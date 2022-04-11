#!/usr/bin/bash
bazel build --cxxopt='-std=c++14' //src:send
bazel build --cxxopt='-std=c++14' //src:receive
bazel build --cxxopt='-std=c++14' //test:testSend
bazel build --cxxopt='-std=c++14' //testReceive:testReceive
./bazel-bin/test/testSend & 
./bazel-bin/testReceive/testReceive &
