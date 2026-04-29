set(FF_model_LIBS
    protobuf::libprotobuf
    gRPC::grpc++
    SQLite::SQLite3
    spdlog::spdlog

    grpc_common
)

set(MODEL_SRC


    # model
    model/errmsg.cpp
    model/errmsg.hpp
    model/utils.cpp
    model/utils.hpp
    model/defines.h

    # global
    controller/global/global.cpp
    controller/global/global.hpp

    # DAO
    model/dao/iconnect.cpp
    model/dao/iconnect.hpp
    model/dao/iqueuelist.hpp
    model/dao/iqueue.hpp

    #sqlite
    model/dao/sqlite/connect.cpp
    model/dao/sqlite/connect.hpp
    model/dao/sqlite/queuelist.cpp
    model/dao/sqlite/queue.hpp
    model/dao/sqlite/queue.cpp
    model/dao/sqlite/queuelist.hpp
    
    #grpc
    model/dao/grpc/connect.cpp
    model/dao/grpc/connect.hpp
    model/dao/grpc/queue.cpp
    model/dao/grpc/queue.hpp
    model/dao/grpc/queuelist.cpp
    model/dao/grpc/queuelist.hpp
    model/dao/grpc/utils.cpp
    model/dao/grpc/utils.hpp

    model/utils.cpp
    model/utils.hpp

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
else ()
    list(APPEND MODEL_SRC
        model/proc/posixproc.cpp
        model/proc/posixproc.hpp
    )
endif (WIN32)

if (LINUX)
    list(APPEND MODEL_SRC
        model/proc/linuxproc.cpp
        model/proc/linuxproc.hpp
    )
else (APPLE)
    list(APPEND MODEL_SRC
        model/proc/macproc.cpp
        model/proc/macproc.hpp
    )
endif (LINUX)

add_library(ffmodel STATIC
    ${MODEL_SRC}
)

add_dependencies(ffmodel grpc_common)

target_link_libraries(ffmodel
    PRIVATE

    ${FF_model_LIBS}
)

set_property(TARGET ffmodel PROPERTY POSITION_INDEPENDENT_CODE ON)
