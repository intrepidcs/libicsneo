==========
C Examples
==========

A variety of examples can be found within ``examples/c``, see below for an
example that uses the polling API to receive CAN frames.

.. code-block:: c

   size_t deviceCount = 10; // Pre-set to the size of your buffer before the icsneo_findAllDevices() call
   neodevice_t devices[10];
   icsneo_findAllDevices(devices, &deviceCount);
   printf("We found %ull devices\n", deviceCount);
   for(size_t i = 0; i < deviceCount; i++) {
       neodevice_t* myDevice = &devices[i];
       char desc[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION];
       size_t sz = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
       icsneo_describeDevice(myDevice, desc, &sz);
       printf("Found %s\n", desc); // "Found neoVI FIRE 2 CY2345"
   }
   neodevice_t* myDevice = &devices[0];
   if(!icsneo_openDevice(myDevice)) {
       neoevent_t error;
       if(icsneo_getLastError(&error))
           printf("Error! %s\n", error.description);
   }
   icsneo_goOnline(myDevice); // Start receiving messages
   icsneo_enableMessagePolling(myDevice); // Allow the use of icsneo_getMessages() later
   sleep(5);
   neomessage_t messages[50];
   size_t messageCount = 50;
   icsneo_getMessages(myDevice, messages, &messageCount, 0 /* non-blocking */);
   printf("We got %ull messages!\n", messageCount);
   for(size_t i = 0; i < messageCount; i++) {
       if(messages[i].type == ICSNEO_NETWORK_TYPE_CAN) {
           // A message of type CAN should be interperated a neomessage_can_t, so we can cast safely
           neomessage_can_t* canmsg = (neomessage_can_t*)&messages[i];
           // canmsg->arbid is valid here
           // canmsg->data is an uint8_t*, you can check canmsg->length for the length of the payload
           // canmsg->timestamp is the time recorded by the hardware in nanoseconds since (1/1/2007 12:00:00 GMT)
       }
   }
   icsneo_closeDevice(myDevice);