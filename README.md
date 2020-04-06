
# Arduino MKR1010

The Arduino MKR WiFi 1010 is the easiest point of entry to basic IoT and pico-network application design. Whether you are looking at building a sensor network connected to your office or home router, or if you want to create a BLE device sending data to a cellphone, the MKR WiFi 1010 is your one-stop-solution for many of the basic IoT application scenarios.

## Requiremets

* [An MKR1010 module](https://store.arduino.cc/usa/mkr-wifi-1010).
* [Arduino IDE 1.6.5 or higher](https://www.arduino.cc/en/Main/Software)
* [Ubidots library](https://github.com/ubidots/ubidots-ArduinoMKR/archive/master.zip) 
* Jumper wires.

**NOTE:** This library is also compatible with the **Nano 33**, please refer to our [help center](https://help.ubidots.com/en/?q=nano33) to find guides and examples codes.   
  


## Setup

1. Download the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
2. Open you IDE and click on **File -> Preferences**.
3. Open the Boards Manager from **Tools -> Board** menu and search for "mkr" and install Arduino SAMD Boards (32-bits ARM Cortex-M0+).
4. Select the MKR1010 board from **Tools > Board > Arduino MKR WiFI 1010**.
5. Download the **Ubidots library** [here](https://github.com/ubidots/ubidots-ArduinoMKR/archive/master.zip).
6. Now, click on **Sketch -> Include Library -> Add .ZIP Library**.
7. Select the .ZIP file of Ubidots and then "**Accept**" or "**Choose**".
8. Close the Arduino IDE and open it again.

**NOTE:** To be able to use this device with TCP over TLS1.2 or HTTPS you must add manually the certificates from Ubidots to the module following the steps into the section **Certificate uploading** in this [guide](https://www.arduino.cc/en/Tutorial/FirmwareUpdater), further add the following domains required for the process in the *Add Domain* button from the *FirwareUpdater* provided by the MKR suit.

`industrial.api.ubidots.com:9812`
`industrial.api.ubidots.com:443`


# Documentation

## Constructor

### Ubidots

```
Ubidots(const char* token, UbiServer server, IotProtocol iot_protocol)
```

> @token, [Required]. Your Ubidots unique account [TOKEN](http://help.ubidots.com/user-guides/find-your-token-from-your-ubidots-account).  
> @server, [Optional], [Options] = [`UBI_INDUSTRIAL`, `UBI_EDUCATIONAL`], [Default] = `UBI_INDUSTRIAL`. The server to send data, set `UBI_EDUCATIONAL` if your account is educational type.  
> @iot_protocol, [Optional], [Options] = [`UBI_HTTP`, `UBI_TCP`, `UBI_UDP`], [Default] = `UBI_TCP`. The IoT protocol that you will use to send or retrieve data.

Creates an Ubidots instance.

**NOTE:** If you use HTTP or TCP to send data, the client will implement TLS 2.0 using the native MKR1010 secure client to secure your data. Keep in mind that due to the security checks needed, the packet may take a little more time to be sent than without TLS. If you wish to send data insecurely, use UDP or any of the available examples at our ![docs](https://ubidots.com/docs/hw/).

As Ubidots makes its best to secure your data, we do not guarantee any issue, data miss or external sniff coming from the native secure client or bugs in the library.

## Methods

```
void add(const char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis)
```

> @variable_label, [Required]. The label of the variable where the dot will be stored.
> @value, [Required]. The value of the dot.  
> @context, [Optional]. The dot's context.  
> @dot_timestamp_seconds, [Optional]. The dot's timestamp in seconds.  
> @dot_timestamp_millis, [Optional]. The dot's timestamp number of milliseconds. If the timestamp's milliseconds values is not set, the seconds will be multplied by 1000.

Adds a dot with its related value, context and timestamp to be sent to a certain data source, once you use add().

**Important:** The max payload lenght is 700 bytes, if your payload is greater it won't be properly sent. You can see on your serial console the payload to send if you call the `setDebug(bool debug)` method and pass a true value to it.

```
float get(const char* device_label, const char* variable_label)
```

> @device_label, [Required]. The device label which contains the variable to retrieve values from.  
> @variable_label, [Required]. The variable label to retrieve values from.

Returns as float the last value of the dot from the variable.
IotProtocol getCloudProtocol()

```
void addContext(char *key_label, char *key_value)
```

> @key_label, [Required]. The key context label to store values.  
> @key_value, [Required]. The key pair value.

Adds to local memory a new key-value context key. The method inputs must be char pointers. The method allows to store up to 10 key-value pairs.

```
void getContext(char *context)
```

> @context, [Required]. A char pointer where the context will be stored.

Builds the context according to the chosen protocol and stores it in the context char pointer.

```
void setDebug(bool debug)
```

> @debug, [Required]. Boolean type to turn off/on debug messages.

Makes available debug messages through the serial port.

```
bool send(const char* device_label, const char* device_name);
```

> @device_label, [Optional], [Default] = Device's MAC Address. The device label to send data. If not set, the device's MAC address will be used.  
> @device_name, [Optional], [Default] = @device_label. The device name otherwise assigned if the device doesn't already exist in your Ubidots account. If not set, the device label parameter will be used. 
> 
> **NOTE**: Device name is only supported through TCP/UDP, if you use another protocol, the device name will be the same as device label.  

Sends all the data added using the add() method. Returns true if the data was sent.


```
bool wifiConnect(const char* ssid, const char* password)
```

> @ssid, [Optional], [Required]. WiFi SSID to connect to name.  
> @password, [Required]. WiFi password credential.

Attempts to connect to the cloud using WiFi with the specified credentials.

```
bool wifiConnected()
```
Returns true if the device is connected to the cloud using WiFi.

```
bool serverConnected();
```
Returns true if the device has a socket opened with Ubidots.

```
void setDeviceType(const char* deviceType)
```

Sets a [device type](https://help.ubidots.com/en/articles/2129204-device-types) to be added in your request. This method works only if you set HTTP as iot protocol in your instance constructor.

# Examples

Refer to the [examples](https://github.com/ubidots/ubidots-ArduinoMKR/tree/master/examples) folder
