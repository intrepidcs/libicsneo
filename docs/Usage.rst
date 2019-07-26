****************
**API Usage**
****************

API Concepts
================

.. _events:

Events 
~~~~~~~~~~~~~~~~~~~~

Events convey information about the API's inner workings to the user. There are 3 severity levels: ``EventInfo``, ``EventWarning``, and ``Error``.
**However, the API treats events of severities** ``EventInfo`` **and** ``EventWarning`` **differently than those of severity** ``Error`` **.**
From here on out, when we (and the API functions) refer to "events", we refer exclusively to those of severities ``EventInfo`` and ``EventWarning``.
Those of severity ``Error`` are referred to as "errors", which use a separate errors_ system.

The API stores events in a single buffer that can has a default size of 10,000 events.
This limit includes 1 reserved slot at the end of the buffer for a potential Event of type ``TooManyEvents`` and severity ``EventWarning``, which is added when there are too many events for the buffer to hold.
This could occur if the events aren't read out by the user often enough, or if the user sets the size of the buffer to a value smaller than the number of existing events.
There will only ever be one of these ``TooManyEvents`` events, and it will always be located at the very end of the buffer if it exists.
Because of this reserved slot, the buffer by default is able to hold 9,999 events. If capacity is exceeded, the oldest events in the buffer are automatically discarded until the buffer is exactly at capacity again.
When events are read out by the user, they are removed from the buffer.

In a multithreaded environment, all threads will log their events to the same buffer. In this case, the order of events will largely be meaningless, although the behavior of ``TooManyEvents`` is still guaranteed to be as described above.

.. _errors:

Errors
~~~~~~~~~

The error system is threadsafe and separate from the events_ system. 
Each thread keeps track of the last error logged on it, and getting the last error will return the last error from the calling thread, removing it in the process.
Trying to get the last error when there is none will return an event of type ``NoErrorFound`` and severity ``EventInfo``.
Only the last error is stored, so make sure to read out the last error immediately after an API function fails!

The API also contains threads for internal use that are inaccessible to the user and may potentially log errors of their own. 
These threads have been marked to downgrade any errors to severity ``EventWarning`` and will log the corresponding event in the events_ system described above.

Device Concepts
================

Open/Close Status
~~~~~~~~~~~~~~~~~~~~~~~

In order to access device functionality, the device must first be opened. The exception to this is setting the message polling status of the device.
Trying to open/close the device when the device is already open/closed will result in an error being logged on the calling thread.

Online/Offline Status
~~~~~~~~~~~~~~~~~~~~~~~

The online/offline status of the device determines whether or not communication (such as sending messages) is available with the device. 
In order to be online, the device must also be open. Trying to go online/offline when the device is already online/offline will result in an error being logged on the calling thread.

It is possible to have a device be both open and offline. In this situation, device settings such as the baudrate may still be read and changed.
This is useful for setting up your device properly before going online and beginning communication.

Message Polling Status
~~~~~~~~~~~~~~~~~~~~~~~

The message polling status of the device controls whether or not the device stores a buffer of messages. The default limit of this buffer is 20,000 messages.
If the limit is exceeded, the oldest messages will be dropped until the buffer is at capacity, and an error will be logged on the calling thread.
To avoid exceeding the limit, try to get messages periodically, which will flush the buffer upon each call.
Attempting to read messages without first enabling message polling or registering a traffic handler will result in an error being logged on the calling thread.

Write Blocking Status
~~~~~~~~~~~~~~~~~~~~~~~
The write blocking status of the device determines the behavior of attempting to transmit to the device (likely via sending messages) with a large backlog of messages.
If write blocking is enabled, then the transmitting thread will wait for the entire buffer to be transmitted.
If write blocking is disabled, then the attempt to transmit will simply fail and an error will be logged on the calling thread.