/*
Copyright (c) 2013-2019 Ubidots.
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
Developed and maintained by Jose Garcia and Cristian Arrieta for IoT Services
Inc
@jotathebest at github: https://github.com/jotathebest
@crisap94 at github: https://github.com/crisap94
*/

#include "UbiHttp.h"

#include "UbiUtils.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

UbiHTTP::UbiHTTP(const char *host, const int port, const char *user_agent,
                 const char *token)
    : UbiProtocol(host, user_agent, token, port) {}

/**************************************************************************
 * Destructor
 ***************************************************************************/

UbiHTTP::~UbiHTTP() {
  delete[] _host;
  delete[] _user_agent;
  delete[] _token;
}

bool UbiHTTP::sendData(const char *device_label, const char *device_name,
                       char *payload) {
  /* Connecting the client */

  _client_https_ubi.connect(_host, _port);
  reconnect(_host, _port);

  if (!_client_https_ubi.connected()) {
    if (_debug) {
      Serial.println(F("[ERROR] Could not connect to the server"));
    }
    return false;
  }

  bool result = false;

  if (_client_https_ubi.connected()) { // Connect to the host
    /* Builds the request POST - Please reference this link to know all the
     * request's structures https://ubidots.com/docs/api/ */

    int content_length = strlen(payload);

    uint16_t pathLength = _pathLength(device_label, "");
    char *path = (char *)malloc(sizeof(char) * pathLength + 1);
    memset(path, '\0', pathLength);
    sprintf(path, "/api/v1.6/devices/%s", device_label);
    Serial.println(path);
    uint16_t requestLength =
        _buildRequestLength(device_label, payload, pathLength);

    Serial.println(F("Making request to Ubidots:\n"));

    char *request = (char *)malloc(sizeof(char) * requestLength + 1);
    sprintf(request,
            "POST %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "X-Auth-Token: %s\r\n"
            "Connection: close\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %i\r\n"
            "\r\n"
            "%s"
            "\r\n",
            path, _host, _user_agent, _token, content_length, payload);

    if (_debug) {
      Serial.println(request);
    }
    _client_https_ubi.print(request);
    _client_https_ubi.flush();

    free(path);
    free(request);

    /* Reads the response from the server */
    if (waitServerAnswer()) {
      if (_debug) {
        Serial.println(F("\nUbidots' Server response:\n"));

        const char *serverReponse = _client_https_ubi.readString().c_str();
        if (strstr(serverReponse, "400 Bad Request") != NULL ||
            strstr(serverReponse, "Internal Server Error") != NULL) {
          Serial.println(F("[Error] There has been an error in the request"));
          _client_https_ubi.flush();
          _client_https_ubi.stop();
        } else {
          Serial.println(serverReponse);
        }
      }

      result = true;
    } else {
      if (_debug) {
        Serial.println(F("Could not read server's response"));
      }
    }
  } else { // Could not connect to the server
    if (_debug) {
      Serial.println(F("Could not send data to ubidots using HTTP"));
    }
  }

  _client_https_ubi.stop();
  return result;
}

/**
 * @brief Calculate the lenght of the request line to be send over HTTP to the
 * server
 *
 * @param device_label label set by the user
 * @param payload  request built by the Library
 * @param pathLength Lengh of the request line
 * @return uint16_t Total length of the request
 */
uint16_t UbiHTTP::_buildRequestLength(const char *device_label,
                                      const char *payload,
                                      uint16_t pathLength) {
  uint16_t endpointLength =
      strlen("POST  HTTP/1.1\r\n"
             "Host: \r\n"
             "User-Agent: \r\n"
             "X-Auth-Token: \r\n"
             "Connection: close\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: \r\n"
             "\r\n"
             "\r\n") +
      pathLength + strlen(device_label) + strlen(_host) + strlen(_user_agent) +
      strlen(_token) + UbiUtils::countDigit(strlen(payload)) + strlen(payload);

  return endpointLength;
}

double UbiHTTP::get(const char *device_label, const char *variable_label) {
  if (_debug) {
    Serial.print(F("Connecting to "));
    Serial.println(_host);
  }

  if (!_client_https_ubi.connectSSL(_host, _port)) {
    if (_debug) {
      Serial.println(F("Connection Failed to Ubidots - Try Again"));
    }
    if (!reconnect(_host, _port)) {
      return ERROR_VALUE;
    }
  }

  uint16_t pathLength = _pathLength(device_label, variable_label);
  char *path = (char *)malloc(sizeof(char) * pathLength + 1);
  sprintf(path, "/api/v1.6/devices/%s/%s/lv", device_label, variable_label);

  if (_debug) {
    Serial.print(F("Requesting to URL: "));
    Serial.println(path);
  }

  uint16_t requestLineLength = _requestLineLength(path);
  char *message = (char *)malloc(sizeof(char) * requestLineLength + 1);
  sprintf(message,
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "X-Auth-Token: %s\r\n"
          "User-Agent: %s\r\n"
          "Content-Type: application/json\r\n"
          "Connection: close\r\n"
          "\r\n",
          path, _host, _token, _user_agent);

  if (_debug) {
    Serial.println(F("Request sent"));
    Serial.println(message);
  }

  _client_https_ubi.print(message);

  while (_client_https_ubi.connected()) {
    String line = _client_https_ubi.readStringUntil('\n');
    if (line == "\r") {
      if (_debug) {
        Serial.println(F("Headers received"));
      }
      break;
    }
  }

  free(message);
  free(path);

  double value = _parseServerAnswer();
  _client_https_ubi.flush();
  _client_https_ubi.stop();
  // return 0;
  return value;
}

double UbiHTTP::_parseServerAnswer() {

  /**
   * @param _charResponseLength 3 is the maximun amount of digits for the length
   * to have
   */
  char *_charLength = (char *)malloc(sizeof(char) * 3);

  readServerAnswer(_charLength);

  /**
   * The server respond the length of the value in HEX so it has to be converted
   * to DEC
   * */
  uint8_t length = UbiUtils::hexadecimalToDecimal(_charLength);

  if (_debug) {
    Serial.print(F("Length: "));
    Serial.println(length);
  }

  char *_charValue = (char *)malloc(sizeof(char) * length + 1);

  readServerAnswer(_charValue);

  double value = strtof(_charValue, NULL);

  if (_debug) {
    Serial.print("Value: ");
    Serial.println(value);
  }

  free(_charLength);
  free(_charValue);

  return value;
}

/**
 * @brief Calculate the lenght of the request line to be send over HTTP to the
 * server
 *
 * @param path address of the endpoint to gather the data
 * @return uint16_t  Lenght of the request line
 */
uint16_t UbiHTTP::_requestLineLength(char *path) {
  uint16_t endpointLength = strlen("GET  HTTP/1.1\r\n"
                                   "Host: \r\n"
                                   "X-Auth-Token: \r\n"
                                   "User-Agent: \r\n"
                                   "Content-Type: application/json\r\n"
                                   "Connection: close\r\n"
                                   "\r\n") +
                            strlen(path) + strlen(_host) + strlen(_token) +
                            strlen(_user_agent);
  return endpointLength;
}

/**
 * @brief Calculate the lenght of the path to be send over HTTP to the server
 *
 * @param device_label device label of the device
 * @param variable_label variable label to be updated or fetched
 * @return uint16_t  Lenght of the endpoint path
 */
uint16_t UbiHTTP::_pathLength(const char *device_label,
                              const char *variable_label) {
  uint16_t endpointLength = strlen("/api/v1.6/devices///lv") +
                            strlen(device_label) + strlen(variable_label);
  return endpointLength;
}

/**
 * Reconnects to the server
 * @return true once the host answer buffer length is greater than zero,
 *         false if timeout is reached.
 */

bool UbiHTTP::reconnect(const char *host, const int port) {
  uint8_t attempts = 0;
  while (!_client_https_ubi.connected() && attempts < _maxReconnectAttempts) {
    if (_debug) {
      Serial.print(F("Trying to connect to "));
      Serial.print(host);
      Serial.print(F(" , attempt number: "));
      Serial.println(attempts);
    }
    _client_https_ubi.connectSSL(host, port);
    if (_debug) {
      Serial.println(F("Attempt finished"));
    }
    attempts++;
    delay(1000);

    if (_client_https_ubi.connected()) {
      return true;
    }
    if (attempts == _maxReconnectAttempts) {
      _client_https_ubi.flush();
      _client_https_ubi.stop();
    }
  }

  return false;
}

/**
 * @arg response [Mandatory] Pointer to store the server's answer
 */

void UbiHTTP::readServerAnswer(char *_serverResponse) {
  bool _firstChar = true;
  while (_client_https_ubi.available()) {
    if (_firstChar) {
      _firstChar = false;
      sprintf(_serverResponse, "%c", _client_https_ubi.read());
    }
    char *c = (char *)malloc(sizeof(char) * 2);
    sprintf(c, "%c", _client_https_ubi.read());
    if (*c == '\r') {
      // Get the last character \n to enable the function to run again
      _client_https_ubi.read();
      free(c);
      break;
    } else if (*c == 'e') {
      /**
       * After 18 digits it will show the response in scientific notation,and
       * the arquitechture does not have space store such a huge number
       * */
      sprintf(_serverResponse, "%f", ERROR_VALUE);
      if (_debug) {
        Serial.println(
            F("[ERROR]The value from the server exceeded memory capacity"));
      }
      break;
    } else if (*c == '<') {
      sprintf(_serverResponse, "%f", ERROR_VALUE);
      if (_debug) {
        Serial.println(F("[ERROR] Internal Server Error"));
      }
      break;
    } else {
      strcat(_serverResponse, c);
    }
  }
}

/**
 * Function to wait for the host answer up to the already set _timeout.
 * @return true once the host answer buffer length is greater than zero,
 *         false if timeout is reached.
 */

bool UbiHTTP::waitServerAnswer() {
  int timeout = 0;
  while (!_client_https_ubi.available() && timeout < _timeout) {
    timeout++;
    delay(1);
    if (timeout > _timeout - 1) {
      if (_debug) {
        Serial.println(F("timeout, could not read any response from the host"));
      }
      return false;
    }
  }
  return true;
}

/**
 * Makes available debug traces
 */

void UbiHTTP::setDebug(bool debug) { _debug = debug; }

/*
 * Checks if the socket is still opened with the Ubidots Server
 */

bool UbiHTTP::serverConnected() { return _client_https_ubi.connected(); }