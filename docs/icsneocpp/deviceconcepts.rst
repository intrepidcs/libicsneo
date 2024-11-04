===============
Device Concepts
===============

Open/Close Status
~~~~~~~~~~~~~~~~~~~~~~~

In order to access device functionality, the device must first be opened, which begins communication between the API and the device.
The exception to this is setting the message polling status of the device.
Trying to open/close the device when the device is already open/closed will result in an error being logged on the calling thread.

Online/Offline Status
~~~~~~~~~~~~~~~~~~~~~~~

Going online begins communication between the device and the rest of the network. In order to be online, the device must also be open.
Trying to go online/offline when the device is already online/offline will result in an error being logged on the calling thread.

It is possible to have a device be both open and offline. In this situation, device settings such as the baudrate may still be read and changed.
This is useful for setting up your device properly before going online and joining the network.

Message Callbacks and Polling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to handle messages, users may register message callbacks, which are automatically called whenever a matching message is received.
Message callbacks consist of a user-defined ``std::function< void( std::shared_ptr<Message> ) >`` and optional message filter used for matching.
If no message filter is provided, the default-constructed one will be used, which matches any message.
Registering a callback returns an ``int`` representing the id of the callback, which should be stored by the user and later used to remove the callback when desired.
Note that this functionality is only available in C and C++. C does not currently support filters.

The default method of handling messages is to enable message polling, which is built upon message callbacks.
Enabling message polling will register a callback that stores each received message in a buffer for later retrieval.
The default limit of this buffer is 20,000 messages.
If the limit is exceeded, the oldest messages will be dropped until the buffer is at capacity, and an error will be logged on the calling thread.
To avoid exceeding the limit, try to get messages periodically, which will flush the buffer upon each call.
Attempting to read messages without first enabling message polling will result in an error being logged on the calling thread.

It is recommended to either enable message polling or manually register callbacks to handle messages, but not both.

Write Blocking Status
~~~~~~~~~~~~~~~~~~~~~~~

The write blocking status of the device determines the behavior of attempting to transmit to the device (likely via sending messages) with a large backlog of messages.
If write blocking is enabled, then the transmitting thread will wait for the entire buffer to be transmitted.
If write blocking is disabled, then the attempt to transmit will simply fail and an error will be logged on the calling thread.

A2B message channel indexing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The libicsneo API allows users to input and output WAV file via receiving or transmitting A2B messages. The library provides several ways to import and export
WAV files via A2B traffic. While using the API, a user will encounter message channels being referenced as unsigned integers for indexing. A ``icsneo::A2BMessage``
object will contain both upstream and downstream channels. This implies that the number of channels that a A2B message has is twice the TDM mode. The ordering for these
channels are interleved. Therefore, a message channel index ``0`` would represent downstream channel ``0`` in the network, message channel index ``1`` would represent
upstream channel ``0`` in the network, and so on. More generally, a message channel index can be computed with the formula ``2 * CHANNEL + IS_UPSTREAM`` where channel
is the channel referred in the A2B network and ``IS_UPSTREAM`` is ``1`` when a channel is upstream and ``0`` if downstream.

One area where these message channel indexes are used are when specifying a ``icsneo::ChannelMap`` for WAV transmit or receive. When transmitting a WAV file, the
map will map message channels to the input WAV file. For receiving a WAV file, the map will map the output WAV channels to channels from received messages.

EX: If we want to transmit, we will need to construct a mapping from message channels to the input WAV file. So, if we have a monotone WAV file that we are inputting
through the API, then we can map channel ``2`` upstream to channel 0 in the input WAV (the only channel in the WAV file) with the following:

``icsneo::ChannelMap chMap``
``chMap[5] = 0``

Since we are transmitting, we must map our desired A2B message channels to the input WAV file. We have ``0`` representing the single channel in the WAV file and
``5`` representing channel ``2`` upstream in the A2B message from using the formula above ``2 * CHANNEL + IS_UPSTREAM = 2 * 2 + 1 = 5``.
