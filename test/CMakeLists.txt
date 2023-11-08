add_executable(unittest
  test_flags.h
  test_flags.cc
  sms_test.cc
  time_test.cc
  time_test.cc
  encoder_test.cc
  pdu_test.cc
  smpp_test.cc
  main.cc)
target_link_libraries(unittest PRIVATE smpp GTest::gtest)
add_test(unittest unittest)

add_executable(smppclient_test
  test_flags.h
  test_flags.cc
  smppclient_test.cc
  smppclient_test.h
  main.cc)
target_link_libraries(smppclient_test PRIVATE smpp GTest::gtest)
add_test(smppclient_test smppclient_test)

add_executable(smppclient_receive_test
  test_flags.h
  test_flags.cc
  smppclient_receive_test.cc
  smppclient_test.h
  main.cc)
target_link_libraries(smppclient_receive_test PRIVATE smpp GTest::gtest)
add_test(smppclient_receive_test smppclient_receive_test)

add_executable(smppclient_send_test
  test_flags.h
  test_flags.cc
  smppclient_send_test.cc
  smppclient_test.h
  main.cc)
target_link_libraries(smppclient_send_test PRIVATE smpp GTest::gtest)
add_test(smppclient_send_test smppclient_send_test)
