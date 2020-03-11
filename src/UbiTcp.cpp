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

#include "UbiTcp.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

UbiTCP::UbiTCP(const char *host, const int port, const char *user_agent,
               const char *token)
    : UbiProtocol(host, user_agent, token, port) {}

/**************************************************************************
 * Destructor
 ***************************************************************************/

UbiTCP::~UbiTCP() {
  delete[] _host;
  delete[] _user_agent;
  delete[] _token;
}

/**************************************************************************
 * Cloud Functions
 ***************************************************************************/

bool UbiTCP::sendData(const char *device_label, const char *device_name,
                      char *payload) {
  if (_debug) {
    Serial.print(F("Connecting to "));
    Serial.print(_host);
    Serial.print(F(" on Port: "));
    Serial.println(_port);
  }

  if (!_client_tcps_ubi.connectSSL(_host, _port)) {
    if (_debug) {
      Serial.println(F("Connection Failed to Ubidots - Try Again"));
    }
    if (!reconnect<WiFiSSLClient>(&_client_tcps_ubi)) {
      return ERROR_VALUE;
    }
  }

  if (_debug) {
    Serial.println(F("Paylaod"));
    Serial.println(payload);
  }

  _client_tcps_ubi.print(payload);

  /* Waits for the host's answer */
  if (!waitServerAnswer()) {
    if (_debug) {
      Serial.println("[ERROR] Could not read server's response");
    }
    _client_tcps_ubi.flush();
    _client_tcps_ubi.stop();
    return false;
  }

  float value = parseTCPAnswer("POST");
  if (value != ERROR_VALUE) {
    _client_tcps_ubi.flush();
    _client_tcps_ubi.stop();
    return true;
  }
  _client_tcps_ubi.flush();
  _client_tcps_ubi.stop();
  return false;
}

double UbiTCP::get(const char *device_label, const char *variable_label) {

  /* Connecting the client */

  if (_debug) {
    Serial.print(F("Connecting to "));
    Serial.print(_host);
    Serial.print(F(" on Port: "));
    Serial.println(_port);
  }

  if (!_client_tcps_ubi.connectSSL(_host, _port)) {
    if (_debug) {
      Serial.println(F("Connection Failed to Ubidots - Try Again"));
    }
    if (!reconnect<WiFiSSLClient>(&_client_tcps_ubi)) {
      return ERROR_VALUE;
    }
  }

  if (!_client_tcps_ubi.connected()) {
    if (_debug) {
      Serial.println(F("[ERROR] Could not connect to the server"));
    }
    return ERROR_VALUE;
  }

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
    return ERROR_VALUE;
  }

  float value = parseTCPAnswer("LV");
  _client_tcps_ubi.flush();
  _client_tcps_ubi.stop();
  return value;
  // return 0;
}

/**************************************************************************
 * Auxiliar
 ***************************************************************************/

/**
 * Function to wait for the host answer up to the already set _timeout.
 * @return true once the host answer buffer length is greater than zero,
 *         false if timeout is reached.
 */

bool UbiTCP::waitServerAnswer() {
  int timeout = 0;
  while (!_client_tcps_ubi.available() && timeout < _timeout) {
    timeout++;
    delay(1);
    if (timeout > _timeout - 1) {
      if (_debug) {
        Serial.println("timeout, could not read any response from the host");
      }
      _client_tcps_ubi.flush();
      _client_tcps_ubi.stop();
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

float UbiTCP::parseTCPAnswer(const char *request_type) {

  if (_debug) {
    Serial.println("----------");
    Serial.println("Server's response:");
  }
  char *readFromServer = (char *)malloc(sizeof(char) * 100);

  readFromServer = strdup(_client_tcps_ubi.readString().c_str());

  if (_debug) {
    Serial.println(readFromServer);
    Serial.println("----------");
  }

  float result = ERROR_VALUE;

  // POST
  if (strcmp(request_type, "POST") == 0) {
    char *pch = strstr(readFromServer, "OK");
    if (pch != NULL) {
      result = 1;
    }
    return result;
  }

  // LV
  char *pch = strchr(readFromServer, '|');
  if (pch != NULL) {
    result = atof(pch + 1);
  }

  free(readFromServer);

  return result;
}

/*
 * Checks if the socket is still opened with the Ubidots Server
 */

bool UbiTCP::serverConnected() { return _client_tcps_ubi.connected(); }
