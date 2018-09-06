#pragma once

#include "nx/fs.h"

#define IVFC_HEADER_SIZE 0xE0
#define IVFC_MAX_LEVEL 6
#define IVFC_MAX_BUFFERSIZE 0x4000

#define MAGIC_IVFC 0x43465649

typedef struct {
    uint64_t logical_offset;
    uint64_t hash_data_size;
    uint32_t block_size;
    uint32_t reserved;
} ivfc_level_hdr_t;

typedef struct {
    uint32_t magic;
    uint32_t id;
    uint32_t master_hash_size;
    uint32_t num_levels;
    ivfc_level_hdr_t level_headers[IVFC_MAX_LEVEL];
    uint8_t _0xA0[0x20];
    uint8_t master_hash[0x20];
} ivfc_hdr_t;

/* RomFS structs. */
#define ROMFS_HEADER_SIZE 0x00000050

typedef struct {
    uint64_t header_size;
    uint64_t dir_hash_table_offset;
    uint64_t dir_hash_table_size;
    uint64_t dir_meta_table_offset;
    uint64_t dir_meta_table_size;
    uint64_t file_hash_table_offset;
    uint64_t file_hash_table_size;
    uint64_t file_meta_table_offset;
    uint64_t file_meta_table_size;
    uint64_t data_offset;
} romfs_hdr_t;

typedef struct {
    uint32_t parent;
    uint32_t sibling;
    uint32_t child;
    uint32_t file;
    uint32_t hash;
    uint32_t name_size;
    char name[];
} romfs_direntry_t;

typedef struct {
    uint32_t parent;
    uint32_t sibling;
    uint64_t offset;
    uint64_t size;
    uint32_t hash;
    uint32_t name_size;
    char name[];
} romfs_fentry_t;

typedef struct {
    ivfc_hdr_t ivfc_header;
    uint8_t _0xE0[0x58];
} romfs_superblock_t;

