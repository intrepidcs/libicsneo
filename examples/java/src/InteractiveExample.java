import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Scanner;

public class InteractiveExample {

    private int msgLimit = 50000;
    private neodevice_t selectedDevice;
    private ArrayList<neodevice_t> devices = new ArrayList<neodevice_t>();
    private int numDevices = 0;

    private void printAllDevices() {
        if(numDevices == 0) {
            System.out.println("No devices found! Please scan for new devices.");
        }

        for(int i = 0; i < numDevices; i++) {
            StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
            int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

            if(icsneojava.icsneo_describeDevice(devices.get(i), description, maxLength)) {
                System.out.print("[" + (i + 1) + "] " + description + "\tConnected: ");

                if(icsneojava.icsneo_isOpen(devices.get(i))) {
                    System.out.print("Yes\t");
                } else System.out.print("No\t");

                System.out.print("Online: ");
                if(icsneojava.icsneo_isOnline(devices.get(i))) {
                    System.out.print("Yes\t");
                } else System.out.print("No\t");

                System.out.print("Msg Polling: ");
                if(icsneojava.icsneo_isMessagePollingEnabled(devices.get(i))) {
                    System.out.print("On\n");
                } else System.out.print("Off\n");
            } else {
                System.out.println("Description for device " + (i + 1) + " not available!");
            }
        }
    }

    private int scanNewDevices() {
        neodevice_t newDevices = icsneojava.new_neodevice_t_array(99);

        int[] count = {10};

        icsneojava.icsneo_findAllDevices(newDevices, count);

        numDevices += count[0];

        for(int i = 0; i < numDevices; i++) {
            devices.add(icsneojava.neodevice_t_array_getitem(newDevices, i));
        }

        icsneojava.delete_neodevice_t_array(newDevices);
        return count[0];
    }

    private void printMainMenu() {
        System.out.println("Press the letter next to the function you want to use:");
        System.out.println("A - List all devices");
        System.out.println("B - Scan for new devices");
        System.out.println("C - Open/close");
        System.out.println("D - Go online/offline");
        System.out.println("E - Enable/disable message polling");
        System.out.println("F - Get messages");
        System.out.println("G - Send message");
        System.out.println("H - Get events");
        System.out.println("I - Set HS CAN to 250K");
        System.out.println("J - Set HS CAN to 500K");
        System.out.println("X - Exit");
    }

    private void printLastError() {
        neoevent_t error = new neoevent_t();
        if(icsneojava.icsneo_getLastError(error))
            System.out.println("Error 0x" + error.getEventNumber() + ": " + error.getDescription());
        else
            System.out.println("No errors found!");
    }

    private void printAPIEvents() {
        neoevent_t events = icsneojava.new_neoevent_t_array(99);
        int[] eventCount = {99};
        if(icsneojava.icsneo_getEvents(events, eventCount)) {
            if(eventCount[0] == 1) {
                neoevent_t evt = icsneojava.neoevent_t_array_getitem(events, 0);
                System.out.println("1 API event found!");
                System.out.println("Event 0x" + evt.getEventNumber() + ": " + evt.getDescription());
            } else {
                System.out.println(eventCount[0] + " API events found!");
                for(var i = 0; i < eventCount[0]; ++i) {
                    neoevent_t evt = icsneojava.neoevent_t_array_getitem(events, i);
                    System.out.println("Event 0x" + evt.getEventNumber() + ": " + evt.getDescription());
                }
            }
        } else {
            System.out.println("Failed to get API events!");
        }
        icsneojava.delete_neoevent_t_array(events);
    }

    private void printDeviceEvents(neodevice_t device) {
        neoevent_t events = icsneojava.new_neoevent_t_array(99);
        int[] eventCount = {99};
        if(icsneojava.icsneo_getDeviceEvents(device, events, eventCount)) {
             if(eventCount[0] == 1) {
                neoevent_t evt = icsneojava.neoevent_t_array_getitem(events, 0);
                System.out.println("1 device event found!");
                System.out.println("Event 0x" + evt.getEventNumber() + ": " + evt.getDescription());
            } else {
                System.out.println(eventCount[0] + " device events found!");
                for(int i = 0; i < eventCount[0]; ++i) {
                    neoevent_t evt = icsneojava.neoevent_t_array_getitem(events, i);
                    System.out.println("Event 0x" + evt.getEventNumber() + ": " + evt.getDescription());
                }
            }
        } else {
            System.out.println("Failed to get API events!");
        }
        icsneojava.delete_neoevent_t_array(events);
    }

    private char getCharInput(char[] allowed) {
        boolean found = false;
        char key = '0';
        Scanner sc = new Scanner(System.in);
        while(!found) {
            key = sc.next().charAt(0);
            System.out.println();
            for(char compare : allowed) {
                if(compare == key) {
                    found = true;
                    break;
                }
            }

            if(!found) {
                System.out.println("Input did not match expected options. Please try again.");
            }
        }
        return key;
    }

    private neodevice_t selectDevice() {
        System.out.println("Please select a device:");
        printAllDevices();
        System.out.println();

        int selectedDeviceNum = 10;
        while(selectedDeviceNum > numDevices) {
            char deviceSelection = getCharInput(new char[] { '1', '2', '3', '4', '5', '6', '7', '8', '9' });
            selectedDeviceNum = deviceSelection - '0';
            if(selectedDeviceNum > numDevices) {
                System.out.println("Selected device out of range!");
            }
        }

        System.out.println();

        return devices.get(selectedDeviceNum - 1);
    }

    public void run() {
        neoversion_t version = icsneojava.icsneo_getVersion();
        System.out.println("ICS libicsneojava.dll version " + version.getMajor() + "." + version.getMinor() + "." + version.getPatch());
        System.out.println();

        while(true) {
            printMainMenu();
            System.out.println();
            char input = getCharInput(new char[] { 'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'X', 'x' });
            System.out.println();
            switch(input) {
                // List current devices
                case 'A':
                case 'a':
                    printAllDevices();
                    System.out.println();
                    break;
                // Scan for new devices
                case 'B':
                case 'b': {
                    var numNewDevices = scanNewDevices();
                    if(numNewDevices == 1) {
                        System.out.println("1 new device found!");
                    } else {
                        System.out.println(numNewDevices + " new devices found!");
                    }
                    printAllDevices();
                    System.out.println();
                    break;
                }
                // Open/close a device
                case 'C':
                case 'c': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    System.out.println("Would you like to open or close " + description + "?");
                    System.out.println("[1] Open\n[2] Close\n[3] Cancel\n");

                    char option = getCharInput(new char[] { '1', '2', '3' });
                    System.out.println();

                    switch(option) {
                        case '1':
                            // Attempt to open the selected device
                            if(icsneojava.icsneo_openDevice(selectedDevice)) {
                                System.out.println(description + " successfully opened!\n");
                            } else {
                                System.out.println(description + " failed to open!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        case '2':
                            // Attempt to close the device
                            if(icsneojava.icsneo_closeDevice(selectedDevice)) {
                                numDevices--;
                                System.out.println("Successfully closed " + description + "!\n");
                                devices.remove(selectedDevice);
                                selectedDevice = null;
                            } else {
                                System.out.println("Failed to close " + description + "!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        default:
                            System.out.println("Canceling!\n");
                            break;
                    }
                    break;
                }
                // Go online/offline
                case 'D':
                case 'd': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    System.out.println("Would you like to have " + description + " go online or offline?");
                    System.out.println("[1] Online\n[2] Offline\n[3] Cancel\n");

                    char option = getCharInput(new char[] { '1', '2', '3' });
                    System.out.println();

                    switch(option) {
                        case '1':
                            // Attempt to go online
                            if(icsneojava.icsneo_goOnline(selectedDevice)) {
                                System.out.println(description + " successfully went online!\n");
                            } else {
                                System.out.println(description + " failed to go online!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        case '2':
                            // Attempt to go offline
                            if(icsneojava.icsneo_goOffline(selectedDevice)) {
                                System.out.println(description + " successfully went offline!\n");
                            } else {
                                System.out.println(description + " failed to go offline!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        default:
                            System.out.println("Canceling!\n");
                            break;
                    }
                    break;
                }
                // Enable/disable message polling
                case 'E':
                case 'e': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    System.out.println("Would you like to enable or disable message polling for " + description + "?");
                    System.out.println("[1] Enable\n[2] Disable\n[3] Cancel\n");

                    char option = getCharInput(new char[] { '1', '2', '3' });
                    System.out.println();

                    switch(option) {
                        case '1':
                            // Attempt to enable message polling
                            if(icsneojava.icsneo_enableMessagePolling(selectedDevice)) {
                                System.out.println("Successfully enabled message polling for " + description + "!\n");
                            } else {
                                System.out.println("Failed to enable message polling for " + description + "!\n");
                                printLastError();
                                System.out.println();
                            }

                            // Manually setting the polling message limit as done below is optional
                            // It will default to 20k if not set
                            // Attempt to set the polling message limit
                            if(icsneojava.icsneo_setPollingMessageLimit(selectedDevice, msgLimit)) {
                                System.out.println("Successfully set message polling limit for " + description + "!\n");
                            } else {
                                System.out.println("Failed to set polling message limit for " + description + "!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        case '2':
                            // Attempt to disable message polling
                            if(icsneojava.icsneo_disableMessagePolling(selectedDevice)) {
                                System.out.println("Successfully disabled message polling for " + description + "!\n");
                            } else {
                                System.out.println("Failed to disable message polling for " + description + "!\n");
                                printLastError();
                                System.out.println();
                            }
                            break;
                        default:
                            System.out.println("Canceling!\n");
                            break;
                    }
                }
                break;
                case 'F':
                case 'f': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    // Prepare the neomessage_t array and size for reading in the messages
                    neomessage_t msgs = icsneojava.new_neomessage_t_array((int)msgLimit);
                    int[] msgCount = {msgLimit};

                    if(!icsneojava.icsneo_getMessages(selectedDevice, msgs, msgCount, BigInteger.ZERO)) {
                        System.out.println("Failed to get messages for " + description + "!\n");
                        icsneojava.delete_neomessage_t_array(msgs);
                        printLastError();
                        System.out.println();
                        break;
                    }

                    if(msgCount[0] == 1) {
                        System.out.println("1 message received from " + description + "!");
                    } else {
                        System.out.println(msgCount[0] + " messages received from " + description + "!");
                    }

                    // Print out the received messages
                    for(int i = 0; i < msgCount[0]; i++) {
                        neomessage_t msg = icsneojava.neomessage_t_array_getitem(msgs, i);
                        if(msg.getType() == icsneojava.ICSNEO_NETWORK_TYPE_CAN) {
                            System.out.print("\t0x" + String.format("%03x", icsneojava.neomessage_can_t_cast(msg).getArbid()) + " [" + msg.getLength() + "] ");
                            for(int j = 0; j < msg.getLength(); j++) {
                                System.out.print(String.format("%02x ", icsneojava.neomessage_can_t_cast(msg).getData()[j]));
                            }
                            System.out.println("(" + msg.getTimestamp() + ")");
                        } else {
                            if(msg.getNetid() != 0)
                                System.out.println("\tMessage on netid " + msg.getNetid() + " with length " + msg.getLength());
                        }
                    }
                    icsneojava.delete_neomessage_t_array(msgs);
                    break;
                }
                // Send message
                case 'G':
                case 'g': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    // Start generating sample msg
                    neomessage_can_t msg = new neomessage_can_t();
                    msg.setArbid(0x120);
                    msg.setLength(6);
                    msg.setNetid(icsneojava.ICSNEO_NETID_HSCAN);
                    msg.setData(new byte[] { (byte) 0xaa, (byte) 0xbb, (byte) 0xcc, (byte) 0xdd, (byte) 0xee, (byte) 0xff});
                    msg.getStatus().setCanfdFDF(0);
                    msg.getStatus().setExtendedFrame(0);
                    msg.getStatus().setCanfdBRS(0);
                    // End generating sample msg

                    // Attempt to transmit the sample msg
                    if(icsneojava.icsneo_transmit(selectedDevice, icsneojava.from_can_neomessage_t_cast(msg))) {
                        System.out.println("Message transmit successful!");
                    } else {
                        System.out.println("Failed to transmit message to " + description + "!\n");
                        printLastError();
                        System.out.println();
                    }
                    break;
                }
                // Get events
                case 'H':
                case 'h':
                    printAPIEvents();
                    System.out.println();
                    break;
                // Set HS CAN to 250k
                case 'I':
                case 'i': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    // Attempt to set baudrate and apply settings
                    if(icsneojava.icsneo_setBaudrate(selectedDevice, icsneojava.ICSNEO_NETID_HSCAN, 250000) && icsneojava.icsneo_settingsApply(selectedDevice)) {
                        System.out.println("Successfully set HS CAN baudrate for " + description + "to 250k!\n");
                    } else {
                        System.out.println("Failed to set HS CAN for " + description + " to 250k!\n");
                        printLastError();
                        System.out.println();
                    }
                    break;
                }
                // Set HS CAN to 500k
                case 'J':
                case 'j': {
                    // Select a device and get its description
                    if(numDevices == 0) {
                        System.out.println("No devices found! Please scan for new devices.\n");
                        break;
                    }
                    selectedDevice = selectDevice();

                    // Get the product description for the device
                    StringBuffer description = new StringBuffer(icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION);
                    int[] maxLength = {icsneojava.ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION};

                    icsneojava.icsneo_describeDevice(selectedDevice, description, maxLength);

                    // Attempt to set baudrate and apply settings
                    if(icsneojava.icsneo_setBaudrate(selectedDevice, icsneojava.ICSNEO_NETID_HSCAN, 500000) && icsneojava.icsneo_settingsApply(selectedDevice)) {
                        System.out.println("Successfully set HS CAN baudrate for " + description + "to 500k!\n");
                    } else {
                        System.out.println("Failed to set HS CAN for " + description + " to 500k!\n");
                        printLastError();
                        System.out.println();
                    }
                    break;
                }
                case 'X':
                case 'x':
                    System.out.println("Exiting program");
                    return;
                default:
                    System.out.println("Unexpected input, exiting!");
                    return;
            }
        }
    }
}