****************
**API Usage**
****************

API Concepts
================

Overview
~~~~~~~~~~~~~~~~~~~~

Events convey information about the API's inner workings to the user. There are 3 severity levels: ``EventInfo``, ``EventWarning``, and ``Error``.
**However, the API treats events of severities** ``EventInfo`` **and** ``EventWarning`` **differently than those of severity** ``Error`` **.**
From here on out, when we (and the API functions) refer to "events", we refer exclusively to those of severities ``EventInfo`` and ``EventWarning``, which use the events_ system.
Those of severity ``Error`` are referred to as "errors", which use a separate errors_ system.

Events should periodically be read out in order to avoid overflowing, and the last error should be read out immediately after an API function fails.

Additionally, `event callbacks`_ can be registered, which may remove the need to periodically read events in some cases.

.. _events:

Events
~~~~~~~~~~~~~~~~~~~~

The API stores events in a single buffer that can has a default size of 10,000 events.
This limit includes 1 reserved slot at the end of the buffer for a potential Event of type ``TooManyEvents`` and severity ``EventWarning``, which is added when there are too many events for the buffer to hold.
This could occur if the events aren't read out by the user often enough, or if the user sets the size of the buffer to a value smaller than the number of existing events.
There will only ever be one of these ``TooManyEvents`` events, and it will always be located at the very end of the buffer if it exists.
Because of this reserved slot, the buffer by default is able to hold 9,999 events. If capacity is exceeded, the oldest events in the buffer are automatically discarded until the buffer is exactly at capacity again.
When events are read out by the user, they are removed from the buffer. If an event filter is used, only the filtered events will be removed from the buffer.

In a multithreaded environment, all threads will log their events to the same buffer. In this case, the order of events will largely be meaningless, although the behavior of ``TooManyEvents`` is still guaranteed to be as described above.

.. _event callbacks:

Event Callbacks
~~~~~~~~~~~~~~~~~~~~

Users may register event callbacks, which are automatically called whenever a matching event is logged.
Message callbacks consist of a user-defined ``std::function< void( std::shared_ptr<APIEvent> ) >`` and optional EventFilter used for matching.
If no EventFilter is provided, the default-constructed one will be used, which matches any event.
Registering a callback returns an ``int`` representing the id of the callback, which should be stored by the user and later used to remove the callback when desired.
Note that this functionality is only available in C and C++. C does not currently support filters.

Event callbacks are run after the event has been added to the buffer of events. The buffer of events may be safely modified within the callback, such as getting (flushing) the type and severity of the triggering event.
Using event callbacks in this manner means that periodically reading events is unnecessary.

.. _errors:

Errors
~~~~~~~~~

The error system is threadsafe and separate from the events_ system.
Each thread keeps track of the last error logged on it, and getting the last error will return the last error from the calling thread, removing it in the process.
Trying to get the last error when there is none will return an event of type ``NoErrorFound`` and severity ``EventInfo``.

The API also contains some threads for internal use which may potentially log errors of their own and are inaccessible to the user.
These threads have been marked to downgrade any errors that occur on them to severity ``EventWarning`` and will log the corresponding event in the events_ system described above.

Device Concepts
================

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
