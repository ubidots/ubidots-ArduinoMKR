/*
Copyright (c) 2013-2018 Ubidots.
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

#ifndef _UbiProtocol_H_
#define _UbiProtocol_H_

#include <WiFiNINA.h>
#include <WiFiUdp.h>

#include "UbiConstants.h"

class UbiProtocol {
protected:
  int _timeout;
  bool _debug;
  uint8_t _maxReconnectAttempts;

  const char *_host;
  const char *_user_agent;
  const char *_token;
  int _port;

public:
  explicit UbiProtocol(const char *host, const char *user_agent,
                       const char *token, int port)
      : _host(host), _user_agent(user_agent), _token(token), _port(port) {
    _timeout = 5000;
    _debug = false;
    _maxReconnectAttempts = 5;
  }

  virtual bool sendData(const char *device_label, const char *device_name,
                        char *payload) = 0;
  virtual double get(const char *device_label, const char *variable_label) = 0;
  virtual bool serverConnected();

  /**
   * Reconnects to the server
   * @return true once the host answer buffer length is greater than zero,
   *         false if timeout is reached.
   */
  template <class Client> bool reconnect(Client *client) {

    uint8_t attempts = 0;
    while (!client->connected() && attempts < _maxReconnectAttempts) {
      if (_debug) {
        Serial.print(F("Trying to connect to "));
        Serial.print(_host);
        Serial.print(F(" , attempt number: "));
        Serial.println(attempts);
      }
      client->connectSSL(_host, _port);
      if (_debug) {
        Serial.println(F("Attempt finished"));
      }
      attempts++;
      delay(1000);

      if (client->connected()) {
        return true;
      }
      if (attempts == _maxReconnectAttempts) {
        client->flush();
        client->stop();
      }
    }

    return false;
  }

  /**
   * Makes available debug traces
   */

  inline void setDebug(bool debug) { _debug = debug; }

private:
  bool reconnectionRoutine() {}
};

#endif
