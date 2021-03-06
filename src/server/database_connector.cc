#include "database_connector.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <cstring>

static constexpr unsigned char DATASOURCE_NAME[20] = "tibero";
static constexpr SQLHANDLE UNINITIALIZED_HANDLE_VALUE =  INT32_MIN;

template <typename Function, typename... Args>
static inline bool CallCLIFunction(Function f, Args... args) {
  SQLRETURN rc = f(args...);
  return rc == SQL_SUCCESS;
}

DatabaseConnector::DatabaseConnector() 
  : henv_(UNINITIALIZED_HANDLE_VALUE),
    hdbc_(UNINITIALIZED_HANDLE_VALUE),
    connected_(false) { 
}

DatabaseConnector::~DatabaseConnector() {
  if (connected_) {
    if (!Disconnect())
      assert("Error occurred while disconnecting from problem database");
  }
}

bool DatabaseConnector::Disconnect() {
  if (!connected_)
    return true;

  if (hdbc_ != UNINITIALIZED_HANDLE_VALUE) {
    if (!CallCLIFunction(SQLFreeConnect, hdbc_))
      return false;
  }

  if (henv_ != UNINITIALIZED_HANDLE_VALUE) {
    if (!CallCLIFunction(SQLFreeEnv, henv_))
      return false;
  }
 
  connected_ = false;
  return true;
}

bool DatabaseConnector::Connect(std::string user, std::string password) {
  if (connected_)
    return true;

  if (!CallCLIFunction(SQLAllocEnv2, &henv_, true))
    return false;
  
  if (!CallCLIFunction(SQLAllocConnect, henv_, &hdbc_))
    return false;

  if (!CallCLIFunction(SQLConnect, hdbc_, (SQLCHAR *)DATASOURCE_NAME, SQL_NTS, 
                       (SQLCHAR *)user.c_str(), SQL_NTS, (SQLCHAR *)password.c_str(), SQL_NTS))
    return false;

  connected_ = true;
  return true;
}

const bool DatabaseConnector::IsConnected() const {
  return connected_;
}

const SQLHDBC DatabaseConnector::GetConnection() const {
  if (!connected_) {
    throw std::runtime_error("GetConnection is not available");
  }

  return hdbc_;
}
