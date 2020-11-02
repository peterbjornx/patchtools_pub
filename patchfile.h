#ifndef __patchfile_h__
#define __patchfile_h__
#include <stdint.h>

#define MSRAM_QWORD_COUNT (0x54)
#define MSRAM_DWORD_COUNT (MSRAM_QWORD_COUNT * 2)
#define MSRAM_GROUP_SIZE  (0x8)
#define MSRAM_GROUP_COUNT (MSRAM_DWORD_COUNT/8)
#define PATCH_CR_OP_COUNT (0x10)
#define MSRAM_BASE_ADDRESS (0xFEB)

typedef struct __attribute__((packed)) {
	uint32_t      header_ver;
	uint32_t      update_rev;
	uint32_t      date_bcd;
	uint32_t      proc_sig;
	uint32_t      checksum;
	uint32_t      loader_ver;
	uint32_t      proc_flags;
	uint32_t      data_size;
	uint32_t      total_size;
	uint8_t       reserved[12];
} patch_hdr_t;

typedef struct __attribute__((packed)) {
	uint32_t      address;
	uint32_t      mask;
	uint32_t      value;
	uint32_t      integrity;
} patch_cr_op_t;

typedef struct __attribute__((packed)) {
	uint32_t      key_seed;
	uint32_t      resvd_0;
	uint32_t      msram[ MSRAM_DWORD_COUNT ];
	uint32_t      msram_integrity;
	uint32_t      resvd_1;
	patch_cr_op_t cr_ops[ PATCH_CR_OP_COUNT ];
} epatch_body_t;

typedef struct {
	uint32_t      msram[ MSRAM_DWORD_COUNT ];
	patch_cr_op_t cr_ops[ PATCH_CR_OP_COUNT ];
} patch_body_t;

typedef struct __attribute__((packed)) {
	patch_hdr_t   header;
	epatch_body_t body;
} epatch_file_t;

#endif
