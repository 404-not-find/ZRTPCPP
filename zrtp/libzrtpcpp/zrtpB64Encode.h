/*
cencode.h - c header for a base64 encoding algorithm

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#ifndef BASE64_CENCODE_H
#define BASE64_CENCODE_H

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif

typedef enum
{
	step_A, step_B, step_C
} base64_encodestep;

typedef struct
{
	base64_encodestep step;
	char result;
	int stepcount;
    int lineLength;
} base64_encodestate;

void base64_init_encodestate(base64_encodestate* state_in, int lineLength);

ptrdiff_t base64_encode_block(const uint8_t *plaintext_in, int length_in, char* code_out, base64_encodestate* state_in);

int base64_encode_blockend(char *code_out, base64_encodestate* state_in);
#if defined(__cplusplus)
}
#endif

#endif /* BASE64_CENCODE_H */
