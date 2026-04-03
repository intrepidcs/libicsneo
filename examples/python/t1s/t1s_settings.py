"""
10BASE-T1S Settings Configuration Example
Demonstrates interactive T1S network configuration
"""

import icsneopy


def get_user_confirmation(prompt):
    """Get yes/no confirmation from user."""
    response = input(f"{prompt} (y/n): ").strip().lower()
    return response == 'y'


def select_networks(available_networks):
    """Let user select which networks to configure."""
    print("\n" + "=" * 70)
    print("Select T1S Networks to Configure")
    print("=" * 70)
    
    for i, net_id in enumerate(available_networks, 1):
        print(f"  [{i}] {net_id}")
    
    response = input("\nEnter network numbers to configure (e.g., '1,3' or '1-3' or 'all'): ").strip().lower()
    
    if not response:
        return []
    
    if response == 'all':
        return available_networks
    
    selected = []
    tokens = response.split(',')
    
    for token in tokens:
        token = token.strip()
        
        if '-' in token:
            try:
                parts = token.split('-')
                start = int(parts[0])
                end = int(parts[1])
                for i in range(start, end + 1):
                    if 1 <= i <= len(available_networks):
                        selected.append(available_networks[i - 1])
            except (ValueError, IndexError):
                pass
        else:
            try:
                num = int(token)
                if 1 <= num <= len(available_networks):
                    selected.append(available_networks[num - 1])
            except ValueError:
                pass
    
    return selected


def get_uint8_input(prompt, default_value):
    """Get uint8 input from user with default."""
    response = input(f"{prompt} [{default_value}]: ").strip()
    if not response:
        return default_value
    try:
        val = int(response)
        if 0 <= val <= 255:
            return val
    except ValueError:
        pass
    return default_value


def get_uint16_input(prompt, default_value):
    """Get uint16 input from user with default."""
    response = input(f"{prompt} [{default_value}]: ").strip()
    if not response:
        return default_value
    try:
        val = int(response)
        if 0 <= val <= 65535:
            return val
    except ValueError:
        pass
    return default_value


def opt_to_string(opt):
    """Convert optional value to string for display."""
    if opt is None:
        return "N/A"
    if isinstance(opt, bool):
        return "true" if opt else "false"
    return str(opt)


def display_t1s_settings(device, network):
    """Display T1S settings for a network."""
    print(f"\t{network} T1S Settings:")
    
    settings = device.get_settings()
    if not settings:
        print("\t  Unable to read settings")
        return
    
    print(f"\t  PLCA Enabled:       {opt_to_string(settings.get_t1s_plca_enabled(network))}")
    print(f"\t  Local ID:           {opt_to_string(settings.get_t1s_local_id(network))}")
    print(f"\t  Max Nodes:          {opt_to_string(settings.get_t1s_max_nodes(network))}")
    print(f"\t  TX Opp Timer:       {opt_to_string(settings.get_t1s_tx_opp_timer(network))}")
    print(f"\t  Max Burst:          {opt_to_string(settings.get_t1s_max_burst(network))}")
    print(f"\t  Burst Timer:        {opt_to_string(settings.get_t1s_burst_timer(network))}")
    
    term_enabled = settings.get_t1s_termination_enabled(network)
    if term_enabled is not None:
        print(f"\t  Termination:        {opt_to_string(term_enabled)}")
    
    local_id_alt = settings.get_t1s_local_id_alternate(network)
    if local_id_alt is not None:
        print(f"\t  Local ID Alternate: {opt_to_string(local_id_alt)}")
        print(f"\t  Bus Dec Beacons:    {opt_to_string(settings.get_t1s_bus_decoding_beacons_enabled(network))}")
        print(f"\t  Bus Dec All:        {opt_to_string(settings.get_t1s_bus_decoding_all_enabled(network))}")
        
        multi_id_mask = settings.get_t1s_multi_id_enable_mask(network)
        if multi_id_mask is not None:
            print(f"\t  Multi-ID Mask:      0x{multi_id_mask:02X}")
            print("\t  Multi-IDs:          ", end="")
            multi_ids = []
            for i in range(7):
                multi_id = settings.get_t1s_multi_id(network, i)
                multi_ids.append(f"[{i}]={opt_to_string(multi_id)}")
            print(", ".join(multi_ids))
    
    print()


def configure_t1s_network(device, network):
    """Interactively configure T1S settings for a network."""
    print("\n" + "=" * 70)
    print(f"Configuring T1S Network: {network}")
    print("=" * 70)
    
    settings = device.get_settings()
    if not settings:
        print("Unable to read settings")
        return
    
    print("\n--- Basic PLCA Settings ---")
    plca_enabled = get_user_confirmation("Enable PLCA")
    settings.set_t1s_plca(network, plca_enabled)
    
    local_id = get_uint8_input("Local ID (0-255)", 1)
    settings.set_t1s_local_id(network, local_id)
    
    max_nodes = get_uint8_input("Max Nodes (0-255)", 8)
    settings.set_t1s_max_nodes(network, max_nodes)
    
    tx_opp_timer = get_uint8_input("TX Opportunity Timer (0-255)", 20)
    settings.set_t1s_tx_opp_timer(network, tx_opp_timer)
    
    max_burst = get_uint16_input("Max Burst (0-65535)", 128)
    settings.set_t1s_max_burst(network, max_burst)
    
    burst_timer = get_uint16_input("Burst Timer (0-65535)", 64)
    settings.set_t1s_burst_timer(network, burst_timer)
    
    if settings.get_t1s_termination_enabled(network) is not None:
        print("\n--- Termination Settings ---")
        term_enabled = get_user_confirmation("Enable Termination")
        settings.set_t1s_termination(network, term_enabled)
    
    if settings.get_t1s_local_id_alternate(network) is not None:
        print("\n--- Extended Settings ---")
        local_id_alt = get_uint8_input("Local ID Alternate (0-255)", 0)
        settings.set_t1s_local_id_alternate(network, local_id_alt)
        
        bus_dec_beacons = get_user_confirmation("Enable Bus Decoding (Beacons)")
        settings.set_t1s_bus_decoding_beacons(network, bus_dec_beacons)
        
        bus_dec_all = get_user_confirmation("Enable Bus Decoding (All Symbols)")
        settings.set_t1s_bus_decoding_all(network, bus_dec_all)
        
        if get_user_confirmation("Configure Multi-ID settings?"):
            multi_id_mask = get_uint8_input("Multi-ID Enable Mask (0x00-0xFF, hex)", 0x00)
            settings.set_t1s_multi_id_enable_mask(network, multi_id_mask)
            
            print("Configure Multi-IDs (7 slots):")
            for i in range(7):
                multi_id = get_uint8_input(f"  Multi-ID [{i}]", 0)
                settings.set_t1s_multi_id(network, i, multi_id)
    
    if not device.set_settings(settings):
        print("✗ Failed to update device settings")
    else:
        print(f"\n✓ Configuration complete for {network}")


def main():
    """Main T1S settings configuration example."""
    device = None
    
    try:
        print("\n" + "=" * 70)
        print("10BASE-T1S SETTINGS CONFIGURATION EXAMPLE")
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
        
        print("\nOpening device... ", end="", flush=True)
        if not device.open():
            print("✗ Failed")
            return 1
        print("✓")
        
        candidate_networks = [
            icsneopy.Network.NetID.AE_01, icsneopy.Network.NetID.AE_02,
            icsneopy.Network.NetID.AE_03, icsneopy.Network.NetID.AE_04,
            icsneopy.Network.NetID.AE_05, icsneopy.Network.NetID.AE_06,
            icsneopy.Network.NetID.AE_07, icsneopy.Network.NetID.AE_08,
            icsneopy.Network.NetID.AE_09, icsneopy.Network.NetID.AE_10
        ]
        
        settings = device.get_settings()
        t1s_networks = []
        for net_id in candidate_networks:
            local_id = settings.get_t1s_local_id(net_id)
            if local_id is not None:
                t1s_networks.append(net_id)
        
        if not t1s_networks:
            print("No T1S networks found on this device")
            device.close()
            return 1
        
        print(f"\nFound {len(t1s_networks)} T1S network{'s' if len(t1s_networks) != 1 else ''}:")
        for i, net_id in enumerate(t1s_networks, 1):
            print(f"  [{i}] {net_id}")
        
        print("\n" + "-" * 70)
        print("Current T1S Settings:")
        print("-" * 70)
        for net_id in t1s_networks:
            display_t1s_settings(device, net_id)
        
        networks_to_config = select_networks(t1s_networks)
        
        if not networks_to_config:
            print("\nNo networks selected for configuration.")
            print("Closing device... ", end="", flush=True)
            device.close()
            print("✓")
            return 0
        
        print(f"\nConfiguring {len(networks_to_config)} network{'s' if len(networks_to_config) != 1 else ''}...")
        
        for net_id in networks_to_config:
            configure_t1s_network(device, net_id)
        
        print("\n" + "=" * 70)
        save_to_eeprom = get_user_confirmation("Save settings to EEPROM (permanent)?")
        print("=" * 70)
        
        settings = device.get_settings()
        print(f"\nApplying settings{' to EEPROM' if save_to_eeprom else ' temporarily'}... ", end="", flush=True)
        success = settings.apply(not save_to_eeprom)
        if not success:
            print("✗ Failed")
            device.close()
            return 1
        print("✓")
        
        print("\n" + "-" * 70)
        print("Updated T1S Settings:")
        print("-" * 70)
        for net_id in t1s_networks:
            display_t1s_settings(device, net_id)
        
        print("Closing device... ", end="", flush=True)
        device.close()
        print("✓")
        
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        return 1
    
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
