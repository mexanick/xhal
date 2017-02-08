#include "xhal/XHALInterface.h"

xhal::XHALInterface::XHALInterface(const std::string& board_domain_name, const std::string& address_table_filename):
  m_board_domain_name(board_domain_name),
  m_address_table_filename(address_table_filename)
{
}

void xhal::XHALInterface::init()
{
  log4cplus::SharedAppenderPtr myAppender(new log4cplus::ConsoleAppender());
  std::auto_ptr<log4cplus::Layout> myLayout = std::auto_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout());
  myAppender->setLayout( myLayout );
  auto t_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("XHALInterface"));
  m_logger = t_logger;
  m_logger.addAppender(myAppender);
  m_logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
  INFO("XHAL Logger tuned up");

  m_parser = new xhal::utils::XHALXMLParser(m_address_table_filename);
  DEBUG("Address table name " << m_address_table_filename);
  m_parser->setLogLevel(2);
  m_parser->parseXML();

  try {
		rpc.connect(m_board_domain_name);
	}
	catch (RPCSvc::ConnectionFailedException &e) {
		//ERROR("Caught RPCErrorException: " << e.message.c_str());
    throw xhal::utils::Exception(strcat("RPC exception: ",e.message.c_str()));
	}
	catch (RPCSvc::RPCException &e) {
		//ERROR("Caught exception: " << e.message.c_str());
    throw xhal::utils::Exception(strcat("RPC exception: ",e.message.c_str()));
	}
}

void xhal::XHALInterface::loadModule(const std::string& module_name, const std::string& module_version)
{
  try {
    ASSERT(rpc.load_module(module_name, module_version));
  }
  STANDARD_CATCH;
}

void xhal::XHALInterface::setLogLevel(int loglevel)
{
  /*
  switch(loglevel)
  {
    case 0:
      m_logger.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
      m_parser->setLogLevel(0);
      break;
    case 1:
      m_logger.setLogLevel(log4cplus::WARN_LOG_LEVEL);
      m_parser->setLogLevel(1);
      break;
    case 2:
      m_logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
      m_parser->setLogLevel(2);
      break;
    case 3:
      m_logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
      m_parser->setLogLevel(3);
      break;
    case 4:
      m_logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
      m_parser->setLogLevel(4);
      break;
  }
  */
}

uint32_t xhal::XHALInterface::readReg(std::string regName)
{
  if (auto t_node = m_parser->getNode(regName.c_str()))
  {
    m_node = t_node.value();
	  req = RPCMsg("memory.read");
    req.set_word("address", m_node.real_address);
    req.set_word("count", 1);
    try {
    	rsp = rpc.call_method(req);
    }
    STANDARD_CATCH;
	  uint32_t result;
    try{
	    ASSERT(rsp.get_word_array_size("data") == 1);
	    rsp.get_word_array("data", &result);
    }
	  STANDARD_CATCH;
    return result;
  } else {
    //ERROR("Register not found in address table!");
    throw xhal::utils::Exception(strcat("XHAL XML exception: can't find node", regName.c_str()));
  }
}

uint32_t xhal::XHALInterface::readReg(uint32_t address)
{
  req = RPCMsg("memory.read");
  req.set_word("address", address);
  req.set_word("count", 1);
  try {
  	rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  uint32_t result;
  try{
    ASSERT(rsp.get_word_array_size("data") == 1);
    rsp.get_word_array("data", &result);
  }
  STANDARD_CATCH;
  return result;
}