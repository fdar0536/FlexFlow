set(FF_MODEL_LIBS
    protobuf::libprotobuf
    gRPC::grpc++
    SQLite::SQLite3
    spdlog::spdlog

    grpc_common
)

set(MODEL_SRC

    # DAO
    model/dao/iconnect.cpp
    model/dao/iconnect.hpp
    model/dao/iqueuelist.hpp
    model/dao/iqueue.hpp

    model/errmsg.cpp
    model/errmsg.hpp
    model/utils.cpp
    model/utils.hpp

    # global
    controller/global/defines.h
    controller/global/global.cpp
    controller/global/global.hpp

    model/dao/dirutils.cpp
    model/dao/dirutils.hpp

    #sqlite
    model/dao/sqliteconnect.cpp
    model/dao/sqliteconnect.hpp
    model/dao/sqlitequeuelist.cpp
    model/dao/sqlitequeuelist.hpp
    model/dao/sqlitequeue.cpp
    model/dao/sqlitequeue.hpp

    #grpc
    model/dao/grpcconnect.cpp
    model/dao/grpcconnect.hpp
    model/dao/grpcqueue.cpp
    model/dao/grpcqueue.hpp
    model/dao/grpcqueuelist.cpp
    model/dao/grpcqueuelist.hpp
    model/dao/grpcutils.cpp
    model/dao/grpcutils.hpp

    # proc
    model/proc/iproc.cpp
    model/proc/iproc.hpp
    model/proc/task.cpp
    model/proc/task.hpp
)

if (WIN32)
    list(APPEND MODEL_SRC
        model/proc/winproc.cpp
        model/proc/winproc.hpp
    )
elseif (LINUX)
    list(APPEND MODEL_SRC
        model/proc/linuxproc.cpp
        model/proc/linuxproc.hpp
    )
endif (WIN32)

if (ENABLE_C_MODEL)
    list(APPEND MODEL_SRC
        model/dao/capi.cpp
        model/dao/capi.hpp
        model/dao/connect.cpp
        model/dao/connect.h
        model/dao/handle.h
        model/dao/handlemanager.cpp
        model/dao/handlemanager.hpp
        model/dao/queue.cpp
        model/dao/queue.h
        model/dao/queuelist.cpp
        model/dao/queuelist.h
    )
endif(ENABLE_C_MODEL)

add_library(ffmodel STATIC
    ${MODEL_SRC}
)

add_dependencies(ffmodel grpc_common)

target_link_libraries(ffmodel
    PRIVATE

    ${FF_MODEL_LIBS}
)
