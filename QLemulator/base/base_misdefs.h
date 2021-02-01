//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

/* various prototype definitions missing in some compilers */

#ifdef NEED_STPCPY
__linux__
char * stpcpy(char *, char *);
#endif
