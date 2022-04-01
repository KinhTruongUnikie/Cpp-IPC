#!/usr/bin/bash
bazel build //test:test
./bazel-bin/test/test 
