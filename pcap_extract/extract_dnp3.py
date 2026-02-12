#!/usr/bin/env python3
"""Extract DNP3 payloads from pcap files at multiple protocol layers."""

import subprocess
import json
import re
import os
import sys

def extract_from_pcap(pcap_path):
    """Extract DNP3 payloads from a pcap file using tshark -x."""
    basename = os.path.splitext(os.path.basename(pcap_path))[0]
    
    # Get hex dump with reassembly
    result = subprocess.run(
        ['tshark', '-r', pcap_path, '-Y', 'dnp3', '-x'],
        capture_output=True, text=True
    )
    
    # Also get frame info
    info_result = subprocess.run(
        ['tshark', '-r', pcap_path, '-Y', 'dnp3', '-T', 'fields',
         '-e', 'frame.number', '-e', 'dnp3.al.func', '-e', '_ws.col.Info'],
        capture_output=True, text=True
    )
    
    frame_info = {}
    for line in info_result.stdout.strip().split('\n'):
        if line.strip():
            parts = line.split('\t')
            if len(parts) >= 3:
                frame_info[parts[0]] = {'func': parts[1], 'info': parts[2]}
            elif len(parts) >= 2:
                frame_info[parts[0]] = {'func': parts[1], 'info': ''}
    
    # Parse hex dump output to extract payloads
    packets = []
    current_packet = None
    current_section = None
    hex_lines = []
    
    for line in result.stdout.split('\n'):
        # New packet
        m = re.match(r'^Packet \((\d+) bytes\):', line)
        if m:
            if current_packet and hex_lines:
                current_packet[current_section] = parse_hex_lines(hex_lines)
                hex_lines = []
            if current_packet:
                packets.append(current_packet)
            current_packet = {'sections': {}}
            current_section = 'raw_packet'
            hex_lines = []
            continue
        
        # Reassembled section
        m = re.match(r'^Reassembled DNP 3\.0 Application Layer message \((\d+) bytes\):', line)
        if m:
            if hex_lines and current_section:
                current_packet[current_section] = parse_hex_lines(hex_lines)
                hex_lines = []
            current_section = 'apdu'
            continue
        
        # Reassembled TCP
        m = re.match(r'^Reassembled TCP', line)
        if m:
            if hex_lines and current_section:
                current_packet[current_section] = parse_hex_lines(hex_lines)
                hex_lines = []
            current_section = 'tcp_reassembled'
            continue
        
        # Hex data line
        m = re.match(r'^([0-9a-fA-F]{4})\s+((?:[0-9a-fA-F]{2}\s)+)', line)
        if m:
            hex_lines.append(m.group(2).strip())
    
    # Don't forget last packet
    if current_packet and hex_lines:
        current_packet[current_section] = parse_hex_lines(hex_lines)
    if current_packet:
        packets.append(current_packet)
    
    return packets, frame_info, basename


def parse_hex_lines(lines):
    """Parse hex dump lines into bytes."""
    all_bytes = []
    for line in lines:
        hex_bytes = line.split()
        all_bytes.extend([int(b, 16) for b in hex_bytes if len(b) == 2])
    return bytes(all_bytes)


def extract_link_frame(raw_packet):
    """Extract DNP3 link frame from raw packet (skip Ethernet + IP + TCP headers)."""
    data = raw_packet
    
    # Find DNP3 start bytes 0x05 0x64
    for i in range(len(data) - 1):
        if data[i] == 0x05 and data[i+1] == 0x64:
            return data[i:]
    return None


def extract_apdu_objects(apdu):
    """Extract just the object headers portion (after app control + function code for requests,
    or after app control + function code + IIN for responses)."""
    if len(apdu) < 2:
        return None
    
    func_code = apdu[1]
    
    # Response (129/0x81) or Unsolicited Response (130/0x82) have IIN bytes
    if func_code in (129, 130):
        if len(apdu) < 4:
            return None
        return apdu[4:]  # Skip control + func + IIN(2)
    else:
        return apdu[2:]  # Skip control + func


def bytes_to_hex_string(data):
    """Convert bytes to space-separated hex string."""
    return ' '.join(f'{b:02X}' for b in data)


def main():
    pcap_files = [
        '/home/feb/pro/opendnp3/pcap_extract/dnp3_read.pcap',
        '/home/feb/pro/opendnp3/pcap_extract/dnp3_write.pcap',
        '/home/feb/pro/opendnp3/pcap_extract/dnp3_file_read.pcap',
        '/home/feb/pro/opendnp3/pcap_extract/dnp3.pcap',
    ]
    
    outdir = '/home/feb/pro/opendnp3/pcap_extract'
    os.makedirs(f'{outdir}/apdu', exist_ok=True)
    os.makedirs(f'{outdir}/link_frames', exist_ok=True)
    os.makedirs(f'{outdir}/apdu_objects', exist_ok=True)
    
    all_results = []
    
    for pcap_path in pcap_files:
        if not os.path.exists(pcap_path):
            print(f"SKIP: {pcap_path} not found")
            continue
            
        packets, frame_info, basename = extract_from_pcap(pcap_path)
        
        print(f"\n{'='*70}")
        print(f"File: {basename}.pcap  ({len(packets)} DNP3 packets)")
        print(f"{'='*70}")
        
        frame_numbers = sorted(frame_info.keys(), key=int)
        
        pkt_idx = 0
        for fn in frame_numbers:
            if pkt_idx >= len(packets):
                break
            
            pkt = packets[pkt_idx]
            info = frame_info[fn]
            func = info.get('func', '?')
            desc = info.get('info', '')
            pkt_idx += 1
            
            print(f"\n--- Frame {fn}: {desc} (func={func}) ---")
            
            # Extract link frame from raw packet
            raw = pkt.get('raw_packet', b'')
            link_frame = extract_link_frame(raw)
            
            # Extract APDU (reassembled or from data chunks)
            apdu = pkt.get('apdu', None)
            
            if link_frame:
                hex_str = bytes_to_hex_string(link_frame)
                print(f"  Link frame ({len(link_frame)} bytes): {hex_str[:120]}{'...' if len(hex_str) > 120 else ''}")
                fname = f"{basename}_frame{fn}_link.bin"
                with open(f'{outdir}/link_frames/{fname}', 'wb') as f:
                    f.write(link_frame)
            
            if apdu:
                hex_str = bytes_to_hex_string(apdu)
                print(f"  APDU ({len(apdu)} bytes): {hex_str}")
                fname = f"{basename}_frame{fn}_apdu.bin"
                with open(f'{outdir}/apdu/{fname}', 'wb') as f:
                    f.write(apdu)
                
                # Extract object headers only
                objects = extract_apdu_objects(apdu)
                if objects and len(objects) > 0:
                    obj_hex = bytes_to_hex_string(objects)
                    print(f"  Objects ({len(objects)} bytes): {obj_hex}")
                    fname = f"{basename}_frame{fn}_objects.bin"
                    with open(f'{outdir}/apdu_objects/{fname}', 'wb') as f:
                        f.write(objects)
                    
                    all_results.append({
                        'file': basename,
                        'frame': fn,
                        'desc': desc,
                        'func': func,
                        'apdu_hex': bytes_to_hex_string(apdu),
                        'objects_hex': bytes_to_hex_string(objects) if objects else '',
                    })
            elif not apdu and link_frame:
                # For multi-fragment packets without reassembly shown, 
                # the APDU may not be separately listed
                print(f"  (No reassembled APDU - may be a fragment)")
    
    # Write summary
    print(f"\n\n{'='*70}")
    print("SUMMARY - Extracted APDUs suitable for parser testing")
    print(f"{'='*70}")
    
    print("\n// Full APDUs (application control + function code + [IIN] + objects)")
    print("// Feed these to MContext::OnReceive() or the fuzz master/outstation\n")
    for r in all_results:
        print(f'// {r["file"]} frame {r["frame"]}: {r["desc"]}')
        print(f'"{r["apdu_hex"]}"')
        print()
    
    print("\n// Object headers only (after control+func+IIN)")
    print("// Feed these to APDUParser::Parse()\n")
    for r in all_results:
        if r['objects_hex']:
            print(f'// {r["file"]} frame {r["frame"]}: {r["desc"]}')
            print(f'"{r["objects_hex"]}"')
            print()
    
    # Write C++ header with all payloads
    with open(f'{outdir}/pcap_payloads.h', 'w') as f:
        f.write('// Auto-extracted DNP3 payloads from pcap files\n')
        f.write('// For use with APDUParser::Parse(), fuzz tests, etc.\n')
        f.write('#pragma once\n\n')
        f.write('#include <string>\n')
        f.write('#include <vector>\n\n')
        f.write('namespace pcap_payloads {\n\n')
        
        f.write('struct Payload {\n')
        f.write('    const char* source;\n')
        f.write('    const char* description;\n')
        f.write('    const char* apdu_hex;      // Full APDU\n')
        f.write('    const char* objects_hex;    // Object headers only\n')
        f.write('};\n\n')
        
        f.write('// clang-format off\n')
        f.write('static const Payload payloads[] = {\n')
        for r in all_results:
            src = f'{r["file"]}_frame{r["frame"]}'
            desc = r['desc'].replace('"', '\\"')
            f.write(f'    {{"{src}", "{desc}",\n')
            f.write(f'     "{r["apdu_hex"]}",\n')
            f.write(f'     "{r["objects_hex"]}"}},\n')
        f.write('};\n')
        f.write('// clang-format on\n\n')
        
        f.write(f'static const size_t num_payloads = {len(all_results)};\n\n')
        f.write('} // namespace pcap_payloads\n')
    
    print(f"\nOutput files written to:")
    print(f"  {outdir}/apdu/          - Raw APDU binary files")
    print(f"  {outdir}/link_frames/   - Raw link frame binary files")
    print(f"  {outdir}/apdu_objects/  - Object header binary files")
    print(f"  {outdir}/pcap_payloads.h - C++ header with all payloads")


if __name__ == '__main__':
    main()
