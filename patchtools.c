#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>   // for uint32_t
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <libgen.h>
#include "patchtools.h"

char fmt_buf[4096];
char *patch_filename;
char *patch_name;
epatch_file_t *patch_in;
patch_body_t patch_body;
epatch_file_t epatch_out;
uint8_t data_in[2048];

/* Command line flags */
int extract_patch_flag, dump_patch_flag, create_patch_flag, help_flag;

/* Command line arguments */
char *patch_path;
char *config_path;
char *msram_path;
uint32_t patch_seed;

void usage( const char *reason ) {
	fprintf( stderr, "%s\n", reason );
	fprintf( stderr,
	"\tpatchtools -h\n" );
	fprintf( stderr,
	"\tpatchtools [-dec] [-p <patch.dat>] [-i <config.txt>]\n\n" );

	if ( !help_flag )
		exit( EXIT_FAILURE );

	fprintf( stderr,
	"\t\n"
	"\tProgram for encrypting and decrypting Pentium II microcode patches\n"
	"\twritten by Peter Bosch <public@pbx.sh>. Patchfiles produced by    \n"
	"\tthis program might crash or damage the system they are loaded onto\n"
	"\tand the author takes no responsibility for any damages resulting  \n"
	"\tfrom use of the software.\n"
	"\t\t-h                Print this message and exit\n"
	"\t\t\n"
	"\t\t-e                Extract a patch to a configuration and \n"
	"\t\t                  MSRAM hexdump file\n"
	"\t\t\n"
	"\t\t-c                Create a patch from a configuration and\n"
	"\t\t                  MSRAM hexdump file\n"
	"\t\t\n"
	"\t\t-d                Dump the patch contents and keys to the\n"
	"\t\t                  console after encrypting or decrypting.\n"
	"\t\t\n"
	"\t\t-p <patch.dat>    Specifies the path of the patchfile to \n"
	"\t\t                  create or decrypt. When encrypting this\n"
	"\t\t                  option is not required as the program  \n"
	"\t\t                  will use the path of the configuration \n"
	"\t\t                  file to generate the output path.\n"
	"\t\t\n"
	"\t\t-i <config.txt>   Specifies the path of the config file  \n"
	"\t\t                  to use or extract. When extracting this\n"
	"\t\t                  option is not required as the program  \n"
	"\t\t                  will use the path of the patch file to \n"
	"\t\t                  generate the output path.\n");
}

void parse_args( int argc, char *const *argv ) {
	char opt;
	while ( (opt = getopt( argc, argv, ":p:i:dech" )) != -1 ) {
		switch( opt ) {
			case 'p':
				patch_path = strdup( optarg );
				break;
			case 'i':
				config_path = strdup( optarg );
				break;
			case 'd':
				dump_patch_flag = 1;
				break;
			case 'e':
				extract_patch_flag = 1;
				break;
			case 'c':
				create_patch_flag = 1;
				break;
			case 'h':
				help_flag = 1;
				break;
			case ':':
				usage("missing argument");
				break;
			default:
			case '?':
				usage("unknown argument");
				break;
		}
	}
}


void load_input_patch( void ) {
	char *patch_fn;

	/* Ensure we have a path */
	if ( !patch_path )
		usage("missing patch path");

	/* Load the patch */
	read_file( patch_path, data_in, sizeof data_in );
	patch_in = (epatch_file_t *) data_in;

	/* Get the patch filename */
	strncpy( fmt_buf, patch_path, sizeof fmt_buf );
	patch_fn = basename( fmt_buf );
	patch_filename = strdup( patch_fn );

	/* Get the patch name */
	strncpy( fmt_buf, patch_filename, sizeof fmt_buf );
	patch_name = strtok( patch_filename, "." );
	patch_name = strdup( patch_name );

	/* Decrypt the patch */
	decrypt_patch_body(
		&patch_body,
		&patch_in->body,
		patch_in->header.proc_sig);
	//TODO: Header only mode?

	patch_seed = patch_in->body.key_seed;

}

void write_output_patch( void ) {

	/* Ensure we have a path */
	if ( !patch_path )
		usage("missing patch path");

	/* Encrypt the patch */
	encrypt_patch_body(
		&epatch_out.body,
		&patch_body,
		patch_in->header.proc_sig,
		patch_seed);

	/* Assemble the header */
	memcpy( &epatch_out.header, &patch_in->header, sizeof(patch_hdr_t) );

	/* Write the file */
	write_file( patch_path, &epatch_out, sizeof(epatch_file_t) );

}

void cleanup( void ) {
	if ( patch_path )
		free( patch_path );
	if ( patch_filename )
		free( patch_filename );
	if ( patch_name )
		free( patch_name );
	if ( config_path )
		free( config_path );
	if ( msram_path )
		free( msram_path );
}

void dump_patch( void ) {
	dump_patch_header( &patch_in->header );
	printf("Key seed: 0x%08X\n", patch_seed);
	dump_patch_body( &patch_body );
}

void extract_patch( void ) {
	size_t s;

	if ( !config_path ) {
		s = snprintf( fmt_buf, sizeof fmt_buf, "%s.txt", patch_name );
		if ( s < 0 )  {
			fprintf( stderr, "Could not generate output path!\n" );
			exit( EXIT_FAILURE );
		}
		config_path = strdup( fmt_buf );
	}

	if ( !msram_path ) {
		s = snprintf( fmt_buf, sizeof fmt_buf, "%s.hex", patch_name );
		if ( s < 0 )  {
			fprintf( stderr, "Could not generate output path!\n" );
			exit( EXIT_FAILURE );
		}
		msram_path = strdup( fmt_buf );
	}

	write_patch_config(
		&patch_in->header,
		&patch_body,
		config_path,
		msram_path,
		patch_seed );

	write_msram_file(
		&patch_body,
		msram_path );

}

/** current directory buffer for use by create_patch */
char current_dir[4096];

/**
 * Creates a new patch
 */
void create_patch( void ) {
	size_t s;
	char *config_fn, *config_dir;

	/* Ensure we have a path */
	if ( !config_path )
		usage("missing config path");

	/* Get the config filename */
	strncpy( fmt_buf, config_path, sizeof fmt_buf );
	config_fn = basename( fmt_buf );
	config_fn = strdup(config_fn);

	/* Get the patch name */
	patch_name = strtok( config_fn, "." );
	patch_name = strdup( patch_name );

	/* Get the config directory */
	strncpy( fmt_buf, config_path, sizeof fmt_buf );
	config_dir = dirname( fmt_buf );
	config_dir = strdup( config_dir );

	/* Determine patchfile path */
	if ( !patch_path ) {
		s = snprintf( fmt_buf, sizeof fmt_buf, "%s.dat", patch_name );
		if ( s < 0 )  {
			fprintf( stderr, "Could not generate output path!\n" );
			exit( EXIT_FAILURE );
		}
		patch_path = strdup( fmt_buf );
	}

	/* Set the patch data pointer */
	patch_in = (epatch_file_t *) data_in;

	/* Parse the configuration file */
	read_patch_config(
		&patch_in->header,
		&patch_body,
		config_path,
		&msram_path,
		&patch_seed );

	if ( !msram_path )
		usage("missing data path");

	/* Switching directories to allow the data path to be relative to the
	   config file path */
	getcwd( current_dir, sizeof current_dir );
	chdir( config_dir );

	/* Read the MSRAM input data */
	read_msram_file(
		&patch_body,
		msram_path );

	/* Restore the working directory */
	chdir( current_dir );

	free( config_dir );

	/* Encode and encrypt the patch */
	write_output_patch();

}

int main( int argc, char * const *argv ) {
	/* Parse the command line arguments */
	parse_args( argc, argv );

	if ( help_flag ) {
		/* The user requested the built in documentation */
		usage("");

	} else if ( create_patch_flag && !extract_patch_flag ) {
		/* We are to create a new patch */

		/* Load the input and encode it */
		create_patch();

		/* If the user requested a dump of the newly created patch,
		   produce it. */
		if ( dump_patch_flag )
			dump_patch();

	} else if ( dump_patch_flag || extract_patch_flag ) {
		/* The user requested to load and decrypt a patch */

		/* Creating and decrypting patches are mutually exclusive ops */
		if ( create_patch_flag )
			usage("invalid combination of modes");

		/* Load and decrypt the patch */
		load_input_patch();

		/* Dump the patch if requested */
		if ( dump_patch_flag )
			dump_patch();

		/* Extract the patch if requested */
		if ( extract_patch_flag )
			extract_patch();

	} else
		usage("no mode specified");

	/* Cleanup dynamically allocated memory  */
	cleanup();

	return EXIT_SUCCESS;
}
