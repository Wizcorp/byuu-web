#pragma once

#include <emscripten/fiber.h>
typedef struct {
    emscripten_fiber_t context;
    char *asyncify_stack;
    char *c_stack;
} Fiber;

struct thread_handle_t {
  Thread* thread;
  thread_handle_t* parent = nullptr;
};

typedef thread_handle_t* thread_handle;