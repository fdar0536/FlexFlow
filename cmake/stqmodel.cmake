set(STQ_MODEL_LIBS
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

    model/dao/dirutils.cpp
    model/dao/dirutils.hpp

    #sqlite
    model/dao/sqliteconnect.cpp
    model/dao/sqliteconnect.hpp
    model/dao/sqlitequeuelist.cpp
    model/dao/sqlitequeuelist.hpp
    model/dao/sqlitequeue.cpp
    model/dao/sqlitequeue.hpp

    # proc
    model/proc/iproc.cpp
    model/proc/iproc.hpp
    model/proc/task.cpp
    model/proc/task.hpp
)

if (ENABLE_CLI OR ENABLE_GUI)
    list(APPEND MODEL_SRC
        #grpc
        model/dao/grpcconnect.cpp
        model/dao/grpcconnect.hpp
        model/dao/grpcqueue.cpp
        model/dao/grpcqueue.hpp
        model/dao/grpcqueuelist.cpp
        model/dao/grpcqueuelist.hpp
        model/dao/grpcutils.cpp
        model/dao/grpcutils.hpp
    )
endif (ENABLE_CLI OR ENABLE_GUI)

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

add_library(stqmodel STATIC
    ${MODEL_SRC}
)

add_dependencies(stqmodel grpc_common)

target_link_libraries(stqmodel
    PRIVATE

    ${STQ_MODEL_LIBS}
)
