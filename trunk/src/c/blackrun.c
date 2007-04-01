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

/* ailt */

#pragma optimize ("ailt", off) 

#pragma optimize ("srd", on) 

int index;

char bitstring[32];

int getbit (void) {
	int c;
	for (;;) {
		c = _getch();
		if (c == EOF) exit(1);
		if (c >= 'A' && c <= 'Z') exit(1);
		if (c == '0' || c == '1') {
			bitstring[index++] = (char) c;
			return (c - '0');
		}
	}
}

#ifndef HACK
int commonmake (void) {
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
		if (getbit()) { /* 000000001 */
		}
		else { /* 000000000 */
			if (getbit()) { /* 0000000001 */
			}
			else { /* 0000000000 */
				if (getbit()) { /* 00000000001 */
				}
				else { /* 00000000000 */
					if (getbit()) { /* 000000000001 */
						fprintf(stderr, "EOL ");
					}
					else { /* 000000000000 */
					}
				}
			}
		}
	}
	return -1;
}
#endif

int commonzero (void) {
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
				#ifdef HACK
					/* 000000000001 == EOL */
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
					}
					#else
						return commonmake();
					#endif
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
#ifdef BAR
					if (getbit()) { /* 00001 */
						if (getbit()) { /* 000011 */
							if (getbit()) { /* 0000111 */
								return 12;
							}
							else {			/* 0000110 */
								if (getbit()) {	/* 00001101 */
								}
								else {			/* 00001100 */
									if (getbit()) {	/* 000011011 */
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
								}
							}
							else {			/* 0000010 */
								if (getbit()) {	/* 00000101 */
								}
								else {			/* 00000100 */
									return 13;
								}
							}
						}
						else {			/* 000000 */
							if (getbit()) { /* 0000001 */
								if (getbit()) {  /* 00000011 */
									if (getbit()) { /* 000000111 */
									}
									else {			/* 000000110 */
									}
								}
								else {			 /* 00000010 */
									if (getbit()) { /* 000000101 */
									}
									else {			/* 000000100 */
										if (getbit()) { /* 0000001001 */
											if (getbit()) { /* 00000010011 */
											}
											else {			/* 00000010010 */
												if (getbit()) { /* 000000100101 */
													if (getbit()) { /* 0000001001011 */
														return 704; /* make */
													}
													else {			/* 0000001001010 */
														return 640; /* make */
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
#ifdef HACK
		/* 000000000001 == EOL */
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
								}
#else
								return commonmake();
#endif
							}
						}
					}
#else
						return commonzero();
#endif
				}
			}
		}
	}
	return -1;	/* error */
}


int main(int argc, char *argv[]) {
	int k, run;
	for (;;) {
		index = 0;
		run = blackrun();
		for (k = 0; k < index; k++) putc(bitstring[k], stdout);  
		putc(' ', stdout);
		printf("black run %d\n", run);
		if (run == 0) break;
	}
	return 0;
}
