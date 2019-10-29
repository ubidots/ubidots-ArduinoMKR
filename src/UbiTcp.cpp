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
Developed and maintained by Jose Garcia for IoT Services Inc
@jotathebest at github: https://github.com/jotathebest
*/

#include "UbiTcp.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

UbiTcp::UbiTcp(const char* host, const int port, const char* user_agent, const char* token) {
  _host = host;
  _user_agent = user_agent;
  _token = token;
  _port = port;
}

/**************************************************************************
 * Overloaded destructor
 ***************************************************************************/

UbiTcp::~UbiTcp() {
  delete[] _host;
  delete[] _user_agent;
  delete[] _token;
}

/***************************************************************************
FUNCTIONS TO SEND/RETRIEVE DATA
***************************************************************************/

bool UbiTcp::sendData(const char* device_label, const char* device_name, char* payload) {
  /* Connecting the client */
  _client_tcps_ubi.connect(_host, UBIDOTS_TCPS_PORT);
  reconnect(_host, UBIDOTS_TCPS_PORT);

  if (!_client_tcps_ubi.connected()) {
    if (_debug) {
      Serial.println("[ERROR] Could not connect to the host");
    }
    _client_tcps_ubi.stop();
    return false;
  }

  /* Sends data to Ubidots */
  if (_debug) {
    Serial.println("[INFO] Payload to send:");
    Serial.println(payload);
  }
  _client_tcps_ubi.print(payload);

  /* Waits for the host's answer */
  if (!waitServerAnswer()) {
    if (_debug) {
      Serial.println("[ERROR] Could not read server's response");
    }
    _client_tcps_ubi.stop();
    return false;
  }

  /* Parses the host answer, returns true if it is 'Ok' */
  char* response = (char*)malloc(sizeof(char) * 100);

  float value = parseTCPAnswer("POST", response);
  free(response);
  if (value != ERROR_VALUE) {
    _client_tcps_ubi.stop();
    return true;
  }

  _client_tcps_ubi.stop();
  return false;
}
float UbiTcp::get(const char* device_label, const char* variable_label) {
  _client_tcps_ubi.connect(_host, UBIDOTS_TCPS_PORT);
  reconnect(_host, UBIDOTS_TCPS_PORT);

  if (_client_tcps_ubi.connected()) {
    /* Builds the request POST - Please reference this link to know all the
     * request's structures https://ubidots.com/docs/api/ */
    _client_tcps_ubi.print(_user_agent);
    _client_tcps_ubi.print("|LV|");
    _client_tcps_ubi.print(_token);
    _client_tcps_ubi.print("|");
    _client_tcps_ubi.print(device_label);
    _client_tcps_ubi.print(":");
    _client_tcps_ubi.print(variable_label);
    _client_tcps_ubi.print("|end");

    if (_debug) {
      Serial.println("----");
      Serial.println("Payload for request:");
      Serial.print(_user_agent);
      Serial.print("|LV|");
      Serial.print(_token);
      Serial.print("|");
      Serial.print(device_label);
      Serial.print(":");
      Serial.print(variable_label);
      Serial.print("|end");
      Serial.println("\n----");
    }

    /* Waits for the host's answer */
    if (!waitServerAnswer()) {
      if (_debug) {
        Serial.println("[ERROR] Could not read the server's response");
      }
      _client_tcps_ubi.stop();
      return ERROR_VALUE;
    }

    /* Reads the response from the server */
    char* response = (char*)malloc(sizeof(char) * MAX_BUFFER_SIZE);
    float value = parseTCPAnswer("LV", response);
    _client_tcps_ubi.stop();
    free(response);
    return value;
  }

  if (_debug) {
    Serial.println("ERROR could not connect to the server");
  }

  _client_tcps_ubi.stop();
  return ERROR_VALUE;
}

/***************************************************************************
Auxiliar functions
***************************************************************************/

/**
 * Makes available debug traces
 */
void UbiTcp::setDebug(bool debug) { _debug = debug; }

/*
 * Checks if the socket is still opened with the Ubidots Server
 */
bool UbiTcp::serverConnected() { return _client_tcps_ubi.connected(); }

/**
 * Function to wait for the host answer up to the already set _timeout.
 * @return true once the host answer buffer length is greater than zero,
 *         false if timeout is reached.
 */

bool UbiTcp::waitServerAnswer() {
  int timeout = 0;
  while (!_client_tcps_ubi.available() && timeout < _timeout) {
    timeout++;
    delay(1);
    if (timeout > _timeout - 1) {
      if (_debug) {
        Serial.println("timeout, could not read any response from the host");
      }
      return false;
    }
  }
  return true;
}

/**
 * Parse the TCP host answer and saves it to the input char pointer.
 * @payload [Mandatory] char payload pointer to store the host answer.
 * @return true if there is an 'Ok' in the answer, false if not.
 */
float UbiTcp::parseTCPAnswer(const char* request_type, char* response) {
  int j = 0;

  if (_debug) {
    Serial.println("----------");
    Serial.println("Server's response:");
  }

  while (_client_tcps_ubi.available()) {
    char c = _client_tcps_ubi.read();
    if (_debug) {
      Serial.write(c);
    }
    response[j] = c;
    j++;
    if (j >= MAX_BUFFER_SIZE - 1) {
      break;
    }
  }

  if (_debug) {
    Serial.println("\n----------");
  }

  response[j] = '\0';
  float result = ERROR_VALUE;

  // POST
  if (request_type == "POST") {
    char* pch = strstr(response, "OK");
    if (pch != NULL) {
      result = 1;
    }
    return result;
  }

  // LV
  char* pch = strchr(response, '|');
  if (pch != NULL) {
    result = atof(pch + 1);
  }

  return result;
}

/**
 * Reconnects to the server
 * @return true once the host answer buffer length is greater than zero,
 *         false if timeout is reached.
 */
void UbiTcp::reconnect(const char* host, const int port) {
  uint8_t attempts = 0;
  while (!_client_tcps_ubi.status() && attempts < 5) {
    if (_debug) {
      Serial.print("Trying to connect to ");
      Serial.print(host);
      Serial.print(" , attempt number: ");
      Serial.println(attempts);
    }
    _client_tcps_ubi.stop();
    _client_tcps_ubi.connect(host, port);
    attempts += 1;
    delay(1000);
  }
}
