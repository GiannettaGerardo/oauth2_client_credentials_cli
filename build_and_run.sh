#!/bin/bash

clang main.c utility.c oauth2login.c api.c parser.c -lcurl -lpthread \
  -pedantic \
  -Wall \
  -Wextra \
  -Wfloat-equal \
  -Wundef \
  -Wshadow \
  -Wpointer-arith \
  -Wcast-align \
  -Wstrict-prototypes \
  -Wstrict-overflow=2 \
  -Wwrite-strings \
  -Waggregate-return \
  -Wcast-qual \
  -Wswitch-default \
  -Wswitch-enum \
  -Wconversion \
  -Wunreachable-code \
  -o execute
  # -fsanitize={address,undefined} \
  #-fsanitize={thread,undefined}

if [ $? -eq 0 ]; then
  ./execute
else
  exit 1
fi
