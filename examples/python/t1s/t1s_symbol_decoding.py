"""
10BASE-T1S Symbol Decoding Example
Demonstrates T1S bus symbol decoding and analysis
"""

import icsneopy
import time
from enum import IntEnum


class T1SSymbol(IntEnum):
    """10BASE-T1S Symbol Types"""
    SSD = 0x04
    ESDOK = 0x07
    BEACON = 0x08
    ESD = 0x0D
    ESDERR = 0x11
    SYNC = 0x18
    ESDJAB = 0x19
    SILENCE = 0x1F
    
    @classmethod
    def get_name(cls, value):
        """Get human-readable name for symbol value."""
        try:
            return cls(value).name
        except ValueError:
            if 0x00 <= value <= 0x0F:
                return f"DATA(0x{value:X})"
            return f"UNKNOWN(0x{value:02X})"


def get_user_confirmation(prompt):
    """Get yes/no confirmation from user."""
    response = input(f"{prompt} (y/n): ").strip().lower()
    return response == 'y'


def configure_t1s_decoding(device, network, enable_symbols, enable_beacons):
    """Configure T1S bus decoding settings."""
    settings = device.get_settings()
    if not settings:
        raise RuntimeError("Failed to get device settings")
    
    print(f"\nConfiguring T1S decoding on network {network}...")
    
    if not settings.set_t1s_bus_decoding_all(network, enable_symbols):
        raise RuntimeError("Failed to set T1S symbol decoding")
    if enable_symbols:
        print("  ✓ Enabled decoding of all T1S symbols")
    else:
        print("  • T1S symbol decoding disabled")
    
    if not settings.set_t1s_bus_decoding_beacons(network, enable_beacons):
        raise RuntimeError("Failed to set T1S beacon decoding")
    if enable_beacons:
        print("  ✓ Enabled T1S beacon decoding")
    else:
        print("  • T1S beacon decoding disabled")
    
    if not device.set_settings(settings):
        raise RuntimeError("Failed to apply settings to device")
    print("  ✓ Settings applied successfully")


def setup_symbol_monitoring(device, network):
    """Setup callback to monitor and decode T1S symbols."""
    state = {
        'symbol_count': 0,
        'beacon_count': 0,
        'wake_count': 0,
        'burst_count': 0,
        'symbol_stats': {},
        'data_frame_count': 0
    }
    
    def symbol_handler(msg):
        """Handle incoming T1S messages."""
        if not isinstance(msg, icsneopy.EthernetMessage):
            return
        
        if not msg.isT1S:
            return
        
        timestamp_ms = msg.timestamp / 1000000.0
        
        if msg.isT1SSymbol:
            num_symbols = len(msg.data)
            
            print(f"[{timestamp_ms:12.3f} ms] T1S Symbols", end="")
            if num_symbols > 0:
                print(f" ({num_symbols} symbol{'s' if num_symbols > 1 else ''})", end="")
            print(f" | Node ID: {msg.t1sNodeId}")
            
            for i, symbol_value in enumerate(msg.data):
                symbol_name = T1SSymbol.get_name(symbol_value)
                
                state['symbol_count'] += 1
                if symbol_name not in state['symbol_stats']:
                    state['symbol_stats'][symbol_name] = 0
                state['symbol_stats'][symbol_name] += 1
                
                if symbol_value == T1SSymbol.BEACON:
                    state['beacon_count'] += 1
                
                print(f"  [{i}] {symbol_name:10s} = 0x{symbol_value:02X}")
            
            if num_symbols == 0 and msg.t1sSymbolType != 0:
                symbol_value = msg.t1sSymbolType
                symbol_name = T1SSymbol.get_name(symbol_value)
                
                state['symbol_count'] += 1
                if symbol_name not in state['symbol_stats']:
                    state['symbol_stats'][symbol_name] = 0
                state['symbol_stats'][symbol_name] += 1
                
                if symbol_value == T1SSymbol.BEACON:
                    state['beacon_count'] += 1
                
                print(f"  {symbol_name:10s} = 0x{symbol_value:02X} (from t1sSymbolType field)")
        
        elif msg.isT1SBurst:
            state['burst_count'] += 1
            print(f"[{timestamp_ms:12.3f} ms] BURST | "
                  f"Node ID: {msg.t1sNodeId} | "
                  f"Burst Count: {msg.t1sBurstCount}")
        
        elif msg.isT1SWake:
            state['wake_count'] += 1
            print(f"[{timestamp_ms:12.3f} ms] WAKE signal detected | "
                  f"Node ID: {msg.t1sNodeId}")
        
        else:
            state['data_frame_count'] += 1
            print(f"[{timestamp_ms:12.3f} ms] T1S Data Frame | "
                  f"Length: {len(msg.data)} bytes | "
                  f"Node ID: {msg.t1sNodeId}")
            
            if msg.data and len(msg.data) > 0:
                preview = ' '.join([f"{b:02X}" for b in msg.data[:16]])
                if len(msg.data) > 16:
                    preview += " ..."
                print(f"                    Data: {preview}")
    
    frame_filter = icsneopy.MessageFilter(network)
    callback = icsneopy.MessageCallback(symbol_handler, frame_filter)
    device.add_message_callback(callback)
    
    return state


def print_statistics(state):
    """Print monitoring statistics."""
    print("\n" + "=" * 70)
    print("T1S SYMBOL DECODING STATISTICS")
    print("=" * 70)
    print(f"Total Symbols:              {state['symbol_count']}")
    print(f"Total Beacons:              {state['beacon_count']}")
    print(f"Total Wake Signals:         {state['wake_count']}")
    print(f"Total Bursts:               {state['burst_count']}")
    print(f"Total Data Frames:          {state['data_frame_count']}")
    
    if state['symbol_stats']:
        print("\n" + "-" * 70)
        print("Symbol Type Breakdown:")
        print("-" * 70)
        for symbol_name, count in sorted(state['symbol_stats'].items(), 
                                         key=lambda x: x[1], reverse=True):
            print(f"  {symbol_name:20s}{count:>10d}")
    print("=" * 70)


def main():
    """Main T1S symbol decoding example."""
    device = None
    
    try:
        MONITOR_NETWORK = icsneopy.Network.NetID.AE_02
        MONITOR_DURATION = 30
        
        print("\n" + "=" * 70)
        print("10BASE-T1S SYMBOL DECODING EXAMPLE")
        print("=" * 70)
        print(f"libicsneo {icsneopy.get_version()}")
        print("=" * 70)
        
        print("\nFinding devices... ", end="", flush=True)
        devices = icsneopy.find_all_devices()
        print(f"OK, {len(devices)} device{'s' if len(devices) != 1 else ''} found")
        
        if not devices:
            print("No devices found!")
            return 1
        
        for d in devices:
            print(f"  {d}")
        
        device = None
        for d in devices:
            if d.get_type() == icsneopy.DeviceType.RADComet3:
                device = d
                break
        
        if not device and devices:
            device = devices[0]
        
        if not device:
            print("No suitable device found!")
            return 1
        
        print(f"\nSelected device: {device}")
        print(f"Serial: {device.get_serial()}")
        
        print("\n" + "-" * 70)
        print("T1S DECODING CONFIGURATION")
        print("-" * 70)
        enable_symbols = get_user_confirmation("Enable T1S symbol decoding (all symbols)")
        enable_beacons = get_user_confirmation("Enable T1S beacon decoding")
        print("-" * 70)
        
        print("\nOpening device... ", end="", flush=True)
        if not device.open():
            print("✗ Failed")
            return 1
        print("✓")
        
        print("Enabling message polling... ", end="", flush=True)
        if not device.enable_message_polling():
            print("✗ Failed")
            device.close()
            return 1
        device.set_polling_message_limit(100000)
        print("✓")
        
        configure_t1s_decoding(device, MONITOR_NETWORK, enable_symbols, enable_beacons)
        
        print("Going online... ", end="", flush=True)
        if not device.go_online():
            print("✗ Failed")
            device.close()
            return 1
        print("✓")
        
        state = setup_symbol_monitoring(device, MONITOR_NETWORK)
        
        print("\n" + "-" * 70)
        print(f"Monitoring T1S traffic for {MONITOR_DURATION} seconds...")
        print("-" * 70)
        
        start_time = time.time()
        while time.time() - start_time < MONITOR_DURATION:
            device.get_messages()
            time.sleep(0.01)
        
        print("\n" + "-" * 70)
        print("Closing device... ", end="", flush=True)
        device.close()
        time.sleep(0.1)
        print("✓")
        
        print_statistics(state)
        
    except KeyboardInterrupt:
        print("\n\nMonitoring interrupted by user")
        if 'state' in locals():
            print_statistics(state)
    
    except Exception as e:
        print(f"\nError: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    finally:
        if device and device.is_open():
            device.close()
    
    return 0


if __name__ == "__main__":
    exit(main())
