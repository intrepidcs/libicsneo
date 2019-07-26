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

Online/Offline Status
~~~~~~~~~~~~~~~~~~~~~~~

Message Polling Status
~~~~~~~~~~~~~~~~~~~~~~~

Write Blocking Status
~~~~~~~~~~~~~~~~~~~~~~~
