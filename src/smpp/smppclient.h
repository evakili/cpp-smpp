// Copyright (C) 2011-2014 OnlineCity
// Licensed under the MIT license, which can be read at: http://www.opensource.org/licenses/mit-license.php
// @author hd@onlinecity.dk & td@onlinecity.dk

#pragma once

#include <boost/numeric/conversion/cast.hpp>
#include <glog/logging.h>

#include <cstdint>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <utility>

#include "asio.hpp"
#include "smpp/exceptions.h"
#include "smpp/pdu.h"
#include "smpp/smpp.h"
#include "smpp/smpp_params.h"
#include "smpp/sms.h"
#include "smpp/timeformat.h"
#include "smpp/time_traits.h"
#include "smpp/tlv.h"

namespace smpp {
typedef asio::basic_deadline_timer<std::chrono::system_clock, smpp::CXX11Traits<std::chrono::system_clock>>
    ChronoDeadlineTimer;
typedef std::tuple<std::string, std::chrono::time_point<std::chrono::system_clock>, int, int> QuerySmResult;

// Class for sending and receiving SMSes through the SMPP protocol.
// This clients goal is to simplify sending an SMS and receiving
// delivery reports and therefore not all features of the SMPP protocol is
// implemented.
class SmppClient {
 public:
  // CSMS types
  enum {
    CSMS_PAYLOAD, CSMS_16BIT_TAGS, CSMS_8BIT_UDH
  };

 private:
  enum {
    OPEN, BOUND_TX, BOUND_RX, BOUND_TRX
  };

  // SMPP bind parameters
  std::string systemType;
  uint8_t interfaceVersion;  // interfaceVersion = 0x34;
  uint8_t addrTon;  // addrTon = 0;
  uint8_t addrNpi;  // addrNpi = 0;
  std::string addrRange;

  // Extra options;
  bool nullTerminateOctetStrings;
  // Method to use when dealing with concatenated messages.
  int csmsMethod;

  std::function<uint16_t()> msgRefCallback;
  int state;
  std::shared_ptr<asio::ip::tcp::socket> socket;
  uint32_t seqNo;
  std::list<PDU> pdu_queue;
  // Socket write timeout in milliseconds. Default is 5000 milliseconds.
  int socketWriteTimeout;
  // Socket read timeout in milliseconds. Default is 30000 milliseconds.
  int socketReadTimeout;

  bool verbose;

 public:
  explicit SmppClient(std::shared_ptr<asio::ip::tcp::socket>);
  ~SmppClient();

  /**
   // Binds the client in transmitter mode.
   // @param login SMSC login.
   // @param password SMS password.
   */
  void BindTransmitter(const std::string &login, const std::string &password);

   // Binds the client in receiver mode.
   // @param login SMSC login
   // @param password SMSC password.
  void BindReceiver(const std::string &login, const std::string &password);

  // Unbinds the client.
  void Unbind();

  // Sends an SMS to the SMSC with default parameters and no optional tags
  // The SMS is split into multiple messages if it dosen't into one.
  // Returns smsc id and number of smses sent.
  std::pair<std::string, int> SendSms(const SmppAddress &sender, const SmppAddress &receiver,
                                      const std::string &short_message);

  // Sends an SMS to the SMSC with extra Smpp parameters
  // The SMS is split into multiple messages if it dosen't into one.
  // Returns smsc id and number of smses sent.
  std::pair<std::string, int> SendSms(const SmppAddress &sender, const SmppAddress &receiver,
                                      const std::string &short_message,
                                      const SmppParams &params);

  // Sends an SMS to the SMSC
  // The SMS is split into multiple messages if it dosen't into one.
  // Returns smsc id and number of smses sent.
  std::pair<std::string, int> SendSms(const SmppAddress &sender, const SmppAddress &receiver,
                                      const std::string &shortMessage,
                                      const SmppParams &params,
                                      std::list<TLV> tags);
  // Returns the first SMS in the PDU queue,
  // or does a blocking read on the socket until we receive an SMS from the SMSC.
  smpp::SMS ReadSms();

  // Query the SMSC about current state/status of a previous sent SMS.
  // You must specify the SMSC assigned message id and source of the sent SMS.
  // Returns an std::tuple with elements: message_id, final_date, message_state and error_code.
  // message_state would be one of the SMPP::STATE_* constants. (SMPP v3.4 section 5.2.28)
  // error_code depends on the telco network, so could be anything.
  //
  // @param messageid
  // @param source
  // @return QuerySmResult
  QuerySmResult QuerySm(std::string messageid, SmppAddress source);

  // Sends an enquire link command to SMSC and blocks until we a response.
  void EnquireLink();

  // Checks if the SMSC has sent us a enquire link command.
  // If they have, a response is sent.
  void EnquireLinkRespond();

  // Returns true if the client is bound.
  bool IsBound() {
    return state != OPEN;
  }

  void setSystemType(const std::string s) {
    systemType = s;
  }

  std::string getSystemType() const {
    return systemType;
  }

  void setInterfaceVersion(const uint8_t i) {
    interfaceVersion = i;
  }

  uint8_t getInterfaceVersion() const {
    return interfaceVersion;
  }

  void setAddrTon(const uint8_t i) {
    addrTon = i;
  }

  uint8_t getAddrTon() const {
    return addrTon;
  }

  void setAddrNpi(const uint8_t i) {
    addrNpi = i;
  }

  uint8_t getAddrNpi() const {
    return addrNpi;
  }

  void setAddrRange(const std::string &s) {
    addrRange = s;
  }

  std::string getAddrRange() const {
    return addrRange;
  }

  void setNullTerminateOctetStrings(const bool b) {
    nullTerminateOctetStrings = b;
  }

  bool getNullTerminateOctetStrings() const {
    return nullTerminateOctetStrings;
  }

  void setCsmsMethod(const int &method) {
    csmsMethod = method;
  }

  int getCsmsMethod() const {
    return csmsMethod;
  }

  // Sets the socket read timeout in milliseconds. Default is 5000 milliseconds.
  // @param timeout Socket read timeout in milliseconds.
  void setSocketReadTimeout(const int &timeout) {
    socketReadTimeout = timeout;
  }

  // Returns the socket read timeout.
  // @return Socket read timeout in milliseconds.
  int getSocketReadTimeout() const {
    return socketReadTimeout;
  }

  // Sets the socket write timeout in milliseconds. Default is 30000 milliseconds.
  // @param timeout Socket write timeout in milliseconds.
  void setSocketWriteTimeout(const int &timeout) {
    socketWriteTimeout = timeout;
  }

  // Returns the socket write timeout in milliseconds.
  // @return Socket write timeout in milliseconds.
  int getSocketWriteTimeout() const {
    return socketWriteTimeout;
  }

  void setVerbose(const bool b) {
    verbose = b;
  }

  bool isVerbose() const {
    return verbose;
  }

  // Set callback method for generating message references.
  // The returned integer must be modulo 65535 (0xffff)
  // @param cb
  void setMsgRefCallback(std::function<uint16_t()> cb) {
    msgRefCallback = cb;
  }

 private:
  // Binds the client to be in the mode specified in the mode parameter.
  // @param mode Mode to bind client in.
  // @param login SMSC login.
  // @param password SMSC password.
  void Bind(uint32_t mode, const std::string &login, const std::string &password);

  // Constructs a PDU for binding the client.
  // @param mode Mode to bind client in.
  // @param login SMSC login.
  // @param password SMSC password.
  // @return PDU for binding the client.
  smpp::PDU SetupBindPdu(uint32_t mode, const std::string &login, const std::string &password);

  // Runs through the PDU queue and returns the first sms it finds, and sends a reponse to the SMSC.
  // While running through the PDU queuy, Alert notification and DataSm PDU are handled as well.
  // @return First sms found in the PDU queue or a null sms if there was no smses.
  smpp::SMS ParseSms();

  // Splits a string, without leaving a dangling escape character, into an vector of substrings of a given length,
  // @param shortMessage String to split.
  // @param split How long each substring should be.
  // @return Vector of substrings.
  std::vector<std::string> Split(const std::string &short_message, const int split);

  // Sends a SUBMIT_SM pdu with the required details for sending an SMS to the SMSC.
  // It blocks until it gets a response from the SMSC.
  // @param sender
  // @param receiver
  // @param short_message
  // @param params
  // @param tags
  // @return SMSC sms id.
  std::string SubmitSm(const SmppAddress &sender,
      const SmppAddress &receiver,
      const std::string &short_message,
      const struct SmppParams &params,
      std::list<TLV> tags);

  // @return Returns the next sequence number.
  // @throw SmppException Throws an SmppException if we run out of sequence numbers.
  uint32_t NextSequenceNumber();

  // Sends one PDU to the SMSC.
  void SendPdu(PDU &pdu);

  // Sends one PDU to the SMSC and blocks until we a response to it.
  // @param pdu PDU to send.
  // @return PDU PDU response to the one we sent.
  smpp::PDU SendCommand(PDU &pdu);

  // Returns one PDU from SMSC.
  PDU ReadPdu(const bool &);

  void ReadPduBlocking();

  void HandleTimeout(bool *had_error, const asio::error_code &error);

  // Async write handler.
  // @param
  // @throw TransportException if an error occurred.
  void WriteHandler(bool *had_error, const asio::error_code &error);

  // Peeks at the socket and returns true if there is data to be read.
  // @return True if there is data to be read.
  bool SocketPeek();

  // Executes any pending async operations on the socket.
  void SocketExecute();

  // Handler for reading a PDU header.
  // If we read a valid PDU header on the socket, the readPduBodyHandler is invoked.
  // @param error Boost error code
  // @param read Bytes read
  void ReadPduHeaderHandler(const asio::error_code &error, size_t read,
                            const PduLengthHeader *pduLength);

  void ReadPduHeaderHandlerBlocking(bool *had_error,
                                    const asio::error_code &error, size_t read,
                                    const PduLengthHeader *pduLength);

  // Handler for reading a PDU body.
  // Reads a PDU body on the socket and pushes it onto the PDU queue.
  //
  // @param error Boost error code
  // @param read Bytes read
  void ReadPduBodyHandler(const asio::error_code &error, size_t read,
                          const PduLengthHeader *pduLength,
                          const PduData *pduBuffer);

  // Returns a response for a PDU we have sent,
  // specified by its sequence number and its command id.
  // First the PDU queue is checked for any responses,
  // if we don't find any we do a blocking read on the socket until
  // we get the desired response.
  //
  // @param sequence Sequence number to look for.
  // @param commandId Command id to look for.
  // @return PDU response to PDU with the given sequence number and command id.
  PDU ReadPduResponse(const uint32_t &sequence, const uint32_t &commandId);

  // Checks the connection.
  // @throw TransportException if there was an problem with the connection.
  void CheckConnection();

  // Checks if the client is in the desired state.
  // @param state Desired state.
  // @throw SmppException if the client is not in the desired state.
  void CheckState(const int state);

  // Default implementation for msgRefCallback.
  // Simple initializes a integer on the heap and increments it for each message reference.
  // Returns a modulo 0xffff int.
  // @return
  static uint16_t DefaultMessageRef();
};
}  // namespace smpp
