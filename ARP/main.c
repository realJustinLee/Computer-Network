//
// Created by 李欣 on 2017/10/19.
//

#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <strings.h>
#include <ctype.h>

#define ETH_HW_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define ARP_FRAME_TYPE 0x0806
#define ETHER_HW_TYPE 1
#define IP_PROTO_TYPE 0x0800
#define OP_ARP_REQUEST 2
#define OP_ARP_QUEST 1
#define DEFAULT_DEVICE "eth0"

char usage[] = {
        "send_arp: sends out custom ARP packet. \nusage: ./send_arp src_ip_addr src_hw_addr targ_hw_addr targ_hw_addr number"};

struct arp_packet {
    u_char targ_hw_addr[ETH_HW_ADDR_LEN];
    u_char src_hw_addr[ETH_HW_ADDR_LEN];
    u_short frame_type;
    u_short hw_type;
    u_short prot_type;
    u_char hw_addr_size;
    u_char prot_addr_size;
    u_short op;
    u_char sndr_hw_addr[ETH_HW_ADDR_LEN];
    u_char sndr_ip_addr[IP_ADDR_LEN];
    u_char rcpt_hw_addr[ETH_HW_ADDR_LEN];
    u_char rcpt_ip_addr[IP_ADDR_LEN];
    u_char padding[18];
};

void die(char *str) {
    fprintf(stderr, "%s\n", str);
    exit(1);
}

void get_ip_addr(struct in_addr *in_addr, char *str) {
    struct hostent *hostp;
    in_addr->s_addr = inet_addr(str);
    if (in_addr->s_addr == -1) {
        if ((hostp = gethostbyname(str))) {
            bcopy(hostp->h_addr, in_addr, hostp->h_length);
        } else {
            fprintf(stderr, "send_arp: unknown host %s\n", str);
            exit(1);
        }
    }
}

int hex_to_dec(const char *elm) {
    char std_char;
    int value = 0;
    if (!(std_char = (char) tolower(*elm))) {
        die("Invalid hardware address");
    }
    if (isdigit(std_char)) {
        value = (char) (std_char - '0');
    } else if (std_char >= 'a' && std_char <= 'f') {
        value = (char) (std_char - 'a' + 10);
    } else {
        die("Invalid hardware address");
    }
    return value;
}

void get_hw_addr(char *buf, char *str) {
    char charVal;
    for (int i = 0; i < ETH_HW_ADDR_LEN; ++i) {
        charVal = (char) (hex_to_dec(str++));
        *buf = charVal << 4;
        charVal = (char) (hex_to_dec(str++));
        *buf++ |= charVal;
        if (*str == ':') {
            str++;
        }
    }
}

int main(int argc, char *argv[]) {
    struct in_addr src_in_addr, targ_in_addr;
    struct arp_packet pkt;
    struct sockaddr sa;
    int sock;
    int number;
    if (argc != 6) {
        die(usage);
    }
    sock = socket(AF_INET, SOCK_PACKET, htons(ETH_P_RARP));
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    number = atoi(argv[5]);
    pkt.frame_type = htons(ARP_FRAME_TYPE);
    pkt.hw_type = htons(ETHER_HW_TYPE);
    pkt.prot_type = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;
    pkt.prot_addr_size = IP_ADDR_LEN;
    pkt.op = htons(OP_ARP_QUEST);
    get_hw_addr(pkt.targ_hw_addr, argv[4]);
    get_hw_addr(pkt.rcpt_hw_addr, argv[4]);
    get_hw_addr(pkt.src_hw_addr, argv[2]);
    get_hw_addr(pkt.sndr_hw_addr, argv[2]);
    get_ip_addr(&src_in_addr, argv[1]);
    get_ip_addr(&targ_in_addr, argv[3]);
    memcpy(pkt.sndr_hw_addr, &src_in_addr, IP_ADDR_LEN);
    memcpy(pkt.rcpt_hw_addr, &targ_in_addr, IP_ADDR_LEN);
    bzero(sa.sa_data, DEFAULT_DEVICE);
    for (int i = 0; i < number; ++i) {
        if (sendto(sock, sizeof(), 0, &sa, sizeof(sa)) < 0) {
            perror("sendto");
            exit(1);
        }
    }
    return 0;
}