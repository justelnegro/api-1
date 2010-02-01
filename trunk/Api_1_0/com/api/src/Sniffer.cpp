#include "Sniffer.h"
#define APP_NAME		"sniffex"
#define APP_DESC		"Sniffer example using libpcap"
#define APP_COPYRIGHT	"Copyright (c) 2005 The Tcpdump Group"
#define APP_DISCLAIMER	"THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM."

#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
namespace api
{
/*
 * sniffex.c
 *
 * Sniffer example of TCP/IP packet capture using libpcap.
 * 
 * Version 0.1.1 (2005-07-05)
 * Copyright (c) 2005 The Tcpdump Group
 *
 * This software is intended to be used as a practical example and 
 * demonstration of the libpcap library; available at:
 * http://www.tcpdump.org/ */
 
 	/*********************************************************************/
 	/*********************************************************************/
 	/******************** Atributos, Constantes **************************/
 	/******************** Y Estructuras de la clase***********************/
 	/*********************************************************************/	
 
 //Esta clase contiene toda la implementacion para el analizador de paquetes
 //realizado por la gente de TCPDUMP
 
 /* default snap length (maximum bytes per packet to capture) */
	#define SNAP_LEN 1518
	
	/* ethernet headers are always exactly 14 bytes [1] */
	#define SIZE_ETHERNET 14
	
	/* Ethernet addresses are 6 bytes */
	#define ETHER_ADDR_LEN	6
	
	/* Ethernet header */
	struct sniff_ethernet {
	        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
	        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
	        u_short ether_type;                     /* IP? ARP? RARP? etc */
	};
	
	/* IP header */
	struct sniff_ip {
	        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
	        u_char  ip_tos;                 /* type of service */
	        u_short ip_len;                 /* total length */
	        u_short ip_id;                  /* identification */
	        u_short ip_off;                 /* fragment offset field */
	        #define IP_RF 0x8000            /* reserved fragment flag */
	        #define IP_DF 0x4000            /* dont fragment flag */
	        #define IP_MF 0x2000            /* more fragments flag */
	        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
	        u_char  ip_ttl;                 /* time to live */
	        u_char  ip_p;                   /* protocol */
	        u_short ip_sum;                 /* checksum */
	        struct  in_addr ip_src,ip_dst;  /* source and dest address */
	};
	#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
	#define IP_V(ip)                (((ip)->ip_vhl) >> 4)
	
	/* TCP header */
	typedef u_int tcp_seq;
	
	struct sniff_tcp {
	        u_short th_sport;               /* source port */
	        u_short th_dport;               /* destination port */
	        tcp_seq th_seq;                 /* sequence number */
	        tcp_seq th_ack;                 /* acknowledgement number */
	        u_char  th_offx2;               /* data offset, rsvd */
	#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
	        u_char  th_flags;
	        #define TH_FIN  0x01
	        #define TH_SYN  0x02
	        #define TH_RST  0x04
	        #define TH_PUSH 0x08
	        #define TH_ACK  0x10
	        #define TH_URG  0x20
	        #define TH_ECE  0x40
	        #define TH_CWR  0x80
	        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	        u_short th_win;                 /* window */
	        u_short th_sum;                 /* checksum */
	        u_short th_urp;                 /* urgent pointer */
	};
	
	
	/*******Contructor********/		
	Sniffer::Sniffer()
	{
	}
	/******Destructor*******/		
	Sniffer::~Sniffer()
	{
	}

	void
	got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
	
	void
	print_payload(const u_char *payload, int len);
	
	void
	print_hex_ascii_line(const u_char *payload, int len, int offset);
	
	void
	print_app_banner(void);
	
	void
	print_app_usage(void);
	
	/*
	 * app name/banner
	 */
	void
	print_app_banner(void)
	{
	
		printf("%s - %s\n", APP_NAME, APP_DESC);
		printf("%s\n", APP_COPYRIGHT);
		printf("%s\n", APP_DISCLAIMER);
		printf("\n");
	
	return;
	}
	
	/*
	 * print help text
	 */
	void
	print_app_usage(void)
	{
	
		printf("Usage: %s [interface]\n", APP_NAME);
		printf("\n");
		printf("Options:\n");
		printf("    interface    Listen on <interface> for packets.\n");
		printf("\n");
	
	return;
	}
	
	/*
	 * print data in rows of 16 bytes: offset   hex   ascii
	 *
	 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
	 */
	void
	print_hex_ascii_line(const u_char *payload, int len, int offset)
	{
	
		int i;
		int gap;
		const u_char *ch;
		int b;
		/* offset */
		printf("%05d   ", offset);
		
		/* hex */	
	
	
		
		ch = payload;
		for(i = 0; i < len; i++) {
			printf("%02x ", *ch);
			ch++;
			/* print extra space after 8th byte for visual aid */
			if (i == 7)
				printf(" ");
		}
		/* print space to handle line less than 8 bytes */
		if (len < 8)
			printf(" ");
		
		/* fill hex gap with spaces if not full line */
		if (len < 16) {
			gap = 16 - len;
			for (i = 0; i < gap; i++) {
				printf("   ");
			}
		}
		printf("   ");
		
		/* ascii (if printable) */
		ch = payload;
		for(i = 0; i < len; i++) {
			if (isprint(*ch))
				printf("%c", *ch);
			else
				printf(".");
			ch++;
		}
	
		printf("\n");
	
	return;
	}
	
	/*
	 * print packet payload data (avoid printing binary data)
	 */
	void
	print_payload(const u_char *payload, int len)
	{
	
		int len_rem = len;
		int line_width = 16;			/* number of bytes per line */
		int line_len;
		int offset = 0;					/* zero-based offset counter */
		const u_char *ch = payload;
	
		if (len <= 0)
			return;
	
		/* data fits on one line */
		if (len <= line_width) {
			print_hex_ascii_line(ch, len, offset);
			return;
		}
	
		/* data spans multiple lines */
		for ( ;; ) {
			/* compute current line length */
			line_len = line_width % len_rem;
			/* print line */
			print_hex_ascii_line(ch, line_len, offset);
			/* compute total remaining */
			len_rem = len_rem - line_len;
			/* shift pointer to remaining bytes to print */
			ch = ch + line_len;
			/* add offset */
			offset = offset + line_width;
			/* check if we have line width chars or less */
			if (len_rem <= line_width) {
				/* print last line and get out */
				print_hex_ascii_line(ch, len_rem, offset);
				break;
			}
		}
	
	return;
	}
	
	/*
	 * dissect/print packet
	 */
	void
	got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
	{
	
		static int count = 1;                   /* packet counter */
		
		/* declare pointers to packet headers */
		const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
		const struct sniff_ip *ip;              /* The IP header */
		const struct sniff_tcp *tcp;            /* The TCP header */
		const u_char *payload;                    /* Packet payload */
	
		int size_ip;
		int size_tcp;
		int size_payload;
		
		printf("\nPacket number %d:\n", count);
		count++;
		
		/* define ethernet header */
		ethernet = (struct sniff_ethernet*)(packet);
		
		/* define/compute ip header offset */
		ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
		size_ip = IP_HL(ip)*4;
		if (size_ip < 20) {
			printf("   * Invalid IP header length: %u bytes\n", size_ip);
			return;
		}
	
		/* print source and destination IP addresses */
		printf("       From: %s\n", inet_ntoa(ip->ip_src));
		printf("         To: %s\n", inet_ntoa(ip->ip_dst));
		
		/* determine protocol */	
		switch(ip->ip_p) {
			case IPPROTO_TCP:
				printf("   Protocol: TCP\n");
				break;
			case IPPROTO_UDP:
				printf("   Protocol: UDP\n");
				return;
			case IPPROTO_ICMP:
				printf("   Protocol: ICMP\n");
				return;
			case IPPROTO_IP:
				printf("   Protocol: IP\n");
				return;
			default:
				printf("   Protocol: unknown\n");
				return;
		}
		
		/*
		 *  OK, this packet is TCP.
		 */
		
		/* define/compute tcp header offset */
		tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
		size_tcp = TH_OFF(tcp)*4;
		if (size_tcp < 20) {
			printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
			return;
		}
		
		printf("   Src port: %d\n", ntohs(tcp->th_sport));
		printf("   Dst port: %d\n", ntohs(tcp->th_dport));
		
		/* define/compute tcp payload (segment) offset */
		payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
		
		/* compute tcp payload (segment) size */
		size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
		
		/*
		 * Print payload data; it might be binary, so don't just
		 * treat it as a string.
		 */
		if (size_payload > 0) {
			printf("   Payload (%d bytes):\n", size_payload);
			print_payload(payload, size_payload);
			
		}
	
	return;
	}
}