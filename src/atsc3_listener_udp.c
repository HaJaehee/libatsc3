/*
 * atsc3_listener_udp.c
 *
 *  Created on: Mar 6, 2019
 *      Author: jjustman
 */

#include "atsc3_listener_udp.h"

udp_packet_t* process_packet_from_pcap(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
	int i = 0;
	int k = 0;
	u_char ethernet_packet[14];
	u_char ip_header[24];
	u_char udp_header[8];
	int udp_header_start = 34;
	udp_packet_t* udp_packet = NULL;

	for (i = 0; i < 14; i++) {
		ethernet_packet[i] = packet[0 + i];
	}
	if (!(ethernet_packet[12] == 0x08 && ethernet_packet[13] == 0x00)) {
		return NULL;
	}

	for (i = 0; i < 20; i++) {
	ip_header[i] = packet[14 + i];
	}

	//check if we are a UDP packet, otherwise bail
	if (ip_header[9] != 0x11) {
		return NULL;
	}

	if ((ip_header[0] & 0x0F) > 5) {
		udp_header_start = 48;
	}

	//malloc our udp_packet_header:
	udp_packet = (udp_packet_t*)calloc(1, sizeof(udp_packet_t));
	udp_packet->udp_flow.src_ip_addr = ((ip_header[12] & 0xFF) << 24) | ((ip_header[13]  & 0xFF) << 16) | ((ip_header[14]  & 0xFF) << 8) | (ip_header[15] & 0xFF);
	udp_packet->udp_flow.dst_ip_addr = ((ip_header[16] & 0xFF) << 24) | ((ip_header[17]  & 0xFF) << 16) | ((ip_header[18]  & 0xFF) << 8) | (ip_header[19] & 0xFF);

	for (i = 0; i < 8; i++) {
		udp_header[i] = packet[udp_header_start + i];
	}

	udp_packet->udp_flow.src_port = (udp_header[0] << 8) + udp_header[1];
	udp_packet->udp_flow.dst_port = (udp_header[2] << 8) + udp_header[3];

	udp_packet->total_packet_length = pkthdr->len;
	udp_packet->data_length = pkthdr->len - (udp_header_start + 8);

	if(udp_packet->data_length <=0 || udp_packet->data_length > 1514) {
		__LISTENER_UDP_ERROR("invalid data length of udp packet: %d", udp_packet->data_length);
		return NULL;
	}
	udp_packet->data = (u_char*)malloc(udp_packet->data_length * sizeof(udp_packet->data));
	memcpy(udp_packet->data, &packet[udp_header_start + 8], udp_packet->data_length);

	return udp_packet;
}