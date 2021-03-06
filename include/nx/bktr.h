#pragma once

#include "nx/ivfc.h"

#define MAGIC_BKTR 0x52544B42

typedef struct {
    uint64_t offset;
    uint64_t size;
    uint32_t magic; /* "BKTR" */
    uint32_t _0x14; /* Version? */
    uint32_t num_entries;
    uint32_t _0x1C; /* Reserved? */
} bktr_header_t;

typedef struct {
	ivfc_hdr_t ivfc_header;
	uint8_t _0xE0[0x18];
	bktr_header_t relocation_header;
	bktr_header_t subsection_header;
} bktr_superblock_t;

#pragma pack(push, 1)
typedef struct {
    uint64_t virt_offset;
    uint64_t phys_offset;
    uint32_t is_patch;
} bktr_relocation_entry_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t _0x0;
    uint32_t num_entries;
    uint64_t virtual_offset_end;
    bktr_relocation_entry_t entries[0x3FF0/sizeof(bktr_relocation_entry_t)];
    uint8_t padding[0x3FF0 % sizeof(bktr_relocation_entry_t)];
} bktr_relocation_bucket_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t _0x0;
    uint32_t num_buckets;
    uint64_t total_size;
    uint64_t bucket_virtual_offsets[0x3FF0/sizeof(uint64_t)];
    bktr_relocation_bucket_t buckets[];
} bktr_relocation_block_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint64_t  offset;
    uint32_t _0x8;
    uint32_t ctr_val;
} bktr_subsection_entry_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t _0x0;
    uint32_t num_entries;
    uint64_t physical_offset_end;
    bktr_subsection_entry_t entries[0x3FF];
} bktr_subsection_bucket_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t _0x0;
    uint32_t num_buckets;
    uint64_t total_size;
    uint64_t bucket_physical_offsets[0x3FF0/sizeof(uint64_t)];
    bktr_subsection_bucket_t buckets[];
} bktr_subsection_block_t;
#pragma pack(pop)