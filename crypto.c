#include <stdint.h>
#include "rotate.h"
#include "crypto.h"

uint32_t crypto_key;
uint32_t crypto_LastCWord;
uint32_t crypto_state;

#ifdef USE_C_BLOCKFUNC
/**
 * The 'block cipher' used as the basis for the update encryption.
 * Basically a Galois LFSR.
 * Because earlier versions of the program contained bruteforce key search
 * functionality, the program also comes with an assembly language version of
 * this function that avoids branches and as such is much faster.
 */
uint32_t crypto_c_blockfunc( uint32_t plain, uint32_t key ) {
	uint32_t lfsr;
	int iter;

	/* Seed the LFSR with the plaintext */
	lfsr = plain;

	/* Run the LFSR for 37 clocks */
	for ( iter = 0; iter < 37; iter++ ) {
		lfsr = rotr32( lfsr, 0x1 );
		lfsr ^= (lfsr & 0x80000000) ? key : 0;
	}

	/* Return the LFSR state XOR the plaintext */
	return lfsr ^ plain;
}
#define crypto_blockfunc crypto_c_blockfunc
#endif

void crypto_init( uint32_t key, uint32_t iv ) {
	crypto_LastCWord = crypto_key = key;
	crypto_state = iv;
}

uint32_t crypto_getstate( void ) {
	return crypto_state;
}

/**
 * Decrypt a block using the mode used by Pentium II patches.
 * See https://twitter.com/peterbjornx/status/1321653489899081728
 */
uint32_t crypto_decrypt(uint32_t ciphertext) {
	uint32_t state;
	uint32_t plaintext;

	state = crypto_blockfunc( crypto_state, crypto_key ) ^ ciphertext;

	plaintext  = state ^ crypto_LastCWord;

	/* Keep track of the previous ciphertext block and state */
	crypto_LastCWord = ciphertext;
	crypto_state = state;

	return plaintext;

}

uint32_t crypto_encrypt(uint32_t plaintext) {
	uint32_t ciphertext;
	uint32_t subkey;

	subkey = crypto_blockfunc( crypto_state, crypto_key );

	crypto_state = plaintext ^ crypto_LastCWord;

	ciphertext = subkey ^ crypto_state;

	crypto_LastCWord = ciphertext;

	return ciphertext;
}

