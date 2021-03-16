add_artery_feature(hybrid LteApp.cc ItsG5App.cc)
generate_opp_message(HybridServicesMessages.msg TARGET hybrid DIRECTORY hybrid_msgs)
generate_opp_message(HybridServicesMessagesLTE.msg TARGET hybrid DIRECTORY hybrid_msgs)
add_opp_run(hybrid_comm DEPENDENCY artery-lte NED_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR})
