//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

/* define memory access fns */

#ifndef _MEMACCESS_H
#define _MEMACCESS_H

extern rw8 ReadByte(aw32 addr);
extern rw16 ReadWord(aw32 addr);
extern rw32 ReadLong(aw32 addr);
extern void WriteByte(aw32 addr,aw8 d);
extern void WriteWord(aw32 addr,aw16 d);
extern void WriteLong(aw32 addr,aw32 d);

extern rw8 ModifyAtEA_b(ashort mode,ashort r);
extern rw16 ModifyAtEA_w(ashort mode,ashort r);
extern rw32 ModifyAtEA_l(ashort mode,ashort r);
extern void RewriteEA_b(aw8 d);
extern void rwb_acc(w8 d);
extern void RewriteEA_w(aw16 d);
extern void rww_acc(w16 d);
extern void RewriteEA_l(aw32 d);
extern void rwl_acc(w32 d);

#define QL_ROM_BASE             0x0000
#define QL_ROM_SIZE             0x10000
#define QL_INTERNAL_IO_BASE     0x18000
#define QL_INTERNAL_IO_SIZE     0x4000

#endif /* _MEMACCESS_H */
