============
C++ Examples
============

A variety of examples can be found within ``examples/cpp``, see below for an
example that uses the polling API to receive CAN frames.

The C++ API is designed to be modern and easy to use. All library functions and
classes are in the namespace ``icsneo``. Most applications will start by calling
``icsneo::FindAllDevices()``. This will return an ``std::vector`` of
``std::shared_ptr<icsneo::Device>`` objects. You will want to keep a copy of the
``shared_ptr`` to any devices you want to use, as allowing it to go out of scope
will automatically close the device and free all memory associated with it.

Any time you get bus traffic from the API, you will receive it as an
``std::shared_ptr<icsneo::Message>``. The message will be valid as long as the
``shared_ptr`` stays in scope. Checking the type of the message allows you to
cast it accordingly and access extra data for certain protocols. For instance,
casting an ``icsneo::Message`` to an ``icsneo::CANMessage`` allows you to access
the arbitration ID.

.. code-block:: cpp

   std::vector<std::shared_ptr<icsneo::Device>> devices = icsneo::FindAllDevices();
   std::cout << devices.size() << " found!" << std::endl;
   for(auto& device : devices)
       std::cout << "Found " << device->describe() << std::endl; // "Found neoVI FIRE 2 CY2345"
   std::shared_ptr<icsneo::Device> myDevice = devices[0];
   if(!myDevice->open()) // Device tried and failed to open, print the last error
       std::cout << icsneo::GetLastError() << std::endl;
   myDevice->goOnline(); // Start receiving messages
   myDevice->enableMessagePolling(); // Allow the use of myDevice->getMessages() later
   // Alternatively, assign a callback for new messages
   std::this_thread::wait_for(std::chrono::seconds(5));
   std::vector<std::shared_ptr<icsneo::Message>> messages = myDevice->getMessages();
   std::cout << "We got " << messages.size() << " messages!" << std::endl;
   for(auto& msg : messages) {
       switch(msg->network.getType()) {
           case icsneo::Network::Type::CAN:
           case icsneo::Network::Type::SWCAN:
           case icsneo::Network::Type::LSFTCAN: {
               // A message of type CAN is guaranteed to be a CANMessage, so we can static cast safely
               auto canmsg = std::static_pointer_cast<icsneo::CANMessage>(msg);
               // canmsg->arbid is valid here
               // canmsg->data is an std::vector<uint8_t>, you can check .size() for the DLC of the message
               // canmsg->timestamp is the time recorded by the hardware in nanoseconds since (1/1/2007 12:00:00 GMT)
           }
           default:
               // Handle others
       }
   }
   myDevice->close();
