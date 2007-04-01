/* Copyright 2007 TeX Users Group

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

/* #pragma optimize ("lge", off)  */

int index;

char bitstring[32];

int blackbits=0, whitebits=1;

int getbit (void) {
	int c;
	for (;;) {
		c = _getch();
		if (c == EOF) exit(1);
		if (c == 'B') {
			blackbits=1; whitebits=0; putc('\n', stdout);
		}
		else if (c == 'W') {
			blackbits=0; whitebits=1; putc('\n', stdout);
		}
		else if (c == 'E' || c == 'Z' || c == 'F') exit(1);
		if (c == '0' || c == '1') {
			bitstring[index++] = (char) c;
			return (c - '0');
		}
	}
}

int byte, int bitsleft;

int getnextbit (FILE *input) {
	if (bitsleft-- <= 0) {
		if ((byte = getc(input)) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(3);
		}
		bitsleft = 7;
	}
	byte = byte << 1;
	if (byte & 256) return 1;
	else return 0;
}

#pragma optimize ("lge", off) 

int commonmake (void) {		/* common black/white make up codes (7 zeros) */
	if (getbit()) {	/* 00000001 */
		if (getbit()) {	/* 000000011 */
			if (getbit()) {	/* 0000000111 */
				if (getbit()) {	/* 00000001111 */
					if (getbit()) {	/* 000000011111 */
						return 2560;
					}
					else { /* 000000011110 */
						return 2496;
					}
				}
				else { /* 00000001110 */
					if (getbit()) {	/* 000000011101 */
						return 2432;
					}
					else { /* 000000011100 */
						return 2368;
					}
				}
			}
			else { /* 0000000110 */
				if (getbit()) {	/* 00000001101 */
					return 1920;
				}
				else { /* 00000001100 */
					return 1856;
				}
			}
		}
		else { /* 000000010 */
			if (getbit()) {	/* 0000000101 */
				if (getbit()) {	/* 00000001011 */
					if (getbit()) {	/* 000000010111 */
						return 2304;
					}
					else { /* 000000010110 */
						return 2240;
					}
				}
				else { /* 00000001010 */
					if (getbit()) {	/* 000000010101 */
						return 2176;
					}
					else { /* 000000010100 */
						return 2112;
					}
				}
			}
			else { /* 0000000100 */
				if (getbit()) {	/* 00000001001 */
					if (getbit()) {	/* 000000010011 */
						return 2048;
					}
					else { /* 000000010010 */
						return 1984;
					}
				}
				else { /* 00000001000 */
					return 1792;
				}
			}
		}
	}
	else { /* 00000000 */
/*	Actually, EOL code is not supposed to be used in TIFF compression 2 */
		if (!getbit()) { /* 000000000 */
			if (!getbit()) { /* 0000000000 */
				if (!getbit()) { /* 00000000000 */
					if (getbit()) { /* 000000000001 */
						fprintf(stderr, "EOL ");
						return 0;				/* ??? */
					}
				}
			}
		}
	}
	return -1;	/* error */
}

int whiterun (void) {
	if (getbit()) {	/* 1 */
		if (getbit()) {	/* 11 */
			if (getbit()) {	/* 111 */
				if (getbit()) {	/* 1111 */
					return 7;
				}
				else {			/* 1110 */
					return 6;
				}
			}
			else {			/* 110 */
				if (getbit()) {	/* 1101 */
					if (getbit()) { /* 11011 */
						return 64;		/* make up */
					}
					else {			/* 11010 */
						if (getbit()) { /* 110101 */
							return 15;
						}
						else {			/* 110100 */
							return 14;
						}
					}
				}
				else {			/* 1100 */
					return 5;
				}
			}
		}
		else {			/* 10 */
			if (getbit()) {	/* 101 */
				if (getbit()) {	/* 1011 */
					return 4;
				}
				else {			/* 1010 */
					if (getbit()) { /* 10101 */
						if (getbit()) { /* 101011 */
							return 17;
						}
						else {			/* 101010 */
							return 16;
						}
					}
					else {			/* 10100 */
						return 9;
					}
				}
			}
			else {			/* 100 */
				if (getbit()) {	/* 1001 */
					if (getbit()) { /* 10011 */
						return 8;
					}
					else {			/* 10010 */
						return 128;	/* make up */
					}
				}
				else {			/* 1000 */
					return 3;
				}
			}
		}
	}
	else {			/* 0 */
		if (getbit()) {	/* 01 */
			if (getbit()) {	/* 011 */
				if (getbit()) {	/* 0111 */
					return 2;
				}
				else {			/* 0110 */
					if (getbit()) { /* 01101 */
						if (getbit()) { /* 011011 */
							if (getbit()) { /* 0110111 */
								return 256;
							}
							else {			/* 0110110 */
								if (getbit()) {	/* 01101101 */
									if (getbit()) {	/* 011011011 */
										return 1408;
									}
									else { /*  011011010 */
										return 1344;
									}
								}
								else {			/* 01101100 */
									if (getbit()) {	/* 011011001 */
										return 1280;
									}
									else { /* 011011000 */
										return 1216;
									}
								}
							}
						}
						else { /* 011010 */
							if (getbit()) { /* 0110101 */
								if (getbit()) { /* 01101011 */
									if (getbit()) { /* 011010111 */
										return 1152;	/* make up */
									}
									else {			/* 011010110 */
										return 1088;	/* make up */
									}
								}
								else { /* 01101010 */
									if (getbit()) { /* 011010101 */
										return 1024;	/* make up */
									}
									else { /* 011010100 */
										return 960; /* make up */
									}
								}
							}
							else { /* 0110100 */
								if (getbit()) { /* 01101001 */
									if (getbit()) { /* 011010011 */
										return 896;	/* make up */
									}
									else { /* 011010010 */
										return 832;	/* make up */
									}
								}
								else { /* 01101000 */
									return 576;	/* make up */
								}
							}
						}
					}
					else { /* 01100 */
						if (getbit()) { /* 011001 */
							if (getbit()) { /* 0110011 */
								if (getbit()) { /* 01100111 */
									return 640;	/* make up */
								}
								else { /* 01100110 */
									if (getbit()) { /* 011001101 */
										return 768;	/* make up */
									}
									else { /* 011001100 */
										return 704;	/* make up */
									}
								}
							}
							else { /* 0110010 */
								if (getbit()) { /* 01100101 */
									return 512;	/* make up */
								}
								else { /* 01100100 */
									return 448;	/* make up */
								}
							}
						}
						else { /* 011000 */
							return 1664;	/* make up */
						}
					}
				}
			}
			else {			/* 010 */
				if (getbit()) {	/* 0101 */
					if (getbit()) { /* 01011 */
						if (getbit()) { /* 010111 */
							return 192;		/* make up */
						}
						else { /* 010110 */
							if (getbit()) { /* 0101101 */
								if (getbit()) { /* 01011011 */
									return 58;
								}
								else { /* 01011010 */
									return 57;
								}
							}
							else { /* 0101100 */
								if (getbit()) { /* 01011001 */
									return 56;
								}
								else { /* 01011000 */
									return 55;
								}
							}
						}
					}
					else {			/* 01010 */
						if (getbit()) { /* 010101 */
							if (getbit()) { /* 0101011 */
								return 25;
							}
							else {			/* 0101010 */
								if (getbit()) { /* 01010101 */
									return 52;
								}
								else { /* 01010100 */
									return 51;
								}
							}
						}
						else {			/* 010100 */
							if (getbit()) { /* 0101001 */
								if (getbit()) { /* 01010011 */
									return 50;
								}
								else { /* 01010010 */
									return 49;
								}
							}
							else {			/* 0101000 */
								return 24;
							}
						}
					}
				}
				else {			/* 0100 */
					if (getbit()) {	/* 01001 */
						if (getbit()) { /* 010011 */
							if (getbit()) { /* 0100111 */
								return 18;
							}
							else {			/* 0100110 */
								if (getbit()) {	/* 01001101 */
									if (getbit()) {	/* 010011011 */
										return 1728;	/* make up */
									}
									else { /* 010011010 */
										return 1600;
									}
								}
								else { /* 01001100 */
									if (getbit()) {	/* 010011001 */
										return 1536;
									}
									else { /* 010011000 */
										return 1472;
									}
								}
							}
						}
						else {			/* 010010 */
							if (getbit()) { /* 0100101 */
								if (getbit()) { /* 01001011 */
									return 60;
								}
								else { /* 01001010 */
									return 59;
								}
							}
							else {			/* 0100100 */
								return 27;
							}
						}
					}
					else {			/* 01000 */
						return 11;
					}
				}
			}
		}
		else {			/* 00 */
			if (getbit()) {	/* 001 */
				if (getbit()) {	/* 0011 */
					if (getbit()) {	/* 00111 */
						return 10;
					}
					else {			/* 00110 */
						if (getbit()) { /* 001101 */
							if (getbit()) { /* 0011011 */
								if (getbit()) {	/* 0110111 */
									return 384; /* make up */
								}
								else { /* 0110110 */
									return 320; /* make up */
								}
							}
							else { /* 0011010 */
								if (getbit()) { /* 00110101 */
									return 0;
								}
								else { /* 00110100 */
									return 63;
								}
							}
						}
						else {			/* 001100 */
							if (getbit()) { /* 0011001 */
								if (getbit()) { /* 00110011 */
									return 62;
								}
								else { /* 00110010 */
									return 61;
								}
							}
							else {			/* 0011000 */
								return 28;
							}
						}
					}
				}
				else {			/* 0010 */
					if (getbit()) { /* 00101 */
						if (getbit()) { /* 001011 */
							if (getbit()) { /* 0010111 */
								return 21;
							}
							else {			/* 0010110 */
								if (getbit()) { /* 00101101 */
									return 44;
								}
								else { /* 00101100 */
									return 43;
								}
							}
						}
						else {			/* 001010 */
							if (getbit()) { /* 0010101 */
								if (getbit()) { /* 00101011 */
									return 42;
								}
								else { /* 00101010 */
									return 41;
								}
							}
							else { /* 0010100 */
								if (getbit()) { /* 00101001 */
									return 40;
								}
								else { /* 00101000 */
									return 39;
								}
							}
						}
					}
					else {			/* 00100 */
						if (getbit()) { /* 001001 */
							if (getbit()) { /* 0010011 */
								return 26;
							}
							else {			/* 0010010 */
								if (getbit()) { /* 00100101 */
									return 54;
								}
								else { /* 00100100 */
									return 53;
								}
							}
						}
						else {			/* 001000 */
							return 12;
						}
					}
				}
			}
			else {		/* 000 */
				if (getbit()) {	/* 0001 */
					if (getbit()) {	/* 00011 */
						if (getbit()) { /* 000111 */
							return 1;
						}
						else {			/* 000110 */
							if (getbit()) { /* 0001101 */
								if (getbit()) { /* 00011011 */
									return 32;
								}
								else { /* 00011010 */
									return 31;
								}
							}
							else {		/* 0001100 */
								return 19;
							}
						}
					}
					else {		/* 00010 */
						if (getbit()) { /* 000101 */
							if (getbit()) { /* 0001011 */
								if (getbit()) { /* 00010111 */
									return 38;
								}
								else { /* 00010110 */
									return 37;
								}
							}
							else { /* 0001010 */
								if (getbit()) { /* 00010101 */
									return 36;
								}
								else { /* 00010100 */
									return 35;
								}
							}
						}
						else {			/* 000100 */
							if (getbit()) { /* 0001001 */
								if (getbit()) { /* 00010011 */
									return 34;
								}
								else { /* 00010010 */
									return 33;
								}
							}
							else {			/* 0001000 */
								return 20;
							}
						}
					}
				}
				else {		/* 0000 */
					if (getbit()) { /* 00001 */
						if (getbit()) { /* 000011 */
							return 13;
						}
						else {			/* 000010 */
							if (getbit()) { /* 0000101 */
								if (getbit()) { /* 00001011 */
									return 48;
								}
								else { /* 00001010 */
									return 47;
								}
							}
							else {			/* 0000100 */
								return 23;
							}
						}
					}
					else {		/* 00000 */
						if (getbit()) { /* 000001 */
							if (getbit()) { /* 0000011 */
								return 22;
							}
							else {			/* 0000010 */
								if (getbit()) { /* 00000101 */
									return 46;
								}
								else { /* 00000100 */
									return 45;
								}
							}
						}
						else {			/* 000000 */
							if (getbit()) { /* 0000001 */
								if (getbit()) { /* 00000011 */
									return 30;
								}
								else { /* 00000010 */
									return 29;
								}
							}
							else { /* 0000000 */
								return commonmake();	/* seven zeros */
							}
						}
					}
				}
			}
		}
	}
	return -1;	/* error */
}

int blackzero (void) {		/* black run code starts with four zeros */
	if (getbit()) { /* 00001 */
		if (getbit()) { /* 000011 */
			if (getbit()) { /* 0000111 */
				return 12;
			}
			else {			/* 0000110 */
				if (getbit()) {	/* 00001101 */
					if (getbit()) { /* 000011011 */
						if (getbit()) { /* 0000110111 */
							return 0;
						}
						else {			/* 0000110110 */
							if (getbit()) { /* 00001101101 */
								if (getbit()) { /* 000011011011 */
									return 43;
								}
								else {			/* 000011011010 */
									return 42;
								}
							}
							else {			/* 00001101100 */
								return 21;
							}
						}
					}
					else {			/* 000011010 */
						if (getbit()) { /* 0000110101 */
							if (getbit()) { /* 00001101011 */
								if (getbit()) { /* 000011010111 */
									return 39;
								}
								else {			/* 000011010110 */
									return 38;
								}
							}
							else {			/* 00001101010 */
								if (getbit()) { /* 000011010101 */
									return 37;
								}
								else {			/* 000011010100 */
									return 36;
								}
							}
						}
						else {			/* 0000110100 */
							if (getbit()) { /* 00001101001 */
								if (getbit()) { /* 000011010011 */
									return 35;
								}
								else { /* 000011010010 */
									return 34;
								}
							}
							else {			/* 00001101000 */
								return 20;
							}
						}
					}
				}
				else {			/* 00001100 */
					if (getbit()) {	/* 000011001 */
						if (getbit()) { /* 0000110011 */
							if (getbit()) { /* 00001100111 */
								return 19;
							}
							else {			/* 00001100110 */
								if (getbit()) { /* 000011001101 */
									return 29;
								}
								else {			/* 000011001100 */
									return 28;
								}
							}
						}
						else {			/* 0000110010 */
							if (getbit()) { /* 00001100101 */
								if (getbit()) { /* 000011001011 */
									return 27;
								}
								else {			/* 000011001010 */
									return 26;
								}
							}
							else {			/* 00001100100 */
								if (getbit()) { /* 000011001001 */
									return 192;	/* make up */
								}
								else {			/* 000011001000 */
									return 128;	/* make up */
								}
							}
						}
					}
					else {			/* 000011000 */
						return 15;
					}
				}
			}
		}
		else {			/* 000010 */
			if (getbit()) { /* 0000101 */
				return 11;
			}
			else {			/* 0000100 */
				return 10;
			}
		}
	}
	else {		/* 00000 */
		if (getbit()) { /* 000001 */
			if (getbit()) { /* 0000011 */
				if (getbit()) {	/* 00000111 */
					return 14;
				}
				else {			/* 00000110 */
					if (getbit()) { /* 000001101 */
						if (getbit()) { /* 0000011011 */
							if (getbit()) { /* 00000110111 */
								return 22;
							}
							else {			/* 00000110110 */
								if (getbit()) { /* 000001101101 */
									return 41;
								}
								else {			/* 000001101100 */
									return 40;
								}
							}
						}
						else {			/* 0000011010 */
							if (getbit()) {  /* 00000110101 */
								if (getbit()) { /* 000001101011 */
									return 33;
								}
								else {			/* 000001101010 */
									return 32;
								}
							}
							else {			/* 00000110100 */
								if (getbit()) { /* 000001101001 */
									return 31;
								}
								else {			/* 000001101000 */
									return 30;
								}
							}
						}
					}
					else {			/* 000001100 */
						if (getbit()) { /* 0000011001 */
							if (getbit()) { /* 00000110011 */
								if (getbit()) { /* 000001100111 */
									return 63;
								}
								else {			/* 000001100110 */
									return 62;
								}
							}
							else {			/* 00000110010 */
								if (getbit()) { /* 000001100101 */
									return 49;
								}
								else {			/* 000001100100 */
									return 48;
								}
							}
						}
						else {			/* 0000011000 */
							return 17;
						}
					}
				}
			}
			else {			/* 0000010 */
				if (getbit()) {	/* 00000101 */
					if (getbit()) { /* 000001011 */
						if (getbit()) { /* 0000010111 */
							return 16;
						}
						else {			/* 0000010110 */
							if (getbit()) { /* 00000101101 */
								if (getbit()) { /* 000001011011 */
									return 256;		/* make up */
								}
								else {			/* 000001011010 */
									return 61;
								}
							}
							else {			/* 00000101100 */
								if (getbit()) { /* 000001011001 */
									return 58;
								}
								else {			/* 000001011000 */
									return 57;
								}
							}
						}
					}
					else {			/* 000001010 */
						if (getbit()) { /* 0000010101 */
							if (getbit()) { /* 00000101011 */
								if (getbit()) { /* 000001010111 */
									return 47;
								}
								else {			/* 000001010110 */
									return 46;
								}
							}
							else {			/* 00000101010 */
								if (getbit()) { /* 000001010101 */
									return 45;
								}
								else {			/* 000001010100 */
									return 44;
								}
							}
						}
						else {			/* 0000010100 */
							if (getbit()) { /* 00000101001 */
								if (getbit()) { /* 000001010011 */
									return 51;
								}
								else {			/* 000001010010 */
									return 50;
								}
							}
							else {			/* 00000101000 */
								return 23;
							}
						}
					}
				}
				else {			/* 00000100 */
					return 13;
				}
			}
		}
		else {			/* 000000 */
			if (getbit()) { /* 0000001 */
				if (getbit()) { /* 00000011 */
					if (getbit()) { /* 000000111 */
						if (getbit()) { /* 0000001111 */
							return 64;	/* make up */
						}
						else {			/* 0000001110 */
							if (getbit()) { /* 00000011101 */
								if (getbit()) { /* 000000111011 */
									if (getbit()) { /* 0000001110111 */
										return 1216;
									}
									else {			/* 0000001110110 */
										return 1152;
									}
								}
								else {			/* 000000111010 */
									if (getbit()) { /* 0000001110101 */
										return 1088;
									}
									else {			/* 0000001110100 */
										return 1024;
									}

								}
							}
							else {			/* 00000011100 */
								if (getbit()) { /* 000000111001 */
									if (getbit()) { /* 0000001110011 */
										return 960;
									}
									else {			/* 0000001110010 */
										return 896;
									}
								}
								else {			/* 000000111000 */
									return 54;
								}
							}
						}
					}
					else {			/* 000000110 */
						if (getbit()) { /* 0000001101 */
							if (getbit()) { /* 00000011011 */
								if (getbit()) { /* 000000110111 */
									return 53;
								}
								else {			/* 000000110110 */
									if (getbit()) { /* 0000001101101 */
										return 576;	/*make up */
									}
									else {			/* 0000001101100 */
										return 512;	/* make up */
									}
								}
							}
							else {			/* 00000011010 */
								if (getbit()) { /* 000000110101 */
									return 448;	/* make up */
								}
								else {			/* 000000110100 */
									return 384;	/* make up */
								}
							}
						}
						else {			/* 0000001100 */
							if (getbit()) { /* 00000011001 */
								if (getbit()) { /* 000000110011 */
									return 320;	/* make up */
								}
								else {			/* 000000110010 */
									if (getbit()) { /* 0000001100101 */
										return 1728;
									}
									else {			/* 0000001100100 */
										return 1664;
									}
								}
							}
							else {			/* 00000011000 */
								return 25;
							}
						}
					}
				}
				else { /* 00000010 */
					if (getbit()) { /* 000000101 */
						if (getbit()) { /* 0000001011 */
							if (getbit()) { /* 00000010111 */
								return 24;
							}
							else {			/* 00000010110 */
								if (getbit()) { /* 000000101101 */
									if (getbit()) { /* 0000001011011 */
										return 1600;
									}
									else {			/* 0000001011010 */
										return 1536;
									}
								}
								else {			/* 000000101100 */
									return 60;
								}
							}
						}
						else {			/* 0000001010 */
							if (getbit()) { /* 00000010101 */
								if (getbit()) { /* 000000101011 */
									return 59;
								}
								else {			/* 000000101010 */
									if (getbit()) { /* 0000001010101 */
										return 1472;
									}
									else {			/* 0000001010100 */
										return 1408;
									}
								}
							}
							else {			/* 00000010100 */
								if (getbit()) { /* 000000101001 */
									if (getbit()) { /* 0000001010011 */
										return 1344;
									}
									else {			/* 0000001010010 */
										return 1280;
									}
								}
								else {			/* 000000101000 */
									return 56;
								}
							}
						}
					}
					else {			/* 000000100 */
						if (getbit()) { /* 0000001001 */
							if (getbit()) { /* 00000010011 */
								if (getbit()) { /* 000000100111 */
									return 55;
								}
								else {			/* 000000100110 */
									if (getbit()) { /* 0000001001101 */
										return 832;
									}
									else {			/* 0000001001100 */
										return 768;
									}
								}
							}
							else {			/* 00000010010 */
								if (getbit()) { /* 000000100101 */
									if (getbit()) { /* 0000001001011 */
										return 704;
									}
									else {			/* 0000001001010 */
										return 640;
									}
								}
								else {			/* 000000100100 */
									return 52;
								}
							}
						}
						else {			/* 0000001000 */
							return 18;
						}
					}
				}
			}
			else { /* 0000000 */
				return commonmake();	/* seven zeros */
			}
		}
	}
	return -1;	/* error */
}

int blackrun (void) {
	if (getbit()) {	/* 1 */
		if (getbit()) {	/* 11 */
			return 2;
		}
		else {			/* 10 */
			return 3;
		}
	}
	else {			/* 0 */
		if (getbit()) {	/* 01 */
			if (getbit()) {	/* 011 */
				return 4;
			}
			else {			/* 010 */
				return 1;
			}
		}
		else {			/* 00 */
			if (getbit()) {	/* 001 */
				if (getbit()) {	/* 0011 */
					return 5;
				}
				else {			/* 0010 */
					return 6;
				}
			}
			else {		/* 000 */
				if (getbit()) {	/* 0001 */
					if (getbit()) {	/* 00011 */
						return 7;
					}
					else {		/* 00010 */
						if (getbit()) { /* 000101 */
							return 8;
						}
						else {			/* 000100 */
							return 9;
						}
					}
				}
				else {		/* 0000 */
					return blackzero();	/* four zeros */
				}
			}
		}
	}
	return -1;	/* error */
}

#pragma optimize ("lge", on) 

void showstring(void) {
	int k;
	for (k = 0; k < index; k++) putc(bitstring[k], stdout);  
	putc(' ', stdout);
}

int index, bitinx;

unsigned char *lpBuffer;

int writewhite (int run) {
	total += run;
	if (total > width) return -1;	/* error */
/*	nothing to do, since already preset to zero */	
	if (total == width) return 1;	/* EOL */
	else return 0;
}

int writeblack (int run) {
	total += run;
	if (total > width) return -1;	/* error */
	while (run > 0) {
		if (bitinx == 0) {
			while (run >= 8) {	
				lpBuffer[index++] = 255;	/* write a byte at a time */
				run -= 8;
			}
		}
		else {
			if (bitinx-- <=  0) {
				index++; bitinx = 7;
			}
			lpBuffer[index] |= (1 << bitinx); 	/* write a bit at a time */
			run--;
		}
	}
	if (total == width) return 1;	/* EOL */
	else return 0;
}

int huffmanrow (FILE *input, int width) {
	int k, bytes;
	int run;
	int total = 0;

	index = -1; bitinx = 0;
	byte = 0; bitsleft = 0;
	bytes = (width + 7) / 8;
	for (k = 0; k < bytes; k++) lpBuffer[k] = 0;

	for (;;) {
		while ((run = whiterun ()) >= 64) 
			if (writewhite(run) != 0) break;	/* make up run */
		if (total >= width) break;
		if (writewhite(run) != 0) break;		/* terminal run */
		while ((run = blackrun ()) >= 64) 
			if (writeblack(run) != 0) break;	/* make up run */
		if (total >= width) break;
		if (writeblack(run) != 0) break;		/* terminal run */
	}
	if (total != width) {
		fprintf(stderr, "Sum of runs %d not equal width %d\n", total, width);
	}
}

int main(int argc, char *argv[]) {
	int run;
	for (;;) {
		index = 0;
		if (blackbits) 	{
			run = blackrun();
			showstring();
			printf("black run ");
		}
		else {
			run = whiterun();
			showstring();
			printf("white run ");
		}
		printf("%d\n", run);
	}
	return 0;
}
