//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#define COW_MAGIC 0x4f4f4f4d  /* MOOO */
#define COW_VERSION 2

struct cow_header_v2 {
    uint32_t magic;
    uint32_t version;
    char backing_file[1024];
    int32_t mtime;
    uint64_t size;
    uint32_t sectorsize;
};

