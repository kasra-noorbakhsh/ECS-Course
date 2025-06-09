from scapy.all import rdpcap, TCP, IP
import sys
from collections import defaultdict

def syn_scanner(pcap_file):
    try:
        packets = rdpcap(pcap_file)
    except FileNotFoundError:
        print(f"Error: File not found: {pcap_file}")
        return []

    syn_sent_count = defaultdict(int)
    syn_ack_received_response_to = defaultdict(int)

    syn_sent_ips_ports = {} # To track (src_ip, sport, dst_ip, dport, seq) of sent SYNs

    for packet in packets:
        if IP in packet and TCP in packet:
            src_ip = packet[IP].src
            dst_ip = packet[IP].dst
            sport = packet[TCP].sport
            dport = packet[TCP].dport
            seq = packet[TCP].seq
            tcp_flags = packet[TCP].flags

            # SYN 
            if tcp_flags == 'S':
                syn_sent_count[src_ip] += 1
                syn_sent_ips_ports[(src_ip, sport, dst_ip, dport, seq)] = True

            # SYN-ACK 
            if tcp_flags == 'SA':
                ack = packet[TCP].ack - 1 # The ack number in SYN-ACK is usually seq + 1
                for sent_syn in list(syn_sent_ips_ports.keys()):
                    s_ip, s_port, d_ip, d_port, s_seq = sent_syn
                    if src_ip == d_ip and dst_ip == s_ip and packet[TCP].ack == s_seq + 1:
                        syn_ack_received_response_to[dst_ip] += 1
                        del syn_sent_ips_ports[sent_syn] # Remove the matched SYN
                        break

    suspect_scanners = []
    for ip, syn_count in syn_sent_count.items():
        ack_count = syn_ack_received_response_to.get(ip, 0)
        if syn_count > 3 * ack_count and ack_count >= 0: # Adjusted condition
            suspect_scanners.append(ip)
        elif syn_count > 10 and ack_count == 0:
            suspect_scanners.append(ip)

    return list(set(suspect_scanners))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python syn_scanner.py <pcap_file>")
        sys.exit(1)

    pcap_file = sys.argv[1]
    suspects = syn_scanner(pcap_file)

    if suspects:
        # print("\nPotential SYN port scanners:")
        for ip in suspects:
            print(ip)
    # else:
        # print("\nNo potential SYN port scanners detected based on the criteria.")