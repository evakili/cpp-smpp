//
// Copyright (C) 2014 OnlineCity
// Licensed under the MIT license, which can be read at: http://www.opensource.org/licenses/mit-license.php
//

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <string>

#include "gtest/gtest.h"
#include "smpp/pdu.h"

TEST(PduTest, readWrite) {
  uint32_t sequence = 3;
  smpp::PDU pdu(smpp::CommandId::NOT_DEFINED, smpp::ESME::RINVCMDLEN, sequence);
  std::string str("test");
  uint32_t i32 = 0xdeadbeef;
  uint16_t i16 = 0x1337;
  uint8_t i8 = 0x80;
  pdu << str;
  pdu << i32;
  pdu << i16;
  pdu << i8;
  pdu.ResetMarker();
  std::string s;
  uint32_t o32;
  uint16_t o16;
  uint8_t o8;
  pdu >> s;
  ASSERT_EQ(str, s);
  pdu >> o32;
  EXPECT_EQ(o32, i32);
  pdu >> o16;
  EXPECT_EQ(o16, i16);
  pdu >> o8;
  EXPECT_EQ(o8, i8);
}

TEST(PduTest, binary) {
  uint8_t testheader[] = { 0x00, 0x00, 0x00, 0x1c };
  uint8_t testdata[] = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74,
    0x00, 0xde, 0xad, 0xbe, 0xef, 0x13, 0x37, 0x80 };
  smpp::PduLengthHeader head;
  smpp::PduData data;
  data.resize(0x1c-4);
  std::copy(testheader, testheader + 4, head.begin());
  std::copy(testdata, testdata + 0x1c - 4, data.begin());
  smpp::PDU pdu(head, data);
  std::string s;
  uint32_t o32;
  uint16_t o16;
  uint8_t o8;
  pdu >> s;
  ASSERT_EQ(s, std::string("test"));
  pdu >> o32;
  EXPECT_EQ(o32, uint32_t(0xdeadbeef));
  pdu >> o16;
  EXPECT_EQ(o16, uint16_t(0x1337));
  pdu >> o8;
}

TEST(PduTest, enumTest) {
  smpp::PDU p;
  p << smpp::TON::INTERNATIONAL;
  smpp::TON t;
  p >> t;
  ASSERT_EQ(t, smpp::TON::INTERNATIONAL);

  smpp::PDU pdu;
  pdu << smpp::NPI(0xff01);
  smpp::NPI npi;
  pdu >> npi;
  ASSERT_EQ(npi, smpp::NPI::E164);
}

