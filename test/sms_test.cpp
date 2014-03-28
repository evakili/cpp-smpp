/*
 * Copyright (C) 2014 OnlineCity
 * Licensed under the MIT license, which can be read at: http://www.opensource.org/licenses/mit-license.php
 */
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <algorithm>
#include <list>
#include <string>

#include "gtest/gtest.h"
#include "smpp/sms.h"
#include "smpp/smpp.h"
#include "smpp/tlv.h"

using std::list;
using std::string;

/**
 * Test the construct and copy-construct of SMS
 * The purpose of this test is to reveal a corruption by the use of TLV params (and std::copy without resizing)
 */

TEST(SmsTest, copy) {
    uint8_t testheader[] = { 0x00, 0x00, 0x00, 57 };
    uint8_t testdata[] = {
            0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x34, 0x35, 0x32,
            0x36, 0x31, 0x35, 0x39, 0x39, 0x31, 0x37, 0x00, 0x05, 0x00, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x69, 0x00, 0x04, 0x27, 0x00, 0x01, 0x02 };

    PduLengthHeader head;
    std::copy(testheader, testheader + 4, head.begin());
    PduData data;
    data.resize(53);
    std::copy(testdata, testdata + 53, data.begin());

    smpp::PDU pdu(head, data);
    smpp::SMS sms(pdu);
    smpp::SMS sms2(sms);
    ASSERT_EQ(sms.is_null, sms2.is_null);
    ASSERT_TRUE(!sms2.is_null);

    // Compare TLVs
    ASSERT_EQ(sms.tlvs.size(), sms2.tlvs.size());
    list<smpp::TLV>::iterator it;
    list<smpp::TLV>::iterator it2;
    it = sms.tlvs.begin();
    it2 = sms2.tlvs.begin();
    while (it != sms.tlvs.end()) {
        ASSERT_EQ((*it).getTag(), (*it2).getTag());
        ASSERT_EQ((*it).getLen(), (*it2).getLen());
        ASSERT_EQ((*it).getOctets(), (*it2).getOctets());
        it++;
        it2++;
    }
}

TEST(SmsTest, dlr) {
    using boost::gregorian::date;
    using boost::posix_time::ptime;
    using boost::posix_time::time_duration;
    uint8_t testheader[] = { 0x00, 0x00, 0x00, 0xe4 };
    uint8_t testdata[] = {
            0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x34, 0x35, 0x32,
            0x36, 0x31, 0x35, 0x39, 0x39, 0x31, 0x37, 0x00, 0x05, 0x00, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x88, 0x69, 0x64, 0x3a, 0x64, 0x63, 0x30, 0x64, 0x63,
            0x38, 0x65, 0x63, 0x36, 0x37, 0x65, 0x31, 0x36, 0x30, 0x38, 0x32, 0x34, 0x38, 0x33, 0x66, 0x39, 0x65, 0x38,
            0x63, 0x64, 0x31, 0x62, 0x31, 0x33, 0x35, 0x64, 0x64, 0x20, 0x73, 0x75, 0x62, 0x3a, 0x30, 0x30, 0x31, 0x20,
            0x64, 0x6c, 0x76, 0x72, 0x64, 0x3a, 0x30, 0x30, 0x31, 0x20, 0x73, 0x75, 0x62, 0x6d, 0x69, 0x74, 0x20, 0x64,
            0x61, 0x74, 0x65, 0x3a, 0x31, 0x31, 0x31, 0x30, 0x32, 0x36, 0x31, 0x36, 0x34, 0x36, 0x20, 0x64, 0x6f, 0x6e,
            0x65, 0x20, 0x64, 0x61, 0x74, 0x65, 0x3a, 0x31, 0x31, 0x31, 0x30, 0x32, 0x36, 0x31, 0x36, 0x34, 0x37, 0x20,
            0x73, 0x74, 0x61, 0x74, 0x3a, 0x44, 0x45, 0x4c, 0x49, 0x56, 0x52, 0x44, 0x20, 0x65, 0x72, 0x72, 0x3a, 0x30,
            0x30, 0x30, 0x20, 0x74, 0x65, 0x78, 0x74, 0x3a, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c,
            0x64, 0x40, 0x04, 0x27, 0x00, 0x01, 0x02, 0x00, 0x1e, 0x00, 0x21, 0x64, 0x63, 0x30, 0x64, 0x63, 0x38, 0x65,
            0x63, 0x36, 0x37, 0x65, 0x31, 0x36, 0x30, 0x38, 0x32, 0x34, 0x38, 0x33, 0x66, 0x39, 0x65, 0x38, 0x63, 0x64,
            0x31, 0x62, 0x31, 0x33, 0x35, 0x64, 0x64, 0x00 };

    PduLengthHeader head;
    std::copy(testheader, testheader + 4, head.begin());
    PduData data;
    data.resize(0xe0);
    std::copy(testdata, testdata + 0xe0, data.begin());

    smpp::PDU pdu(head, data);
    smpp::SMS sms(pdu);
    smpp::DeliveryReport dlr(sms);

    // Assertions for SMS
    EXPECT_EQ(sms.source_addr, string("4526159917"));
    EXPECT_EQ(sms.source_addr_ton, 1);
    EXPECT_EQ(sms.source_addr_npi, 1);
    EXPECT_EQ(sms.dest_addr, string("default"));
    EXPECT_EQ(sms.dest_addr_ton, 5);
    EXPECT_EQ(sms.dest_addr_npi, 0);
    EXPECT_EQ(sms.esm_class, smpp::ESM_DELIVER_SMSC_RECEIPT);
    EXPECT_TRUE(!sms.is_null);
    EXPECT_EQ(sms.data_coding, smpp::DATA_CODING_ISO8859_1);

    // Assertions for TLV fields
    EXPECT_EQ(static_cast<int>(sms.tlvs.size()), 2);
    list<smpp::TLV>::iterator it;
    it = sms.tlvs.begin();
    EXPECT_EQ(it->getTag(), smpp::tags::MESSAGE_STATE);
    EXPECT_EQ(it->getOctets()[0], smpp::STATE_DELIVERED);
    it++;
    EXPECT_EQ(it->getTag(), smpp::tags::RECEIPTED_MESSAGE_ID);
    EXPECT_EQ(it->getOctets(), string("dc0dc8ec67e16082483f9e8cd1b135dd", 33));

    // Assertions for DLR part of SMS
    EXPECT_EQ(dlr.id, string("dc0dc8ec67e16082483f9e8cd1b135dd"));
    EXPECT_EQ(dlr.sub, uint32_t(1));
    EXPECT_EQ(dlr.dlvrd, uint32_t(1));
    EXPECT_EQ(dlr.submitDate, ptime(date(2011, boost::gregorian::Oct, 26), time_duration(16, 46, 0)));
    EXPECT_EQ(dlr.doneDate, ptime(date(2011, boost::gregorian::Oct, 26), time_duration(16, 47, 0)));
    EXPECT_EQ(dlr.stat, string("DELIVRD"));
    EXPECT_EQ(dlr.err, string("000"));
}

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
