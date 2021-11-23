
add_artery_feature(hybrid LteApp.cc ItsG5App.cc HybridApp.cc HybridService.cc SubApplication.cc BaseProtocol.cc DupCommMode.cc)
generate_opp_message(HybridServicesMessages.msg TARGET hybrid DIRECTORY hybrid_msgs)
add_opp_run(hybrid_comm DEPENDENCY artery-lte NED_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_PREFIX_PATH /home/byacheur/Apps/PytorchC++/libtorch)
find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

target_link_libraries(hybrid PRIVATE "${TORCH_LIBRARIES}")

set_property(TARGET hybrid PROPERTY CXX_STANDARD 14)