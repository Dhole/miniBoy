#include <stdio.h>
#include <stdint.h>

typedef enum {
	Z_FLAG = 1 << 7,
	N_FLAG = 1 << 6,
	H_FLAG = 1 << 5,
	C_FLAG = 1 << 4,
} flag_t;

uint8_t val_a, val_b;
uint8_t *ra, *rb;
uint8_t F;

char *byte2bin_str(uint8_t b, char *s) {
	int i, bi;
	for (i = 0; i < 8; i++) {
		bi = 7 - i;
		s[i] = ((b & (1 << bi)) >> bi) + 48;
	}
	s[i] = '\0';
	return s;
}

void set_flag(flag_t f, int v) {
	if (v) {
		F |= f;
	} else {
		F &= ~f;
	}
}

void set_flag_Z(uint8_t *a) {
	if (*a == 0) {
		set_flag(Z_FLAG, 1);
	} else {
		set_flag(Z_FLAG, 0);
	}	
}

uint8_t get_flag(flag_t f) {
	return (F & f) > 0 ? 1 : 0;
}

void op_daa_1(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	
        uint8_t corr = 0;

	corr |= (get_flag(H_FLAG) || ((*a & 0x0F) > 0x09)) ? 0x06 : 0x00;
	corr |= (get_flag(C_FLAG) || (*a > 0x99)) ? 0x60 : 0x00;
	
	if (get_flag(N_FLAG)) {
		*a -= corr;
	} else {
		*a += corr;
	}
	//printf("Corr: %02X\n", corr);

	set_flag(C_FLAG, (corr << 2 & 0x100) ? 1 : 0);
        set_flag_Z(a);
	set_flag(H_FLAG, 0);
	
}

void op_daa_GB() {
	//calcHF(HF1, HF2);
	uint8_t HF2 = F;
	int A = *ra;
            
	//unsigned correction = (CF & 0x100) ? 0x60 : 0x00;
	unsigned correction = (get_flag(C_FLAG)) ? 0x60 : 0x00;
               
	if (HF2 & 0x200)
		correction |= 0x06;
               
	if (!(HF2 &= 0x400)) {
		if ((A & 0x0F) > 0x09)
			correction |= 0x06;
                  
		if (A > 0x99)
			correction |= 0x60;
                  
		A += correction;
	} else
		A -= correction;
               
	//CF = correction << 2 & 0x100;
	set_flag(C_FLAG, correction << 2 & 0x100);
	//ZF = A;
	set_flag(Z_FLAG, A);
	A &= 0xFF;

	*ra = A;
}

void op_daa_2(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;

	uint16_t a_cpy = *a;

	if (get_flag(N_FLAG)) {
		if (get_flag(H_FLAG)) {
			a_cpy = (a_cpy - 0x06) & 0xFF;
		}
		if (get_flag(C_FLAG)) {
			a_cpy -= 0x60;
		}
	} else {
		if (get_flag(H_FLAG) || (a_cpy & 0x0F) > 9) {
			a_cpy += 0x06;
		}
		if (get_flag(C_FLAG) || a_cpy > 0x9F) {
			a_cpy += 0x60;
		}
	}

	if ((a_cpy & 0x100) == 0x100) {
		set_flag(C_FLAG, 1);
	}
	
	set_flag(H_FLAG, 0);

	*a = a_cpy;
	
	set_flag_Z(a);		
}

void op_daa_DP()
{
	int a = *ra;

	if (!get_flag(N_FLAG))
	{
		if (get_flag(H_FLAG) || (a & 0xF) > 9)
			a += 0x06;

		if (get_flag(C_FLAG) || a > 0x9F)
			a += 0x60;
	}
	else
	{
		if (get_flag(H_FLAG))
			a = (a - 6) & 0xFF;

		if (get_flag(C_FLAG))
			a -= 0x60;
	}

	F &= ~(H_FLAG | Z_FLAG);

	if ((a & 0x100) == 0x100)
		F |= C_FLAG;

	a &= 0xFF;

	if (a == 0)
		F |= Z_FLAG;

	*ra = (uint8_t)a;
}

int main() {
        ra = &val_a;
	rb = &val_b;
	F = 0;

	uint8_t ra_tmp[4];
	uint8_t F_tmp[4];
	char s[16];
	int dif = 0;

	int a, f;
	for (a = 0; a < 0x100; a++) {
		for (f = 0; f < 0x10; f++) {
			*ra = a;
			F = f << 4;
			
			//op_daa_1(ra, NULL);
			op_daa_DP();
			ra_tmp[0] = *ra;
			F_tmp[0] = F;
			
			*ra = a;
			F = f << 4;

			op_daa_2(ra, NULL);
			ra_tmp[1] = *ra;
			F_tmp[1] = F;
			
			//if (F_tmp[0] != F_tmp[1]) {
			if (F_tmp[0] != F_tmp[1] || ra_tmp[0] != ra_tmp[1]) {
				dif++;
				printf("IN   -> A: %02X, F: %s\n",
				       a, byte2bin_str(f << 4, s));
				//printf("IN   -> A: %02X, F: %02X\n",
				//       a, f << 4);
				printf("RES_1-> A: %02X, F: %s\n",
				       ra_tmp[0], byte2bin_str(F_tmp[0], s));
				printf("RES_2-> A: %02X, F: %s\n",
				       ra_tmp[1], byte2bin_str(F_tmp[1], s));
				printf("\n");
			}
		}		
	}

	printf("There has been %d/%d differences in F or A result\n", dif, 0xff*0xf);
	
	
	return 0;
}

