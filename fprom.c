#include <stdint.h>
#include <assert.h>
#include <string.h>

#define ROM_FLAG_VAL (0x13371337)

static uint32_t FPROM[512];

__attribute__((constructor))
static void init_fprom() {
	int i;
	/* Fill the FPROM table with a flag value so we know what addresses are
	 * unimplemented */
	for ( i = 0; i < 512; i++ )
		FPROM[i] = ROM_FLAG_VAL;

	/* Load the FPROM table with the data provided in fprom_data.c */
#include "fprom_data.c"
	;
}

/**
 * Checks if a given FPROM address is implemented in this program
 * @param addr     Address into the FPROM to check (truncated to 9 bits)
 * @return         Non-zero if FPROM[addr mod 512] is implemented.
 */
int fprom_exists( uint32_t addr ) {
	uint32_t v = FPROM[ addr & 0x1FF ];
	return v != ROM_FLAG_VAL;
}

/**
 * Gets the low 32 bits of the floating point constant ROM entry with address
 * addr mod 512.
 * @param addr     Address into the FPROM to check (truncated to 9 bits)
 * @return         The low 32 bits of FPROM[addr mod 512].
 */
uint32_t fprom_get( uint32_t addr ) {
	assert ( fprom_exists( addr ) );
	return FPROM[ addr & 0x1FF ];

}
