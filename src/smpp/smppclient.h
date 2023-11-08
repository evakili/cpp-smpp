// Copyright (C) 2011-2014 OnlineCity
// Licensed under the MIT license, which can be read at: http://www.opensource.org/licenses/mit-license.php
// @author hd@onlinecity.dk & td@onlinecity.dk

#pragma once

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include "smpp/exceptions.h"
#include "smpp/pdu.h"
#include "smpp/smpp.h"
#include "smpp/smpp_params.h"
#include "smpp/sms.h"
#include "smpp/time_traits.h"
#include "smpp/timeformat.h"
#include "smpp/tlv.h"

DECLARE_string(system_type);
DECLARE_string(addr_range);
DECLARE_int32(socket_write_timeout);
DECLARE_int32(socket_read_timeout);
DECLARE_bool(null_terminate_octet_strings);
DECLARE_int32(addr_npi);
DECLARE_int32(interface_version);

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
  enum class ClientState {
    OPEN, BOUND_TX, BOUND_RX, BOUND_TRX
  };

  // CSMS types
  enum {
    CSMS_PAYLOAD, CSMS_16BIT_TAGS, CSMS_8BIT_UDH
  };

  explicit SmppClient(std::shared_ptr<asio::ip::tcp::socket>, asio::io_context& ctx);
  ~SmppClient();

  // Binds the client in transmitter mode.
  void BindTransmitter(const std::string &login, const std::string &password);

  // Binds the client in receiver mode.
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

  // Cancels a blocking call.
  void CancelBlocking();

  // Query the SMSC about current state/status of a previous sent SMS.
  // You must specify the SMSC assigned message id and source of the sent SMS.
  // Returns an std::tuple with elements: message_id, final_date, message_state and error_code.
  // message_state would be one of the smpp::STATE_* constants. (SMPP v3.4 section 5.2.28)
  // error_code depends on the telco network, so could be anything.
  //
  QuerySmResult QuerySm(std::string messageid, const SmppAddress &source);

  // Sends an enquire link command to SMSC and blocks until we a response.
  void EnquireLink();

  // Checks if the SMSC has sent us a enquire link command.
  // If they have, a response is sent.
  bool EnquireLinkRespond();

  // Returns true if the client is bound.
  inline bool IsBound() {
    return state_ != ClientState::OPEN;
  }

  inline void set_csms_method(const int csms_method) {
    csms_method_ = csms_method;
  }

  inline int csms_method() const {
    return csms_method_;
  }

  // Set callback method for generating message references.
  // The returned integer must be modulo 65535 (0xffff)
  inline void set_msg_ref_callback(std::function<uint16_t()> msg_ref_callback) {
    msg_ref_callback_ = msg_ref_callback;
  }

 private:
  // Binds the client to be in the mode specified in the mode parameter.
  void Bind(const CommandId &bind_cmd, const std::string &login, const std::string &password);

  // Constructs a PDU for binding the client.
  smpp::PDU MakeBindPdu(const CommandId &cmd_id, const std::string &login, const std::string &password);

  // Runs through the PDU queue and returns the first sms it finds, and sends a reponse to the SMSC.
  // While running through the PDU queuy, Alert notification and DataSm PDU are handled as well.
  // @return First sms found in the PDU queue or a null sms if there was no smses.
  smpp::SMS ParseSms();

  // Splits a string, without leaving a dangling escape character, into an vector of substrings of a given length,
  // @param shortMessage String to split.
  // @param split How long each substring should be.
  // @return Vector of substrings.
  static std::vector<std::string> Split(const std::string &short_message, const int split);

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
  void SendPdu(PDU *pdu);

  // Sends one PDU to the SMSC and blocks until we a response to it.
  // @param pdu PDU to send.
  // @return PDU PDU response to the one we sent.
  smpp::PDU SendCommand(PDU *pdu);

  // Returns one PDU from SMSC.
  PDU ReadPdu(const bool &);

  void ReadPduBlocking();

  void HandleTimeout(bool *callback_result, const boost::system::error_code &error);

  // Async write handler.
  // @param
  // @throw TransportException if an error occurred.
  void WriteHandler(bool *callback_result, const boost::system::error_code &error);

  // Peeks at the socket and returns true if there is data to be read.
  //
  // Because ASIO doesn't allow peeking. SocketPeek accutally reads the
  // data on the socket and if it is a valid PDU, it is put in the pdu
  // queue.
  bool SocketPeek();

  // Executes any pending async operations on the socket.
  void SocketExecute();

  // Handler for reading a PDU header.
  // If we read a valid PDU header on the socket, the readPduBodyHandler is invoked.
  void ReadPduHeaderHandler(const boost::system::error_code &error, size_t read,
                            const PduLengthHeader *pdu_length);

  void ReadPduHeaderHandlerBlocking(bool *callback_result,
                                    const boost::system::error_code &error, size_t read,
                                    const PduLengthHeader *pdu_length);

  // Handler for reading a PDU body.
  // Reads a PDU body on the socket and pushes it onto the PDU queue.
  void ReadPduBodyHandler(const boost::system::error_code &error, size_t read,
                          const PduLengthHeader *pdu_length,
                          const PduData *pdu_buffer);

  // Returns a response for a PDU we have sent,
  // specified by its sequence number and its command id.
  // First the PDU queue is checked for any responses,
  // if we don't find any we do a blocking read on the socket until
  // we get the desired response.
  //
  // @param sequence Sequence number to look for.
  // @param commandId Command id to look for.
  // @return PDU response to PDU with the given sequence number and command id.
  PDU ReadPduResponse(const uint32_t &sequence, const CommandId &command_id);

  // Checks the connection.
  // @throw TransportException if there was an problem with the connection.
  void CheckConnection();

  // Checks if the client is in the desired state.
  // @param state Desired state.
  // @throw SmppException if the client is not in the desired state.
  void CheckState(const ClientState &state);

  // Default implementation for msgRefCallback.
  // Simple initializes a integer on the heap and increments it for each message reference.
  // Returns a modulo 0xffff int.
  // @return
  static uint16_t DefaultMessageRef();

  int csms_method_;  // Method to use when dealing with concatenated messages.
  std::function<uint16_t()> msg_ref_callback_;
  ClientState state_;  // Bind state
  asio::io_context& ctx_;
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  smpp::ChronoDeadlineTimer timer_;
  uint32_t seq_no_;
  std::list<PDU> pdu_queue_;
};
}  // namespace smpp
