#include <stdio.h>
#include <string.h>
#include "patchfile.h"

void dump_patch_header( const patch_hdr_t *hdr ) {
	printf("Header version:  %08X\n", hdr->header_ver);
	printf("Update revision: %08X\n", hdr->update_rev);
	printf("Date:            %08X\n", hdr->date_bcd);
	printf("Processor sign.: %08X\n", hdr->proc_sig);
	printf("Checksum:        %08X\n", hdr->checksum);
	printf("Loader revision: %08X\n", hdr->loader_ver);
	printf("Processor flags: %08X\n", hdr->proc_flags);
	printf("Data size:       %08X\n", hdr->data_size);
	printf("Total size:      %08X\n", hdr->total_size);
}

void dump_patch_body( const patch_body_t *body ) {
	const uint32_t *groupbase;
	uint32_t grp_or[MSRAM_GROUP_SIZE];
	int i,j;
	printf("MSRAM: \n");
	memset( grp_or, 0, sizeof grp_or );
	for ( i = 0; i < MSRAM_GROUP_COUNT; i++ ) {
		groupbase = body->msram + MSRAM_GROUP_SIZE * i;
		printf("\t%04X: %08X %08X %08X %08X %08X %08X %08X %08X\n",
			i * 8,
			groupbase[0], groupbase[1], groupbase[2], groupbase[3],
			groupbase[4], groupbase[5], groupbase[6], groupbase[7]);
		for ( j = 0; j < MSRAM_GROUP_SIZE; j++ )
			grp_or[j] |= groupbase[j];
	}
	groupbase = grp_or;
	printf("\n\tOR  : %08X %08X %08X %08X %08X %08X %08X %08X\n",

			groupbase[0], groupbase[1], groupbase[2], groupbase[3],
			groupbase[4], groupbase[5], groupbase[6], groupbase[7]);
	printf("Control register ops: \n");
	for ( i = 0; i < PATCH_CR_OP_COUNT; i++ ) {
		printf("\tAddr: %08X  Mask: %08X  Value: %08X\n",
			body->cr_ops[i].address,
			body->cr_ops[i].mask,
			body->cr_ops[i].value);
	}
}
