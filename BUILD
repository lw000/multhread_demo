# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary

cc_library(
    name = "core",
    # srcs = [
    #     "core/thread_pool.cpp",
    #     "core/msg_queue.cpp",
    # ],
    srcs = glob(["core/*.cpp"]),
)


cc_library(
    name = "net",
    srcs = glob(["net/*.cpp"]),
)

cc_binary(
    name = "multhread_demo",
    srcs = [
        "multhread_demo.cpp",
    ],
    copts = [],
    deps = [
        ":core",
        ":net"
    ],
)