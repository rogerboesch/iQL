//
// (c) UQLX - see COPYRIGHT
// QVFS is (c) by HPR
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

/* this file implements something remotely similar */
/* hoping to become compatible sometimes */
/* currently this IS NOT HPR's QVFS*/

int qvf_init(int,void *); 
int  qvf_open(int,void **);
int  qvf_test(int,char *);
void qvf_close(int, void *);
void qvf_io(int, void *);


typedef struct QVF_PRIV
{
  int isdev;
  int fd;
  struct mdvFile f;
  struct HF_FCB fcb;
  char padd[4000];   /* longer than standard name in FCB !*/
} qvf_priv;

