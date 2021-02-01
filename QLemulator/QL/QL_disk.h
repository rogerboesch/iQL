//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#define	ERR_UNINITIALIZED_DISK	-9810
#define	ERR_NO_FILE_BLOCK		-9811

OSErr	AllocateDisk(int);
void	DeallocateDisk(void);
void	QDiskFlush(void);
Cond	QDiskPresent(void);
OSErr	QDiskEject(void);


