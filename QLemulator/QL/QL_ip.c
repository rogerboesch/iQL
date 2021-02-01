//
// QLIP (c) 1998 Jonathan Hudson & Richard Zidlicky
// This code is part of the uqlx QDOS emulator for Unix
//
// This file is outside the uqlx copyright and may be freely copied,
// used and modified for any non-commercial purpose.
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

#ifdef IPDEV

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "QL_68000.h"
#include "QL_serial.h"
#include "QL_ip.h"
#include "rb_endian.h"
#include "base_driver.h"
#include "base_cfg.h"
#include "base_util.h"
#include "base_iptraps.h"
#include "base_xc68.h"
#include "base_proto.h"
#include "QDOS.h"

/* some (many?) OS's don't know this yet ...*/
#ifdef SOLARIS
int getdomainname(char *name, size_t len)
{
  char *p=getenv("DOMAIN");
  
  if (strlen(p) > len)
    {
      errno=EINVAL;
      return -1;
    }
  strncpy(name,p,len);

  return 0;
}

#endif

#define IPDEBUGGER 1

#ifdef IPDEBUGGER
# define IPDEBUG(a) printf a
#else
# define IPDEBUG(a)
#endif



static open_arg ip_par[2];

int ip_init(int idx, void *p)
{
    return 0;
}

int ip_test(int id,char *name)
{
    int res=decode_name(name,Drivers[id].namep,ip_par);
    if (res)
      printf("ip_test: matched %s",name);
    return res;
}

extern char *a0addr(Cond check);

int ip_open(int id, void **priv)
{
    int qerr = 0;
    int fd, rv = -1;
    char *host = NULL;
    char *aport = NULL;
    u_short port = 0;
    struct sockaddr_in name;
    int cnstatus=0;
    static const char *dnms[] = {"tcp,","udp","uxs","uxd","sck",NULL};
    const char **dp;
    short dindx;
    
    host = ip_par[0].s;
    aport = ip_par[1].s;

     for(dindx = 0, dp = dnms; *dp; dp++,dindx++)
      {
         if(strncasecmp(Drivers[id].namep->name, *dp, 3) == 0)
             break;
      }
     
     if(dindx < 4 && (reg[3] >= 0 && reg[3] < 4)) 
       {
         int proto = (dindx & 1) ? SOCK_DGRAM : SOCK_STREAM; 
         int domain = (dindx < 2) ? AF_INET : AF_UNIX;
         /* Just use 0 for protocol, like GNU tell us */
         if((fd = socket (domain, proto, 0)) != -1)
	   {
	     cnstatus=-2; /* no connection yet*/
	     
	     if(reg[3] && host && aport)
	       {
		 struct hostent *h;
		 struct servent *s;
                
		 if((h = gethostbyname(host)) != NULL)
		   {
		     int (*func)();
		     
		     s = getservbyname(aport, (dindx & 1) ? "udp" : "tcp");
		     
		     port = (s) ? s->s_port : htons(atoi(aport));
		     if(port)
		       {
			 func = (reg[3] > 1) ? bind : connect;
			 name.sin_family = h->h_addrtype;
			 name.sin_port = port;
			 name.sin_addr = *((struct in_addr *) h->h_addr);
			 rv = (func) (fd, (struct sockaddr *) &name,
				      sizeof(struct sockaddr_in));
			 if (rv==0) cnstatus=0; /* connection complete */
			 if (rv<0 && reg[3]==1 && errno==EINTR)
			   {
			     cnstatus=-1;
			     rv=0;
			   }
		       }
		     else rv = -1;
		   }
		 if(h == NULL || rv == -1)
		   {
		     qerr = qmaperr();
		     close(fd);
		   }
	       }
	   }
	 else qerr = qmaperr ();
	 
       }
     else if( dindx == 4)
       {
	 if(reg[3] > 4)
	   {
	     int nlen = sizeof(name);
	     char *f;
	     int a0 = aReg[0];
	     ipdev_t *ip;
	     w32 scht;
	     
	     scht = RL((Ptr)theROM+aReg[6]+0x78);
	     aReg[0] = RL((Ptr)theROM+scht + ((reg[3] & 0xffff) << 2));
	     
	     /* !!! ADD USUAL ERROR CHECKING !!! */
	     
	     f = a0addr(0);
	     ip = DGET_PRIV(f);
	     
	     fd = accept(ip->sock, (struct sockaddr *) &name, &nlen);
	     
	     aReg[0] = a0;
	     if(fd > 0)
	       {
		 qerr = 0;
		 cnstatus=0; /* accept doesn't support async mode ...*/
	       }
	     else
	       qerr = qmaperr ();
	   }
	 else
	   {
	     qerr = 0;
	     fd = -1;
	   }
       }
     else qerr = QERR_BP;
     
     if(qerr == 0)
       {
	 *priv=malloc(sizeof(ipdev_t));
	 ((ipdev_t *)(*priv))->sock = fd;
	 ((ipdev_t *)(*priv))->name = name;    
	 ((ipdev_t *)(*priv))->status = cnstatus;
	 ((ipdev_t *)(*priv))->lerrno = 0;   
       }
     else
       {
	 reg[0] = qerr;
	 *priv = NULL;
       }
     return qerr;
}

void static check_status(ipdev_t *s)
{
  struct timeval tv;
  fd_set wfd,errfd;
  int res;
  
  if (s->status==-1)
    {
      if (check_pend(s->sock,SLC_WRITE)>0)
	s->status=0;
      else if (check_pend(s->sock,SLC_ERR)>0)
	{
	  int e = sizeof(errno);
	  
	  s->status=-2;
  	  perror("asynchronous connect ");
 	  printf("getsockopt res %d\n",getsockopt(s->sock,SOL_SOCKET,SO_ERROR,&errno,&e));
	}
      
    }
}


int ip_pend(ipdev_t *p)
{
  if (p->status==-1)
    check_status(p);
  
  switch (p->status)
    {
    case 0: case -2:
      if (check_pend(p->sock,SLC_READ)) return 0;
      else return QERR_NC;
    case -1: return QERR_NC;
    }
    
    return 0;
}

static int ip_read(ipdev_t *sd, void *buf, int pno)
{  
  long count=pno;  
  long ci;
  long res;
  char *c;

  ci=0;
  c=buf;
  
  res=recv(sd->sock, buf, pno, MSG_DONTWAIT);

  if (res<0)
	  res=qmaperr();
  return res;
}


static int ip_write(ipdev_t *sd, void *buf, int pno)
{
  long count=pno;
  long res;

    res=write(sd->sock,buf,pno);
  if (res<0) res=qmaperr();
  return res;
}


#define QERRNO(__qerr, __res) __qerr = (__res < 0) ? qmaperr() : __res

void net_convert(struct netent *h1, struct netent *h2)
{
    int i,m,n;
    char *h,*hql;
    
    h = (char *)h2 + sizeof(struct netent);
    hql = h - (int)theROM;
    WL((w32 *)&h2->n_name, (w32)hql);
    
    m = 1+strlen(h1->n_name);
    strcpy(h, h1->n_name);
    m = (m + 3) & ~3;
    h = h + m;
    hql = hql + m;
    
    {
        char **q,*s,*sql;
        int l;
        
        s = h;
        for(q = h1->n_aliases;*q; q++)
        {
            strcpy(s, *q);
            s = s + 1 + strlen(s);
        }

        l = s-h;
        l = (l + 3) & ~3;

        sql = hql + l;
        s = h + l;
        WL((w32*)&h2->n_aliases, (w32)sql);

        for(q = h1->n_aliases;*q; q++)
        {
            WL((w32*)s, (w32)hql);
            s += sizeof(int);
            hql = hql + 1 + strlen(*q);
        }
	WL((w32*)s,0);
    }

    WL(&h2->n_addrtype, h1->n_addrtype);
    WL((w32*)&h2->n_net, h1->n_net);
    
}

void proto_convert(struct protoent *h1, struct protoent *h2)
{
    int i,m,n;
    char *h,*hql;
    
    h = (char *)h2 + sizeof(struct protoent);
    hql = h - (int)theROM;
    WL((w32 *)&h2->p_name, (w32)hql);
    m = 1+strlen(h1->p_name);
    strcpy(h, h1->p_name);
    m = (m + 3) & ~3;
    h = h + m;
    hql = hql + m;
    
    {
        char **q,*s,*sql;
        int l;
        
        s = h;
        for(q = h1->p_aliases;*q; q++)
        {
            strcpy(s, *q);
            s = s + 1 + strlen(s);
        }

        l = s-h;
        l = (l + 3) & ~3;

        sql = hql + l;
        s = h + l;
        WL((w32*)&h2->p_aliases, (w32)sql);
        for(q = h1->p_aliases;*q; q++)
        {
            WL((w32*)s, (w32)hql);
            s += sizeof(int);
            hql = hql + 1 + strlen(*q);
        }
	WL((w32*)s,0);
    }
    WL(&h2->p_proto, h1->p_proto);
}

void serv_convert(struct servent *h1, struct servent *h2)
{
    int i,m,n;
    char *h,*hql;
    
    h = (char *)h2 + sizeof(struct servent);
    hql = h - (int)theROM;
    WL((w32 *)&h2->s_name, (w32)hql);
    m = 1+strlen(h1->s_name);
    strcpy(h, h1->s_name);

    h = h + m;
    hql = hql + m;
    WL((w32 *)&h2->s_proto, (w32)hql);
    m = 1+strlen(h1->s_proto);
    strcpy(h, h1->s_proto);
  
    m = (m + 3) & ~3;
    h = h + m;
    hql = hql + m;
    
    {
        char **q,*s,*sql;
        int l;
        
        s = h;
        for(q = h1->s_aliases;*q; q++)
        {
            strcpy(s, *q);
            s = s + 1 + strlen(s);
        }

        l = s-h;
        l = (l + 3) & ~3;

        sql = hql + l;
        s = h + l;
        WL((w32*)&h2->s_aliases, (w32)sql);
        for(q = h1->s_aliases;*q; q++)
        {
            WL((w32*)s, (w32)hql);
            s += sizeof(int);
            hql = hql + 1 + strlen(*q);
        }
	WL((w32*)s,0);
    }
    WL(&h2->s_port, ntohs(h1->s_port));
}

void host_convert(struct hostent *h1, struct hostent *h2)
{
  int i,m,n;
  char *h,*hql;
  int tmp;
  
  h = (char *)h2 + sizeof(struct hostent); /* start of free space */
  hql = h - (int)theROM;                   /* address in QDOS */
  WL((w32 *)&h2->h_name, (w32)hql);        /* set address for name */
  m = 1+strlen(h1->h_name);                /* length + 0 */
  strcpy(h, h1->h_name);                   /* copy in name */
  m = (m + 3) & ~3;                        /* get long address */
  h = h + m;                               /* next free */
  hql = hql + m;                           /* next free in QDOS speak */
  WL((w32 *)&h2->h_addr_list, (w32)hql);   /* set addr list for QDOS */
    for(i = 0; h1->h_addr_list[i];i++)        /* loop for addresses */
            ; /* count aliases */
    n = i+1;
    
    for(i = 0; h1->h_addr_list[i];i++)        /* loop for addresses  */
    {
        int x;
        memcpy(&x,h1->h_addr_list[i],sizeof(int)); /* ip in net format */
        WL((w32 *)h, (w32)(hql+n*sizeof(int))); /* set up address */
        memcpy(h+n*sizeof(int),&x,sizeof(int)); /* copy in net order addr */
        h += sizeof(int);                     /* next free */
        hql += sizeof(int);                   /* ditto */
    } 

    /* Ensure list is terminated */

    WL((w32 *)h, 0); /* set up address */
    
    h += n*sizeof(int);
    hql += n*sizeof(int);    
    
    {
        char **q,*s,*sql;
        int l;
        
        s = h;
        for(q = h1->h_aliases;*q; q++)
        {
            strcpy(s, *q);
            s = s + 1 + strlen(s);
        }

        l = s-h;
        l = (l + 3) & ~3;

        sql = hql + l;
        s = h + l;
        WL((w32*)&h2->h_aliases, (w32)sql);
        for(q = h1->h_aliases;*q; q++)
        {
            WL((w32*)s, (w32)hql);
            s += sizeof(int);
            hql = hql + 1 + strlen(*q);
        }
        WL((w32*)s,0);
        /* *(w32*)s = 0;*/
    }
    WL(&h2->h_addrtype, h1->h_addrtype);
    WL(&h2->h_length, h1->h_length);
}

static int ip_gethostbyname(const char *name, struct hostent *h)
{
    int qerr = 0;
    struct hostent *ph;
    
    ph = gethostbyname(name);
    if(ph)
    {
        host_convert(ph, h);
    }
    else
    {
        qerr = -15;
    }
    return qerr;
}

static int ip_gethostbyaddr(const char *addr, int len, int type,
                             struct hostent *h)
{
    int qerr = 0;
    struct hostent *ph;
    
    ph = gethostbyaddr(addr, len, type);
    if(ph)
    {
        host_convert(ph, h);
    }
    else
    {
        qerr = -15;
    }
    return qerr;
}

static int ip_sethostent(int stayopen)
{
    sethostent(stayopen);
    return 0;
}

static int ip_endhostent(void)
{
    endhostent();
    return 0;
}
    
static int ip_h_errno(int *s)
{
    *s = h_errno;
    return 0;
}

static int ip_h_strerror(char *s)
{
    switch(h_errno)
    {
        case HOST_NOT_FOUND:
            strcpy(s,"Host not found");
            break;
        case NO_ADDRESS:
            strcpy(s,"No address");            
            break;
        case NO_RECOVERY:
            strcpy(s,"No recovery");            
            break;
        case TRY_AGAIN:
            strcpy(s,"Try again");            
            break;
    }

    return 0;
}

static int ip_gethostname (char *buf, int len)
{
    int qerr,res;
    res = gethostname(buf, len);
    QERRNO(qerr,res);
    return qerr;
}

static int ip_getdomainname (char *buf, int len)
{
    int qerr,res;
    res = getdomainname(buf, len);
    QERRNO(qerr,res);
    return qerr;
}

static int ip_getservent(struct servent *s)
{
    int qerr = 0;
    struct servent *ps;
    
    ps = getservent();
    if(ps)
        serv_convert(ps, s);
    else
        qerr = -15;
    return qerr;
}

static int ip_getservbyname(const char *name, const char *proto,
                            struct servent *s)
{
    int qerr = 0;
    struct servent *ps;
    
    ps = getservbyname (name, proto);
    if(ps)
        serv_convert(ps, s);
    else
        qerr = -15;
    return qerr;
}

static int ip_getservbyport(int port, const char *proto, struct servent *s)
{
    int qerr = 0;
    struct servent *ps;
    
    ps = getservbyport (port, proto);
    if(ps)
        serv_convert(ps, s);
    else
        qerr = -15;
    return qerr;
}

static int ip_setservent(int stayopen)
{
    setservent(stayopen);
    return 0;
}

static int ip_endservent(void)
{
    endservent();
    return 0;
}

static int ip_getnetent(struct netent *n)
{
    int qerr = 0;
    struct netent *pn;
    
    pn = getnetent();
    if(pn)
        net_convert(pn, n);
    else
        qerr = -15;
    return qerr;
}

static int ip_getnetbyname(const char *name, struct netent *n)
{
    int qerr = 0;
    struct netent *pn;
    
    pn = getnetbyname (name);
    if(pn)
        net_convert(pn, n);
    else
        qerr = -15;
    return qerr;
}


static int ip_getnetbyaddr(unsigned long net, int type, struct netent *n)
{
    int qerr = 0;
    struct netent *pn;
    
    pn = getnetbyaddr(net, type);
    if(pn)
        net_convert(pn, n);
    else
        qerr = -15;
    return qerr;
}

static int ip_setnetent(int stayopen)
{
    setnetent(stayopen);
    return 0;
}

static int ip_endnetent(void)
{
    endnetent();
    return 0;
}

static int ip_getprotoent(struct protoent *p)
{
    int qerr = 0;
    struct protoent *pp;
    
    pp = getprotoent();
    if(pp)
        proto_convert(pp, p);        
    else
        qerr = -15;
    return qerr;
}

static int ip_getprotobyname(const char *name, struct protoent *p)
{
    int qerr = 0;
    struct protoent *pp;
    
    pp = getprotobyname (name);
    if(pp)
        proto_convert(pp, p);
    else
        qerr = -15;
    return qerr;
}

static int ip_getprotobynumber(int proto, struct protoent *p)
{
    int qerr = 0;
    struct protoent *pp;
    
    pp = getprotobynumber(proto);
    if(pp)
        proto_convert(pp, p);        
    else
        qerr = -15;
    return qerr;
}

static int ip_setprotoent(int stayopen)
{
    setprotoent(stayopen);
    return 0;
}

static int ip_endprotoent(void)
{
    endprotoent();
    return 0;
}

static int ip_inet_aton(const char *cp, struct in_addr *inp, int *res)
{
#ifdef __linux__
    *res = inet_aton(cp, inp);
    return 0;
#else
    *res= inet_addr(cp);
    if (*res != -1)
      {
	*res=0;
	inp->s_addr=*res;
      }
    
    return 0;
#endif
}

static int ip_inet_addr(const char *cp, unsigned long int *u)
{
    *u = inet_addr(cp);
    return 0;
}

static int ip_inet_ntoa(struct in_addr *in, char *a)
{
    char *pa;
    int qerr = 0;
    
    pa = inet_ntoa(*in);
    if(pa)
        strcpy(a,pa);
    else
        qerr = -15;
    return qerr;
}

static int ip_inet_makeaddr(int net, int host, struct in_addr *in)
{
    *in = inet_makeaddr(net, host);
    return 0;
}

static int ip_inet_lnaof(struct in_addr *in, unsigned long int *u)
{
    *u = inet_lnaof(*in);
    return 0;
}
    
static int ip_inet_netof(struct in_addr *in, unsigned long int *u)
{
    *u = inet_netof(*in);
    return 0;
}


static int ip_recv(ipdev_t *p, void *buf, long blen, int flag,
            struct sockaddr *from, int *flen)
{
    int qerr,res;

    if(from)
    {
        res = recvfrom(p->sock, buf, blen, flag, from, flen);
    }
    else
    {
        res = recv(p->sock, buf, blen, flag);        
        if(res == -1) strerror(errno);
    }
    QERRNO(qerr,res);
    return qerr;
}

    static int ip_send (ipdev_t *p, void *buf, long blen, int flag,
             struct sockaddr *from, int flen)
{
    int qerr,res;
    
    if(from)
    {
        res = sendto (p->sock, buf, blen, flag, from, flen);
    }
    else
    {
        res = send (p->sock, buf, blen, flag);
    }
    
    QERRNO(qerr,res);
    return qerr;
}

static int ip_fcntl(ipdev_t *p, int act, int val)
{
    int qerr,res;

    res = fcntl(p->sock, act, val);
    QERRNO(qerr,res);
    return qerr;  
}

static int ip_ioctl(ipdev_t *p, int act, w32 *val)
{
    int qerr,res;
    *val = RL(val);
    res = ioctl(p->sock, act, val);
    *val = RL(val);
    QERRNO(qerr,res);
    return qerr;  
}

static int ip_bind (ipdev_t *ip, struct sockaddr *sa, int len)
{
    int res,qerr;
    char *s1;


    res = bind (ip->sock, sa, len);

    QERRNO(qerr,res);
    return qerr;
}

static int ip_connect (ipdev_t *ip, struct sockaddr *sa, int len)
{
    int res,qerr;
        
    if (ip->status==0)
      return QERR_IU;   /* already connected */
    
    res = connect (ip->sock, sa, len);
    if ( res<0 )
      {
	if (errno==EINTR || errno == EINPROGRESS || errno == EALREADY)
	  {
	    res=0;
	  ip->status=-1;  /* there is hope ...*/
	  }
	else
	  {
	    perror("ip_connect");
	    ip->status=-2;  /* hopeless  */
	  }
      }
    else ip->status=0;
    
    QERRNO(qerr,res);
    return qerr;
}

static int ip_listen (ipdev_t *ip, int len)
{
    int res,qerr;      
    res = listen (ip->sock, len);
    QERRNO(qerr,res);
    return qerr;
}

static int endianise(int optname, void *optval)
{
    switch(optname)
    {
        case SO_LINGER:
          return -1;
          break;
        default:
            *(uint32_t *)optval = RB_SwapBE32(*(uint32_t *)optval);
            break;
    }
    return 0;
}

static int ip_getsockopt(ipdev_t *ip, int level, int optname,
                         void *optval, socklen_t *olen)
{
    int res,qerr;
    xso_q2x(level, optname, optval, 0);
    res = getsockopt(ip->sock, level, optname, optval, olen);
    xso_x2q(level, optname, optval, *olen);
    if(endianise(optname, optval)) return -1;
    QERRNO(qerr,res);
    return qerr;
}

static int ip_setsockopt(ipdev_t *ip, int level, int optname,
                         void *optval, socklen_t len)
{
    int res,qerr;
    int one = 1;
    
    if(endianise(optname, optval)) return -1;
    xso_q2x(level, optname, optval, len);
    res = setsockopt(ip->sock, level, optname, optval, len);
    xso_x2q(level, optname, optval, len);
    if(endianise(optname, optval)) return -1;
    QERRNO(qerr,res);
    return qerr;
}

static int ip_getsockname (ipdev_t *ip, struct sockaddr *sa, int *len)
{
    int res,qerr;
    res = getsockname (ip->sock, sa, len);
    QERRNO(qerr,res);
    return qerr;
}

static int ip_getpeername (ipdev_t *ip, struct sockaddr *sa, int *len)
{
    int res,qerr;
    res = getpeername (ip->sock, sa, len);
    QERRNO(qerr,res);
    return qerr;
}

static int ip_shutdown (ipdev_t *ip, int how)
{
    int res,qerr;
    res = shutdown (ip->sock, how);

    if (res==0 && how==2)
      ip->status=-2;
    
    QERRNO(qerr,res);
    return qerr;
}

struct sockaddr *setsockaddr(ipdev_t *priv, long qladdr)
{
    struct sockaddr *sa;
    
    if(qladdr)
    {
        sa = (struct sockaddr *)((Ptr)theROM+qladdr);
        WW((w16*)&((struct sockaddr_in *)sa)->sin_family,
           (w16)((struct sockaddr_in *)sa)->sin_family);
    }
    else
    {
        sa = (struct sockaddr *)&priv->name;
    }
    return sa;
}

void resetsockaddr(ipdev_t *priv, struct sockaddr *sa)
{
    if(sa != (struct sockaddr *)&priv->name)
    {
        WW((w16*)&((struct sockaddr_in *)sa)->sin_family,
           (w16)((struct sockaddr_in *)sa)->sin_family);
    }
}


void ip_io(int id, void *p )
{
    ipdev_t *priv=p;
    int op=(w8)reg[0];
    int res, count, flag = 0;
    struct sockaddr *sa = NULL;
    w32 *params;
    w32 qaddr;
    socklen_t len;
    
    len = (reg[1]) ? reg[1] : sizeof(struct sockaddr_in);
    count = reg[2];

    if (priv->status == -1)
      check_status(priv);
    if (priv->status == -1)
      {
	reg[0]=QERR_NC;
	return;
      }

    switch(op)
    {
        case IP_GETDOMAIN:
            *reg = ip_getdomainname ((Ptr)theROM+aReg[1], count);
            break;
        case IP_GETHOSTNAME:
            *reg = ip_gethostname ((Ptr)theROM+aReg[1], count);
            break;
        case IP_GETSOCKNAME:
            *reg = ip_getsockname(priv, (Ptr)theROM+aReg[1], &count);
            reg[1] = count;
            break;
        case IP_GETPEERNAME:
            *reg = ip_getpeername(priv, (Ptr)theROM+aReg[1], &count);
            reg[1] = count;
            break;
        case IP_GETHOSTBYNAME:
            *reg = ip_gethostbyname((Ptr)theROM+aReg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_GETHOSTBYADDR:
            *reg = ip_gethostbyaddr((Ptr)theROM+aReg[1],
                                    reg[1], reg[2], (Ptr)theROM+aReg[2]);
            break;
        case IP_SETHOSTENT:
            *reg = ip_sethostent(reg[1]);
            break;
        case IP_ENDHOSTENT:
            *reg = ip_endhostent();
            break;
        case IP_H_ERRNO:
            *reg = ip_h_errno(&res);
            reg[1] = res;
            break;
        case IP_H_STRERROR:
            *reg = ip_h_strerror((Ptr)theROM+aReg[1]);
            break;
        case IP_GETSERVENT:
            *reg = ip_getservent((Ptr)theROM+aReg[2]);
            break;
        case IP_GETSERVBYNAME:
            *reg = ip_getservbyname((Ptr)theROM+aReg[1],
                                    (Ptr)theROM+reg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_GETSERVBYPORT:
            *reg = ip_getservbyport(reg[1],
                                    (Ptr)theROM+aReg[3], (Ptr)theROM+aReg[2]);
            break;
        case IP_SETSERVENT:
            *reg = ip_setservent(reg[1]);
            break;
        case IP_ENDSERVENT:
            *reg = ip_endservent();
            break;
        case IP_GETNETENT:
            *reg = ip_getnetent((Ptr)theROM+aReg[2]);
            break;
        case IP_GETNETBYNAME:
            *reg = ip_getnetbyname((Ptr)theROM+aReg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_GETNETBYADDR:
            *reg = ip_getnetbyaddr(reg[1], reg[2], (Ptr)theROM+aReg[2]);
            break;
        case IP_SETNETENT:
            *reg = ip_setnetent(reg[1]);
            break;
        case IP_ENDNETENT:
            *reg = ip_endnetent();            
            break;
        case IP_GETPROTOENT:
            *reg = ip_getprotoent((Ptr)theROM+aReg[2]);
            break;
        case IP_GETPROTOBYNAME:
            *reg = ip_getprotobyname((Ptr)theROM+aReg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_GETPROTOBYNUMBER:
            *reg = ip_getprotobynumber(reg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_SETPROTOENT:
            *reg = ip_setprotoent(reg[1]);
            break;
        case IP_ENDPROTOENT:
            *reg = ip_endprotoent();            
            break;
        case IP_INET_ATON:
            *reg =  ip_inet_aton((Ptr)theROM+aReg[1], (Ptr)theROM+aReg[2],
                                 &res);
            reg[1] = res;
            break;
        case IP_INET_NETWORK:
#ifdef QM_BIG_ENDIAN
            *reg =  ip_inet_network ((Ptr)theROM+aReg[1],
                                     (unsigned long *)&res);
            reg[1] = res;
            break;
#endif
        case IP_INET_ADDR:
            *reg =  ip_inet_addr ((Ptr)theROM+aReg[1], (unsigned long *)&res);
            reg[1] = res;
            break;
        case IP_INET_NTOA:
            *reg = ip_inet_ntoa( (Ptr)theROM+aReg[1], (Ptr)theROM+aReg[2]);
            break;
        case IP_INET_MAKEADDR:
            *reg = ip_inet_makeaddr(reg[1], reg[2], (Ptr)theROM+aReg[2]);
            break;
        case IP_INET_LNAOF:
            *reg = ip_inet_lnaof((Ptr)theROM+aReg[1], (unsigned long *)&res);
            reg[1] = res;            
            break;
        case IP_INET_NETOF:
            *reg = ip_inet_netof((Ptr)theROM+aReg[1],(unsigned long *) &res);
            reg[1] = res;  
            break;
        case IP_FCNTL:
            *reg = ip_fcntl(priv, reg[1], reg[2]);
            break;
        case IP_IOCTL:
            *reg = ip_ioctl(priv, reg[1], ((Ptr)theROM+aReg[1]));
            break;
        case IP_BIND:
            sa = setsockaddr(priv, aReg[2]);
            *reg = ip_bind (priv, sa, len);
            resetsockaddr(priv, sa);
            break;
        case IP_CONNECT:
            sa = setsockaddr(priv, aReg[2]);
            *reg = ip_connect (priv, sa, len);            
            break;
        case IP_LISTEN:
            *reg = ip_listen(priv, reg[1]);
            break;

        case IP_SEND:
            qaddr = aReg[1];
            res = ip_send (priv,(Ptr)theROM+aReg[1], count, reg[1], NULL, 0);
            if(res >= 0)
            {
                reg[1] = res;
                *reg = 0;
                aReg[1] = qaddr+res;
            }
            else *reg = res;
            break;
        case IP_SENDTO:
            params = (long *)aReg[2];
            len = ReadLong((w32)(params+1));
            sa = setsockaddr(priv, ReadLong((w32)params));
            *reg = ip_send (priv,(Ptr)theROM+aReg[1], count, reg[1], sa, len);
            resetsockaddr(priv, sa);
            break;
        case IP_RECV:
            qaddr = aReg[1];
            res = ip_recv (priv,(Ptr)theROM+aReg[1], count,
                            reg[1], NULL, NULL);
	    
            if(res >= 0)
            {
                reg[1] = res;
                *reg = 0;
            }
            else *reg = res;
            break;
        case IP_RECVFM:
            params = (long *)aReg[2];
            len = ReadLong((w32)(params+1));
            sa = setsockaddr(priv, ReadLong((w32)params));            
            *reg = ip_recv (priv, (Ptr)theROM+aReg[1], count, reg[1],
                            sa, &len);
            resetsockaddr(priv, sa);
            reg[1] = len;
            break;
        case IP_GETOPT:
            len = reg[1];
            *reg = ip_getsockopt(priv, reg[2], (int)aReg[2],
                                 (Ptr)theROM+aReg[1], &len);
            reg[1] = (uint32_t)len;
            break;
        case IP_SETOPT:
            *reg = ip_setsockopt(priv, reg[2], (int)aReg[2],
                                 (Ptr)theROM+aReg[1], reg[1]);
            break;
         case IP_SHUTDWN:
            ip_shutdown(priv, reg[1]);
            break;
         case IP_ERRNO:
	   *reg = 0;
	   reg[1] = priv->lerrno;
	   break;
    default:
            io_handle(ip_read, ip_write, ip_pend, priv);
	    return;

    }
    if(*reg < 0)
      priv->lerrno = c68err(errno);
    else priv->lerrno = 0;
}

void ip_close(int id, void *p )
{
    ipdev_t *priv=p;

    close(priv->sock);
    free(priv);
}

#endif /* IPDEV */
