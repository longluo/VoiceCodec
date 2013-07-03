/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
*/

#ifndef ADPCM_H
#define ADPCM_H

#ifdef __cplusplus
extern "C" {
#endif

struct adpcm_state {
    short	valprev;	/* Previous output value */
    char	index;		/* Index into stepsize table */
};

int adpcm_coder(short *indata, unsigned char *outdata, int len, struct adpcm_state *state);
int adpcm_decoder(unsigned char *indata, short *outdata, int len, struct adpcm_state *state);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* ADPCM_H*/
