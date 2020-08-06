using System.Collections.Generic;

namespace libicsneocsharp_example {
    class InteractiveExample {

        private uint msgLimit = 50000;
        private neodevice_t selectedDevice;
        private List<neodevice_t> devices = new List<neodevice_t>();
        private uint numDevices = 0;

        private void PrintAllDevices() {
            if(numDevices == 0) {
                System.Console.WriteLine("No devices found! Please scan for new devices.");
            }

            for(int i = 0; i < numDevices; i++) {
                System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                if(icsneocsharp.icsneo_describeDevice(devices[i], description, ref maxLength)) {
                    System.Console.Write("[" + (i + 1) + "] " + description.ToString() + "\tConnected: ");

                    if(icsneocsharp.icsneo_isOpen(devices[i])) {
                        System.Console.Write("Yes\t");
                    } else System.Console.Write("No\t");

                    System.Console.Write("Online: ");
                    if(icsneocsharp.icsneo_isOnline(devices[i])) {
                        System.Console.Write("Yes\t");
                    } else System.Console.Write("No\t");

                    System.Console.Write("Msg Polling: ");
                    if(icsneocsharp.icsneo_isMessagePollingEnabled(devices[i])) {
                        System.Console.Write("On\n");
                    } else System.Console.Write("Off\n");
                } else {
                    System.Console.WriteLine("Description for device " + (i + 1) + " not available!");
                }
            }
        }

        private uint ScanNewDevices() {
            neodevice_t newDevices = icsneocsharp.new_neodevice_t_array(99);

            int count = 10;

            icsneocsharp.icsneo_findAllDevices(newDevices, ref count);

            numDevices += (uint) count;

            for(int i = 0; i < numDevices; i++) {
                devices.Add(icsneocsharp.neodevice_t_array_getitem(newDevices, i));
            }
            icsneocsharp.delete_neodevice_t_array(newDevices);
            return (uint) count;
        }

        private void PrintMainMenu() {
            System.Console.WriteLine("Press the letter next to the function you want to use:");
            System.Console.WriteLine("A - List all devices");
            System.Console.WriteLine("B - Scan for new devices");
            System.Console.WriteLine("C - Open/close");
            System.Console.WriteLine("D - Go online/offline");
            System.Console.WriteLine("E - Enable/disable message polling");
            System.Console.WriteLine("F - Get messages");
            System.Console.WriteLine("G - Send message");
            System.Console.WriteLine("H - Get events");
            System.Console.WriteLine("I - Set HS CAN to 250K");
            System.Console.WriteLine("J - Set HS CAN to 500K");
            System.Console.WriteLine("X - Exit");
        }

        void PrintLastError() {
            neoevent_t error = new neoevent_t();
            if(icsneocsharp.icsneo_getLastError(error))
                System.Console.WriteLine("Error 0x" + error.eventNumber + ": " + error.description);
            else
                System.Console.WriteLine("No errors found!");
        }

        void PrintAPIEvents() {
            neoevent_t events = icsneocsharp.new_neoevent_t_array(99);
            int eventCount = 99;
            if(icsneocsharp.icsneo_getEvents(events, ref eventCount)) {
                if(eventCount == 1) {
                    neoevent_t evt = icsneocsharp.neoevent_t_array_getitem(events, 0);
                    System.Console.WriteLine("1 API event found!");
                    System.Console.WriteLine("Event 0x" + evt.eventNumber + ": " + evt.description);
                } else {
                    System.Console.WriteLine(eventCount + " API events found!");
                    for(var i = 0; i < eventCount; ++i) {
                        neoevent_t evt = icsneocsharp.neoevent_t_array_getitem(events, i);
                        System.Console.WriteLine("Event 0x" + evt.eventNumber + ": " + evt.description);
                    }
                }
            } else {
                System.Console.WriteLine("Failed to get API events!");
            }
            icsneocsharp.delete_neoevent_t_array(events);
        }

        void PrintDeviceEvents(neodevice_t device) {
            neoevent_t events = icsneocsharp.new_neoevent_t_array(99);
            int eventCount = 99;
            if(icsneocsharp.icsneo_getDeviceEvents(device, events, ref eventCount)) {
                if(eventCount == 1) {
                    neoevent_t evt = icsneocsharp.neoevent_t_array_getitem(events, 0);
                    System.Console.WriteLine("1 device event found!");
                    System.Console.WriteLine("Event 0x" + evt.eventNumber + ": " + evt.description);
                } else {
                    System.Console.WriteLine(eventCount + " device events found!");
                    for(var i = 0; i < eventCount; ++i) {
                        neoevent_t evt = icsneocsharp.neoevent_t_array_getitem(events, i);
                        System.Console.WriteLine("Event 0x" + evt.eventNumber + ": " + evt.description);
                    }
                }
            } else {
                System.Console.WriteLine("Failed to get API events!");
            }
            icsneocsharp.delete_neoevent_t_array(events);
        }

        private char GetCharInput(List<char> allowed) {
            bool found = false;
            char key = '0';
            while(!found) {
                key = System.Console.ReadKey().KeyChar;
                System.Console.WriteLine();
                foreach(char compare in allowed) {
                    if(compare == key) {
                        found = true;
                        break;
                    }
                }

                if(!found) {
                    System.Console.WriteLine("Input did not match expected options. Please try again.");
                }
            }
            return key;
        }

        neodevice_t SelectDevice() {
            System.Console.WriteLine("Please select a device:");
            PrintAllDevices();
            System.Console.WriteLine();

            int selectedDeviceNum = 10;
            while(selectedDeviceNum > numDevices) {
                char deviceSelection = GetCharInput(new List<char> { '1', '2', '3', '4', '5', '6', '7', '8', '9' });
                selectedDeviceNum = (int)char.GetNumericValue(deviceSelection);
                if(selectedDeviceNum > numDevices) {
                    System.Console.WriteLine("Selected device out of range!");
                }
            }

            System.Console.WriteLine();

            return devices[selectedDeviceNum - 1];
        }

        public void Run() {
            neoversion_t version = icsneocsharp.icsneo_getVersion();
            System.Console.WriteLine("ICS icsneocsharp.dll version " + version.major + "." + version.minor + "." + version.patch);
            System.Console.WriteLine();

            while(true) {
                PrintMainMenu();
                System.Console.WriteLine();
                char input = GetCharInput(new List<char> { 'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'X', 'x' });
                System.Console.WriteLine();
                switch(input) {
                // List current devices
                case 'A':
                    goto case 'a';
                case 'a':
                    PrintAllDevices();
                    System.Console.WriteLine();
                    break;
                // Scan for new devices
                case 'B':
                    goto case 'b';
                case 'b': {
                    var numNewDevices = ScanNewDevices();
                    if(numNewDevices == 1) {
                        System.Console.WriteLine("1 new device found!");
                    } else {
                        System.Console.WriteLine(numNewDevices + " new devices found!");
                    }
                    PrintAllDevices();
                    System.Console.WriteLine();
                    break;
                }
                // Open/close a device
                case 'C':
                    goto case 'c';
                case 'c': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    System.Console.WriteLine("Would you like to open or close " + description + "?");
                    System.Console.WriteLine("[1] Open\n[2] Close\n[3] Cancel\n");

                    char option = GetCharInput(new List<char> { '1', '2', '3' });
                    System.Console.WriteLine();

                    switch(option) {
                    case '1':
                        // Attempt to open the selected device
                        if(icsneocsharp.icsneo_openDevice(selectedDevice)) {
                            System.Console.WriteLine(description + " successfully opened!\n");
                        } else {
                            System.Console.WriteLine(description + " failed to open!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    case '2':
                        // Attempt to close the device
                        if(icsneocsharp.icsneo_closeDevice(selectedDevice)) {
                            numDevices--;
                            System.Console.WriteLine("Successfully closed " + description + "!\n");
                            devices.Remove(selectedDevice);
                            selectedDevice = null;
                        } else {
                            System.Console.WriteLine("Failed to close " + description.ToString() + "!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    default:
                        System.Console.WriteLine("Canceling!\n");
                        break;
                    }
                    break;
                }
                // Go online/offline
                case 'D':
                    goto case 'd';
                case 'd': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    System.Console.WriteLine("Would you like to have " + description.ToString() + " go online or offline?");
                    System.Console.WriteLine("[1] Online\n[2] Offline\n[3] Cancel\n");

                    char option = GetCharInput(new List<char> { '1', '2', '3' });
                    System.Console.WriteLine();

                    switch(option) {
                    case '1':
                        // Attempt to go online
                        if(icsneocsharp.icsneo_goOnline(selectedDevice)) {
                            System.Console.WriteLine(description + " successfully went online!\n");
                        } else {
                            System.Console.WriteLine(description + " failed to go online!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    case '2':
                        // Attempt to go offline
                        if(icsneocsharp.icsneo_goOffline(selectedDevice)) {
                            System.Console.WriteLine(description + " successfully went offline!\n");
                        } else {
                            System.Console.WriteLine(description + " failed to go offline!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    default:
                        System.Console.WriteLine("Canceling!\n");
                        break;
                    }
                    break;
                }
                // Enable/disable message polling
                case 'E':
                    goto case 'e';
                case 'e': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    System.Console.WriteLine("Would you like to enable or disable message polling for " + description.ToString() + "?");
                    System.Console.WriteLine("[1] Enable\n[2] Disable\n[3] Cancel\n");

                    char option = GetCharInput(new List<char> { '1', '2', '3' });
                    System.Console.WriteLine();

                    switch(option) {
                    case '1':
                        // Attempt to enable message polling
                        if(icsneocsharp.icsneo_enableMessagePolling(selectedDevice)) {
                            System.Console.WriteLine("Successfully enabled message polling for " + description.ToString() + "!\n");
                        } else {
                            System.Console.WriteLine("Failed to enable message polling for " + description.ToString() + "!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }

                        // Manually setting the polling message limit as done below is optional
                        // It will default to 20k if not set
                        // Attempt to set the polling message limit
                        if(icsneocsharp.icsneo_setPollingMessageLimit(selectedDevice, msgLimit)) {
                            System.Console.WriteLine("Successfully set message polling limit for " + description.ToString() + "!\n");
                        } else {
                            System.Console.WriteLine("Failed to set polling message limit for " + description.ToString() + "!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    case '2':
                        // Attempt to disable message polling
                        if(icsneocsharp.icsneo_disableMessagePolling(selectedDevice)) {
                            System.Console.WriteLine("Successfully disabled message polling for " + description.ToString() + "!\n");
                        } else {
                            System.Console.WriteLine("Failed to disable message polling for " + description.ToString() + "!\n");
                            PrintLastError();
                            System.Console.WriteLine();
                        }
                        break;
                    default:
                        System.Console.WriteLine("Canceling!\n");
                        break;
                    }
                }
                break;
                case 'F':
                    goto case 'f';
                case 'f': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    // Prepare the neomessage_t array and size for reading in the messages
                    neomessage_t msgs = icsneocsharp.new_neomessage_t_array((int)msgLimit);
                    int msgCount = (int)msgLimit;

                    if(!icsneocsharp.icsneo_getMessages(selectedDevice, msgs, ref msgCount, 0)) {
                        System.Console.WriteLine("Failed to get messages for " + description.ToString() + "!\n");
                        PrintLastError();
                        icsneocsharp.delete_neomessage_t_array(msgs);
                        System.Console.WriteLine();
                        break;
                    }

                    if(msgCount == 1) {
                        System.Console.WriteLine("1 message received from " + description.ToString() + "!");
                    } else {
                        System.Console.WriteLine(msgCount + " messages received from " + description.ToString() + "!");
                    }

                    // Print out the received messages
                    for(int i = 0; i < msgCount; i++) {
                        neomessage_t msg = icsneocsharp.neomessage_t_array_getitem(msgs, i);
                        if(msg.type == icsneocsharp.ICSNEO_NETWORK_TYPE_CAN) {
                            System.Console.Write("\t0x" + "{0:x}" + " [" + msg.length + "] ", icsneocsharp.neomessage_can_t_cast(msg).arbid);
                            for(int j = 0; j < msg.length; j++) {
                                System.Console.Write("{0:x} ", icsneocsharp.neomessage_can_t_cast(msg).data[j]);
                            }
                            System.Console.WriteLine("(" + msg.timestamp + ")");
                        } else {
                            if(msg.netid != 0)
                                System.Console.WriteLine("\tMessage on netid " + msg.netid + " with length " + msg.length);
                        }
                    }
                    icsneocsharp.delete_neomessage_t_array(msgs);
                    break;
                }
                // Send message
                case 'G':
                    goto case 'g';
                case 'g': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    // Start generating sample msg
                    neomessage_can_t msg = new neomessage_can_t();
                    msg.arbid = 0x120;
                    msg.length = 6;
                    msg.netid = (ushort)icsneocsharp.ICSNEO_NETID_HSCAN;
                    msg.data = new byte[6] { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
                    msg.status.canfdFDF = 0;
                    msg.status.extendedFrame = 0;
                    msg.status.canfdBRS = 0;
                    // End generating sample msg

                    // Attempt to transmit the sample msg
                    if(icsneocsharp.icsneo_transmit(selectedDevice, icsneocsharp.from_can_neomessage_t_cast(msg))) {
                        System.Console.WriteLine("Message transmit successful!");
                    } else {
                        System.Console.WriteLine("Failed to transmit message to " + description.ToString() + "!\n");
                        PrintLastError();
                        System.Console.WriteLine();
                    }
                    break;
                }
                // Get events
                case 'H':
                    goto case 'h';
                case 'h':
                    PrintAPIEvents();
                    System.Console.WriteLine();
                    break;
                // Set HS CAN to 250k
                case 'I':
                    goto case 'i';
                case 'i': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    // Attempt to set baudrate and apply settings
                    if(icsneocsharp.icsneo_setBaudrate(selectedDevice, (ushort)icsneocsharp.ICSNEO_NETID_HSCAN, 250000) && icsneocsharp.icsneo_settingsApply(selectedDevice)) {
                        System.Console.WriteLine("Successfully set HS CAN baudrate for " + description.ToString() + "to 250k!\n");
                    } else {
                        System.Console.WriteLine("Failed to set HS CAN for " + description.ToString() + " to 250k!\n");
                        PrintLastError();
                        System.Console.WriteLine();
                    }
                    break;
                }
                // Set HS CAN to 500k
                case 'J':
                    goto case 'j';
                case 'j': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.Console.WriteLine("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = SelectDevice();

                    // Get the product description for the device
                    System.Text.StringBuilder description = new System.Text.StringBuilder(icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int maxLength = icsneocsharp.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

                    icsneocsharp.icsneo_describeDevice(selectedDevice, description, ref maxLength);

                    // Attempt to set baudrate and apply settings
                    if(icsneocsharp.icsneo_setBaudrate(selectedDevice, (ushort)icsneocsharp.ICSNEO_NETID_HSCAN, 500000) && icsneocsharp.icsneo_settingsApply(selectedDevice)) {
                        System.Console.WriteLine("Successfully set HS CAN baudrate for " + description.ToString() + "to 500k!\n");
                    } else {
                        System.Console.WriteLine("Failed to set HS CAN for " + description.ToString() + " to 500k!\n");
                        PrintLastError();
                        System.Console.WriteLine();
                    }
                    break;
                }
                case 'X':
                    goto case 'x';
                case 'x':
                    System.Console.WriteLine("Exiting program");
                    return;
                default:
                    System.Console.WriteLine("Unexpected input, exiting!");
                    return;
                }
            }
        }
    }
}