#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "patchfile.h"

void write_patch_config( 
	const patch_hdr_t *hdr, 
	const patch_body_t *body, 
	const char *filename,
	const char *msram_fn,
	uint32_t key_seed ) {
	FILE *file;
	int i;

	file = fopen(filename, "w");
	if ( !file ) {
		perror( "Could not open patch config output file" );
		exit( EXIT_FAILURE );
	}

	fprintf( file, "header_ver 0x%08X\n", hdr->header_ver );
	fprintf( file, "update_rev 0x%08X\n", hdr->update_rev );
	fprintf( file, "date_bcd   0x%08X\n", hdr->date_bcd );
	fprintf( file, "proc_sig   0x%08X\n", hdr->proc_sig );
	fprintf( file, "checksum   0x%08X\n", hdr->checksum );
	fprintf( file, "loader_rev 0x%08X\n", hdr->loader_ver );
	fprintf( file, "proc_flags 0x%08X\n", hdr->proc_flags );
	fprintf( file, "data_size  0x%08X\n", hdr->data_size );
	fprintf( file, "total_size 0x%08X\n", hdr->total_size );
	fprintf( file, "key_seed   0x%08X\n", key_seed );
	fprintf( file, "msram_file %s\n"    , msram_fn );

	for ( i = 0; i < PATCH_CR_OP_COUNT; i++ ) {
		fprintf( file,
		        "write_creg 0x%03X 0x%08X 0x%08X\n",
		        body->cr_ops[i].address,
		        body->cr_ops[i].mask,
		        body->cr_ops[i].value);	
	}

	fclose( file );
}

char line_buf[4096];

void read_patch_config(
	patch_hdr_t *hdr,
	patch_body_t *body,
	const char *filename,
	char **msram_fnp,
	uint32_t *key_seed ) {
	
	int i;
	char *par_n, *par_v, *par_v2, *par_v3;
	char *msram_fn;
	uint32_t addr, mask, data;
	FILE *file;
	msram_fn = NULL;

	file = fopen(filename, "r");
	if ( !file ) {
		perror( "Could not open patch config input file" );
		exit( EXIT_FAILURE );
	}
	
	i = 0;

	while ( fgets( line_buf, sizeof line_buf, file ) ) {
		par_n = strtok(line_buf, " ");
		if ( !par_n )
			continue;
		par_v = strtok(NULL, " \n");
		if ( !par_v ) {
			fprintf( stderr, 
				"Config key without value: \"%s\"\n", 
				par_n );
			exit( EXIT_FAILURE );
		}

		if ( strcmp( par_n, "header_ver" ) == 0 ) {
			hdr->header_ver = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "update_rev" ) == 0 ) {
			hdr->update_rev = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "date_bcd" ) == 0 ) {
			hdr->date_bcd = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "proc_sig" ) == 0 ) {
			hdr->proc_sig = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "checksum" ) == 0 ) {
			hdr->checksum = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "loader_rev" ) == 0 ) {
			hdr->loader_ver = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "proc_flags" ) == 0 ) {
			hdr->proc_flags = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "data_size" ) == 0 ) {
			hdr->data_size  = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "total_size" ) == 0 ) {
			hdr->total_size = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "key_seed" ) == 0 ) {
			*key_seed = strtol( par_v, NULL, 0 );
		} else if ( strcmp( par_n, "msram_file" ) == 0 ) {
			msram_fn = strdup( par_v );
		} else if ( strcmp( par_n, "write_creg" ) == 0 ) {
			par_v2 = strtok(NULL, " \n");
			par_v3 = strtok(NULL, " \n");
			if ( !(par_v2 || par_v3) ){
				fprintf( stderr, "Incomplete write_creg\n" );
				exit( EXIT_FAILURE );
			}
			addr = strtol( par_v,  NULL, 0 );
			mask = strtol( par_v2, NULL, 0 );
			data = strtol( par_v3, NULL, 0 );
			if ( addr & ~0x1FF ) {
				fprintf( stderr, 
					"Invalid creg address: 0x%03X\n", 
					addr );
				exit( EXIT_FAILURE );
			}
			if ( i >= PATCH_CR_OP_COUNT ) {
				fprintf( stderr, 
					"Too many write_creg statements\n");
				exit( EXIT_FAILURE );
			}
			body->cr_ops[i].address = addr;
		        body->cr_ops[i].mask = mask;
		        body->cr_ops[i].value = data;
			i++;
		} else {
			fprintf( stderr, "Unknown config key \"%s\"\n", par_n );
			exit( EXIT_FAILURE );
		}
	}
	
	fclose( file );

	*msram_fnp = msram_fn;

}

void write_msram_file( const patch_body_t *body, const char *filename ) {
	FILE *file;
	const uint32_t *groupbase;
	uint32_t grp_or[MSRAM_GROUP_SIZE];
	int i,j, base;

	file = fopen(filename, "w");
	if ( !file ) {
		perror( "Could not open MSRAM output file" );
		exit( EXIT_FAILURE );
	}

	base = MSRAM_BASE_ADDRESS * 8;

	memset( grp_or, 0, sizeof grp_or );
	for ( i = 0; i < MSRAM_GROUP_COUNT; i++ ) {
		groupbase = body->msram + MSRAM_GROUP_SIZE * i;
		fprintf(file,
			"%04X: %08X %08X %08X %08X %08X %08X %08X %08X\n",
			base + i * 8,
			groupbase[0], groupbase[1], groupbase[2], groupbase[3],
			groupbase[4], groupbase[5], groupbase[6], groupbase[7]);
		for ( j = 0; j < MSRAM_GROUP_SIZE; j++ )
			grp_or[j] |= groupbase[j];
	}

	fclose( file );

}

void read_msram_file( patch_body_t *body, const char *filename ) {
	char *ts;
	FILE *file;
	int addr, raddr;
	int g;
	uint32_t *groupbase;


	file = fopen(filename, "r");
	if ( !file ) {
		perror( "Could not open MSRAM input file" );
		exit( EXIT_FAILURE );
	}

	while ( fgets( line_buf, sizeof line_buf, file ) ) {
		ts = strtok(line_buf, ": ");
		if ( !ts )
			continue;
		addr = strtol( ts, NULL, 16 );
		if ( addr % 8 ) {
			fprintf( stderr, "Misaligned address in input :%08X\n",
				 addr );
			exit( EXIT_FAILURE );
		}
		if ( addr < MSRAM_BASE_ADDRESS * 8 ) {
			fprintf( stderr, 
				"Address not in MSRAM range :%08X\n",
				 addr );
			exit( EXIT_FAILURE );
		}
		raddr = ( addr / 8 ) - MSRAM_BASE_ADDRESS;
		if ( raddr >= MSRAM_GROUP_COUNT ) {
			fprintf( stderr, 
				"Address  not in MSRAM range :%08X\n", addr );
			exit( EXIT_FAILURE );
		}
		groupbase = body->msram + MSRAM_GROUP_SIZE * raddr;
		for ( g = 0; g < MSRAM_GROUP_SIZE; g++ ) {
			ts = strtok(NULL, " ");
			if ( !ts ) {
				fprintf( stderr, 
					"Incomplete data for address %04X", 
					raddr );
				exit( EXIT_FAILURE );
			}
			groupbase[g] = strtol( ts, NULL, 16 );
		}
	
	}

	fclose( file );

}

