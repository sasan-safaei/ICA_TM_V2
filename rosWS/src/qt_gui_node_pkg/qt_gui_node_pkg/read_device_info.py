#!/usr/bin/env python3
"""
Read IP and MAC address of network devices on Linux
"""

import os
import re
import subprocess
from datetime import datetime
tm_workspace = os.environ.get("TM_WORKSPACE")
CONFIG_CFG_PATH = os.path.join(tm_workspace, "config.cfg")

def get_ip_address(interface='eth0'):
    """
    Get IP address for a specific network interface
    
    Args:
        interface: Network interface name (default: 'eth0')
        
    Returns:
        IP address as string or None if not found
    """
    try:
        # Using 'ip addr show' command
        result = subprocess.run(
            ['ip', 'addr', 'show', interface],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Parse IP address from output
        match = re.search(r'inet (\d+\.\d+\.\d+\.\d+)', result.stdout)
        if match:
            return match.group(1)
        return None
    except subprocess.CalledProcessError:
        return None


def get_mac_address(interface='eth0'):
    """
    Get MAC address for a specific network interface
    
    Args:
        interface: Network interface name (default: 'eth0')
        
    Returns:
        MAC address as string or None if not found
    """
    try:
        # Using 'ip link show' command
        result = subprocess.run(
            ['ip', 'link', 'show', interface],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Parse MAC address from output
        match = re.search(r'link/ether ([0-9a-f:]{17})', result.stdout)
        if match:
            return match.group(1)
        return None
    except subprocess.CalledProcessError:
        return None


def get_all_interfaces():
    """
    Get list of all network interfaces
    
    Returns:
        List of interface names
    """
    try:
        result = subprocess.run(
            ['ip', 'link', 'show'],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Parse interface names
        interfaces = re.findall(r'\d+: ([^:]+):', result.stdout)
        # Filter out loopback
        return [iface for iface in interfaces if iface != 'lo']
    except subprocess.CalledProcessError:
        return []


def get_machine_number(config_path=CONFIG_CFG_PATH):
    """
    Get the machine number from config.cfg StoreFolder entry.

    Returns:
        Machine number as string, or "unKnown Machin Number" if unavailable.
    """
    if not os.path.isfile(config_path):
        return "unKnown Machin Number"

    try:
        with open(config_path, "r", encoding="utf-8") as config_file:
            for line in config_file:
                if line.strip().startswith("StoreFolder"):
                    parts = line.split("=", 1)
                    if len(parts) < 2:
                        break
                    folder_value = parts[1].strip()
                    folder_value = folder_value.rstrip("/\\")
                    folder_value = folder_value.replace("\\", "/")
                    machine_number = folder_value.split("/")[-1]
                    return machine_number or "unKnown Machin Number"
    except OSError:
        return "unKnown Machin Number"

    return "unKnown Machin Number"


def get_store_folder_path(config_path=CONFIG_CFG_PATH):
    """
    Get the StoreFolder path from config.cfg.

    Returns:
        Full StoreFolder path string, or None if not available.
    """
    if not os.path.isfile(config_path):
        return None

    try:
        with open(config_path, "r", encoding="utf-8") as config_file:
            for line in config_file:
                if line.strip().startswith("StoreFolder"):
                    parts = line.split("=", 1)
                    if len(parts) < 2:
                        return None
                    folder_value = parts[1].strip().strip("\"")
                    folder_value = folder_value.replace("\\", "/").rstrip("/")
                    if folder_value.startswith("./"):
                        folder_value = folder_value[2:]
                    return os.path.join(CONFIG_CFG_PATH, folder_value)
    except OSError:
        return None

    return None


def get_store_folder_file_list(config_path=CONFIG_CFG_PATH):
    """
    Return file name, date, and time for StoreFolder entries.

    Returns:
        Output text with filename, date, and time, or an empty string if unavailable.
    """
    folder_path = get_store_folder_path(config_path)
    if not folder_path or not os.path.isdir(folder_path):
        return "No store folder found!"

    try:
        entries = []
        for name in sorted(os.listdir(folder_path)):
            full_path = os.path.join(folder_path, name)
            try:
                stat_result = os.stat(full_path)
            except OSError:
                continue
            timestamp = datetime.fromtimestamp(stat_result.st_mtime)
            entries.append(f"{name}\t{timestamp:%Y-%m-%d}\t{timestamp:%H:%M}")
        return "\n".join(entries)
    except OSError:
        return "No store folder found."


def get_device_info(interface=None):
    """
    Get IP and MAC address for a device
    
    Args:
        interface: Network interface name. If None, uses first available interface
        
    Returns:
        Dictionary with 'ip' and 'mac' keys, or None if interface not found
    """
    if interface is None:
        interfaces = get_all_interfaces()
        if not interfaces:
            return None
        interface = interfaces[0]
    
    ip = get_ip_address(interface)
    mac = get_mac_address(interface)
    
    return {
        'interface': interface,
        'ip': ip,
        'mac': mac,
        'machine_number': get_machine_number()
    }


if __name__ == '__main__':
    # Example usage
    print("\n" + "="*50)
    print("Available network interfaces:")
    interfaces = get_all_interfaces()
    for iface in interfaces:
        print(f"  - {iface}")
    
    print("\nDevice information:")
    # Get info for all interfaces
    for iface in interfaces:
        info = get_device_info(iface)
        if info:
            
            print(f"\nInterface: {info['interface']}")
            print(f"  IP Address:  {info['ip'] or 'Not assigned'}")
            print(f"  MAC Address: {info['mac'] or 'Not found'}")
    # Or get info for specific interface
    print("\n" + "="*50)
    print("Getting info for first available interface:")
    info = get_device_info()
    if info:
        print(f"Interface: {info['interface']}")
        print(f"IP Address:  {info['ip']}")
        print(f"MAC Address: {info['mac']}")

    print(f"\nMachine Number: {info['machine_number']}")
    print ("\nResult Store Folder file list:")
    print(get_store_folder_file_list())
    print("\n" + "="*50)

