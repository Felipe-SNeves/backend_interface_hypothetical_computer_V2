#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define AXR 0xF0
#define BXR 0xF1
#define CXR 0xF2
#define DXR 0xF3

// Registradores gerais
union {
    int16_t H;
} AX, BX, CX, DX;

// Registradores especiais
union {
    uint8_t * H;
} SP, IP, BP;

// Registrador de flags
struct {
    unsigned int zf : 1;
    unsigned int sf : 1;
} EFLAGS;

uint8_t RAM[65536];

void init_sys ();
void load_program ();
void fetch ();
int decode ();
void flag_ver ();

int main () {

    int ret;
    
    init_sys ();
    load_program ();

    do {
        fetch ();
        ret = decode ();
    } while (ret);

    return ret;
}

void flag_ver () {
    
    if (AX.H == 0)
        EFLAGS.zf = 1;
    else
        EFLAGS.zf = 0;

    if (AX.H < 0)
        EFLAGS.sf = 1;
    else
        EFLAGS.sf = 0;
}

void init_sys () {

    // Inicialização de opcodes na RAM
    RAM[0] = 0x0A; // HLT
    RAM[1] = 0x0B; // SHW
    RAM[2] = 0x2A; // MOV
    RAM[3] = 0x2B; // LDA
    RAM[4] = 0x2C; // STA
    RAM[5] = 0x1A; // ADD
    RAM[6] = 0x1B; // SUB
    RAM[7] = 0x1C; // MUL
    RAM[8] = 0x1D; // DIV
    RAM[9] = 0x3A; // CMP
    RAM[10] = 0x3B; // CMZ
    RAM[11] = 0x3C; // CMS
    RAM[12] = 0x4A; // JMP
    RAM[13] = 0x4B; // JMZ
    RAM[14] = 0x4C; // JMS
    RAM[15] = 0x5A; // PSH
    RAM[16] = 0x5B; // POP
    RAM[17] = 0x6A; // WRT

    RAM[0x0164] = 71;
    RAM[0x0170] = 9;

    BP.H = RAM + 0xEA60; // Base pointer setado para o endereço 60000 (base da stack)
    SP.H = BP.H;
    IP.H = RAM + 0x3E7; // Registrador IP setado no endereço 999 (inicio do primeiro fetch)
}

void load_program () {

    FILE * program = fopen ("programa.f", "rb");
    int i = 0x3E8;

    while (!feof (program)) {
        fread (RAM + i++, 1, 1, program);
    }

    fclose (program);
}

void fetch () {
    IP.H++;
}

int show () {
    printf ("AX: %.2X\t", AX.H);
    printf ("BX: %.2X\t", BX.H);
    printf ("CX: %.2X\t", CX.H);
    printf ("DX: %.2X\n", DX.H);
    printf ("SP: %.2X\t", SP.H);
    printf ("BP: %.2X\t", BP.H);
    printf ("IP: %.2X\n", IP.H);
    printf ("ZF: %d\t", EFLAGS.zf == 1);
    printf ("SF: %d\n", EFLAGS.sf == 1);

    return 1;
}

void operands_two (uint16_t * a, uint16_t * b) {
    fetch();
    *a = *IP.H;
    fetch ();
    *b = *IP.H;
}

void operands_two_16 (uint16_t * a, uint16_t * b) {
    fetch();
    *a = *IP.H;
    fetch ();
    *b = *IP.H;
    fetch ();
    *b = *b << 8;
    *b += *IP.H;
}

int mov () {
    
    uint16_t dest;
    uint16_t source;

    operands_two (&dest, &source);

    if (dest == 0xF0) {
        if (source == 0xF1) {
            AX.H = BX.H;
            flag_ver ();
        }
        if (source == 0xF2) {
            AX.H = CX.H;
            flag_ver ();
        }
        if (source == 0xF3) {
            AX.H = DX.H;
            flag_ver ();
        }
    }
    else if (dest == 0xF1) {
        if (source == 0xF0)
            BX.H = AX.H;
        if (source == 0xF2)
            BX.H = CX.H;
        if (source == 0xF3)
            BX.H = DX.H;
    }
    else if (dest == 0xF2) {
        if (source == 0xF0)
            CX.H = AX.H;
        if (source == 0xF1)
            CX.H = BX.H;
        if (source == 0xF3)
            CX.H = DX.H;
    }
    else {
        if (source == 0xF0)
            DX.H = AX.H;
        if (source == 0xF1)
            DX.H = BX.H;
        if (source == 0xF2)
            DX.H = CX.H;
    }
    
    return 1;
}

int lda () {

    uint16_t dest;
    uint16_t source;

    operands_two_16 (&dest, &source);

    switch (dest) {
        case 0xF0:
            AX.H = RAM[source]; 
            flag_ver ();
            break;
        case 0xF1:
            BX.H = RAM[source]; break;
        case 0xF2:
            CX.H = RAM[source]; break;
        case 0xF3:
            DX.H = RAM[source]; break;
    }

    return 1;
}

int sta () {

    uint16_t dest, source;

    operands_two_16 (&source, &dest);

    switch (source) {
        case 0xF0:
            RAM[dest] = AX.H; break;
        case 0xF1:
            RAM[dest] = BX.H; break;
        case 0xF2:
            RAM[dest] = CX.H; break;
        case 0xF3:
            RAM[dest] = DX.H; break;
    }

    return 1;
}

int add () {

    uint16_t source, dest;

    operands_two (&dest, &source);

    switch (dest) {
        case 0xF0:
            if (source == AXR)
                AX.H += AX.H;
            else if (source == BXR)
                AX.H += BX.H;
            else if (source == CXR)
                AX.H += CX.H;
            else if (source == DXR)
                AX.H += DX.H;
            else
                AX.H += source;
            flag_ver ();
            break;
        case 0xF1:
            if (source == AXR)
                BX.H += AX.H;
            else if (source == BXR)
                BX.H += BX.H;
            else if (source == CXR)
                BX.H += CX.H;
            else if (source == DXR)
                BX.H += DX.H;
            else
                BX.H += source;
            break;
        case 0xF2:
            if (source == AXR)
                CX.H += AX.H;
            else if (source == BXR)
                CX.H += BX.H;
            else if (source == CXR)
                CX.H += CX.H;
            else if (source == DXR)
                CX.H += DX.H;
            else
                CX.H += source;
            break;
        case 0xF3:
            if (source == AXR)
                DX.H += AX.H;
            else if (source == BXR)
                DX.H += BX.H;
            else if (source == CXR)
                DX.H += CX.H;
            else if (source == DXR)
                DX.H += DX.H;
            else
                DX.H += source;
    }

    return 1;
}

int sub () {

    uint16_t source, dest;

    operands_two (&dest, &source);

    switch (dest) {
        case 0xF0:
            if (source == AXR)
                AX.H -= AX.H;
            else if (source == BXR)
                AX.H -= BX.H;
            else if (source == CXR)
                AX.H -= CX.H;
            else if (source == DXR)
                AX.H -= DX.H;
            else
                AX.H -= source;
            flag_ver ();
            break;
        case 0xF1:
            if (source == AXR)
                BX.H -= AX.H;
            else if (source == BXR)
                BX.H -= BX.H;
            else if (source == CXR)
                BX.H -= CX.H;
            else if (source == DXR)
                BX.H -= DX.H;
            else
                BX.H -= source;
            break;
        case 0xF2:
            if (source == AXR)
                CX.H -= AX.H;
            else if (source == BXR)
                CX.H -= BX.H;
            else if (source == CXR)
                CX.H -= CX.H;
            else if (source == DXR)
                CX.H -= DX.H;
            else
                CX.H -= source;
            break;
        case 0xF3:
            if (source == AXR)
                DX.H -= AX.H;
            else if (source == BXR)
                DX.H -= BX.H;
            else if (source == CXR)
                DX.H -= CX.H;
            else if (source == DXR)
                DX.H -= DX.H;
            else
                DX.H -= source;
    }

    return 1;
}

int mov_clone (uint16_t dest, uint16_t source) {
    
    if (dest == 0xF0) {
        if (source == 0xF1) {
            AX.H = BX.H;
            flag_ver ();
        }
        if (source == 0xF2) {
            AX.H = CX.H;
            flag_ver ();
        }
        if (source == 0xF3) {
            AX.H = DX.H;
            flag_ver ();
        }
    }
    else if (dest == 0xF1) {
        if (source == 0xF0)
            BX.H = AX.H;
        if (source == 0xF2)
            BX.H = CX.H;
        if (source == 0xF3)
            BX.H = DX.H;
    }
    else if (dest == 0xF2) {
        if (source == 0xF0)
            CX.H = AX.H;
        if (source == 0xF1)
            CX.H = BX.H;
        if (source == 0xF3)
            CX.H = DX.H;
    }
    else {
        if (source == 0xF0)
            DX.H = AX.H;
        if (source == 0xF1)
            DX.H = BX.H;
        if (source == 0xF2)
            DX.H = CX.H;
    }
    
    return 1;
}

int sub_clone (uint16_t dest, uint16_t source) {

    switch (dest) {
        case 0xF0:
            if (source == AXR)
                AX.H -= AX.H;
            else if (source == BXR)
                AX.H -= BX.H;
            else if (source == CXR)
                AX.H -= CX.H;
            else if (source == DXR)
                AX.H -= DX.H;
            else
                AX.H -= source;
            flag_ver ();
            break;
        case 0xF1:
            if (source == AXR)
                BX.H -= AX.H;
            else if (source == BXR)
                BX.H -= BX.H;
            else if (source == CXR)
                BX.H -= CX.H;
            else if (source == DXR)
                BX.H -= DX.H;
            else
                BX.H -= source;
            break;
        case 0xF2:
            if (source == AXR)
                CX.H -= AX.H;
            else if (source == BXR)
                CX.H -= BX.H;
            else if (source == CXR)
                CX.H -= CX.H;
            else if (source == DXR)
                CX.H -= DX.H;
            else
                CX.H -= source;
            break;
        case 0xF3:
            if (source == AXR)
                DX.H -= AX.H;
            else if (source == BXR)
                DX.H -= BX.H;
            else if (source == CXR)
                DX.H -= CX.H;
            else if (source == DXR)
                DX.H -= DX.H;
            else
                DX.H -= source;
    }

    return 1;
}

int add_clone (uint16_t dest, uint16_t source) {

    switch (dest) {
        case 0xF0:
            if (source == AXR)
                AX.H += AX.H;
            else if (source == BXR)
                AX.H += BX.H;
            else if (source == CXR)
                AX.H += CX.H;
            else if (source == DXR)
                AX.H += DX.H;
            else
                AX.H += source;
            flag_ver ();
            break;
        case 0xF1:
            if (source == AXR)
                BX.H += AX.H;
            else if (source == BXR)
                BX.H += BX.H;
            else if (source == CXR)
                BX.H += CX.H;
            else if (source == DXR)
                BX.H += DX.H;
            else
                BX.H += source;
            break;
        case 0xF2:
            if (source == AXR)
                CX.H += AX.H;
            else if (source == BXR)
                CX.H += BX.H;
            else if (source == CXR)
                CX.H += CX.H;
            else if (source == DXR)
                CX.H += DX.H;
            else
                CX.H += source;
            break;
        case 0xF3:
            if (source == AXR)
                DX.H += AX.H;
            else if (source == BXR)
                DX.H += BX.H;
            else if (source == CXR)
                DX.H += CX.H;
            else if (source == DXR)
                DX.H += DX.H;
            else
                DX.H += source;
    }

    return 1;
}

int mul () {

    uint16_t source, dest;

    operands_two (&dest, &source);
    
    if (source == AXR || source == BXR)
        mov_clone (CXR, source);
    else
        CX.H = source;

    switch (dest) {
        case AXR: 
            mov_clone (DXR, AXR);
            while (CX.H > 1) {
                add_clone (AXR, DXR);
                sub_clone (CXR, 0x01);
            }
            break;
        case BXR:
            mov_clone (DXR, BXR);
            while (CX.H > 1) {
                add_clone (BXR, DXR);
                sub_clone (CXR, 0x01);
            }
    }

    return 1;
}

int divN () {

    uint16_t source, dest;

    operands_two (&dest, &source);

    sub_clone (CXR, CXR);

    switch (dest) {
        case AXR:
            if (source == BXR) {
                while (AX.H >= BX.H) {
                    sub_clone (AXR, BXR);
                    add_clone (CXR, 0x01);
                }
                mov_clone (DXR, AXR);
                mov_clone (AXR, CXR);
            }
            else {
                while (AX.H >= source) {
                    sub_clone (AXR, source);
                    add_clone (CXR, 0x01);
                }
                mov_clone (DXR, AXR);
                mov_clone (AXR, CXR);
            }
            break;
        case BXR:
            if (source == AXR) {
                while (BX.H >= AX.H) {
                    sub_clone (BXR, AXR);
                    add_clone (CXR, 0x01);
                }
                mov_clone (DXR, BXR);
                mov_clone (BXR, CXR);
            }
            else {
                while (BX.H >= source) {
                    sub_clone (BXR, source);
                    add_clone (CXR, 0x01);
                }
                mov_clone (DXR, BXR);
                mov_clone (BXR, CXR);
            }
    }

    return 1;
}

int comp () {

    uint16_t source, dest; 

    operands_two (&dest, &source);

    if (dest == AXR && source == BXR)
        DX.H = (int) AX.H == BX.H;
    else if (dest == AXR)
        DX.H = (int) AX.H == CX.H;
    else
        DX.H = (int) BX.H == CX.H;

    return 1;
}

int cmz () {
    DX.H = (EFLAGS.zf) ? 1 : 0;
    return 1;
}

int cms () {
    DX.H = (EFLAGS.sf) ? 1 : 0;
    return 1;
}

int jmp () {
    
    int8_t offset; 
    
    fetch ();
    offset = (int8_t) *IP.H;
    IP.H -= offset; 
}

int jz () {

    int8_t offset;

    fetch ();
    offset = (int8_t) *IP.H;

    if (EFLAGS.zf)
        IP.H -= offset;

    return 1;
}

int js () {

    int8_t offset;

    fetch ();
    offset = (int8_t) *IP.H;

    if (EFLAGS.sf)
        IP.H -= offset;

    return 1;
}

int wrt () {
    char letra;
    fetch ();
    letra = (char) *IP.H;
    write (1, &letra, 1);
    return 1;
}

int psh () {
    *SP.H = DX.H;

    printf ("Top: %.2X\n", *SP.H);

    SP.H++;

    return 1;
}

int poo () {

    SP.H--;
    DX.H = *SP.H;
    printf ("Return: %.2X\n", *SP.H);

    return 1;
}

int decode () {

    if (*IP.H == RAM[0])
        return 0;

    if (*IP.H == RAM[1])
        return show ();

    if (*IP.H == RAM[2])
        return mov ();

    if (*IP.H == RAM[3])
        return lda ();

    if (*IP.H == RAM[4])
        return sta ();

    if (*IP.H == RAM[5])
        return add ();

    if (*IP.H == RAM[6])
        return sub ();

    if (*IP.H == RAM[7])
        return mul ();

    if (*IP.H == RAM[8])
        return divN ();

    if (*IP.H == RAM[9])
        return comp ();

    if (*IP.H == RAM[10])
        return cmz ();

    if (*IP.H == RAM[11])
        return cms ();

    if (*IP.H == RAM[12])
        return jmp ();

    if (*IP.H == RAM[13])
        return jz ();

    if (*IP.H == RAM[14])
        return js ();

    if (*IP.H == RAM[15])
        return psh ();

    if (*IP.H == RAM[16])
        return poo ();

    if (*IP.H == RAM[17])
        return wrt ();
}