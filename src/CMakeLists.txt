SET(headers
	smpp/exceptions.h
	smpp/gsmencoding.h
	smpp/pdu.h
	smpp/smppclient.h
	smpp/smpp.h
	smpp/sms.h
	smpp/timeformat.h
  smpp/time_traits.h
	smpp/tlv.h
	smpp/hexdump.h
  smpp/smpp_params.h
)

SET(sources
	smpp/gsmencoding.cc
	smpp/pdu.cc
	smpp/smppclient.cc
	smpp/smpp.cc
	smpp/sms.cc
	smpp/timeformat.cc
	smpp/hexdump.cc
)


ADD_LIBRARY(smpp STATIC ${sources} ${headers})
SET_TARGET_PROPERTIES(smpp PROPERTIES VERSION 1.0.0 SOVERSION 1 PUBLIC_HEADER "${headers}")
TARGET_LINK_LIBRARIES(smpp PUBLIC
	Boost::boost
	Boost::date_time
	glog::glog
	gflags::gflags
	$<$<CXX_COMPILER_ID:MSVC>:Ws2_32>
)

SET(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
SET(CMAKE_INSTALL_HEADERDIR include CACHE PATH "Output directory for headers")
INSTALL(TARGETS smpp LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_HEADERDIR}/smpp)
