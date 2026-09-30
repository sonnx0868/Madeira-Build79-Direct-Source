#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
static int ios_teb_tls_slot_offset=0x900;
static int report_log(int fd,const char *format,...){(void)fd;(void)format;return 0;}
#define dprintf report_log
static unsigned char *ios_x18_build_data_map( const char *text, size_t text_size )
{
    unsigned char *data_map = calloc( 1, text_size / 32 + 1 );
    if (!data_map) return NULL;
    for (size_t i = 0; i < text_size; i += 4)
    {
        uint32_t insn = *(const uint32_t *)(text + i);
        uint32_t top8 = insn >> 24;
        size_t lit_bytes;
        int64_t imm19;
        size_t tgt;
        switch (top8)
        {
        case 0x18: case 0x1C: case 0x98: lit_bytes = 4;  break;  /* LDR Wt/St, LDRSW */
        case 0x58: case 0x5C: case 0xD8: lit_bytes = 8;  break;  /* LDR Xt/Dt, PRFM  */
        case 0x9C:                       lit_bytes = 16; break;  /* LDR Qt           */
        default: continue;
        }
        imm19 = (int64_t)(int32_t)(insn << 8) >> 13;  /* sign-extend bits[23:5] */
        tgt = i + (size_t)(imm19 * 4);
        if (imm19 * 4 + (int64_t)i < 0 || tgt >= text_size) continue;
        for (size_t b = tgt; b < tgt + lit_bytes && b < text_size; b += 4)
            data_map[(b / 4) >> 3] |= 1 << ((b / 4) & 7);
    }
    return data_map;
}

static void bad_pass(char *text_rw, size_t text_size, unsigned char *data_map) {
char *text_rx=text_rw;
    if (ios_teb_tls_slot_offset && text_size >= 12)
    {
        const uint32_t want_imm = (uint32_t)(ios_teb_tls_slot_offset / 8) << 10;
        static int announced;
        int found = 0, retargeted = 0;

        /* Liveness beacon: without it, "no retarget line" is ambiguous between
         * "the pass never ran" and "it ran and matched nothing" -- and the
         * first cut of this probe used ERR(), which is muted in this file, so
         * it could not report at all. */
        if (!announced)
        {
            announced = 1;
            dprintf(2, "[teb-tsd] retarget pass armed, offset=0x%x\n", ios_teb_tls_slot_offset);
        }

        for (size_t i = 0; i + 12 <= text_size; i += 4)
        {
            uint32_t i0, i1, i2;
            unsigned reg;

            if (data_map && (data_map[i / 4] || data_map[(i + 4) / 4] || data_map[(i + 8) / 4]))
                continue;

            i0 = *(uint32_t *)(text_rw + i);
            if ((i0 & 0xffffffe0) != 0xd53bd060) continue;      /* mrs xN, TPIDRRO_EL0 */
            reg = i0 & 0x1f;

            i1 = *(uint32_t *)(text_rw + i + 4);
            /* and xN, xN, #0xfffffffffffffff8 */
            if (i1 != (0x927df000u | (reg << 5) | reg)) continue;

            i2 = *(uint32_t *)(text_rw + i + 8);
            /* ldr xN, [xN, #imm12*8] -- same register throughout */
            if ((i2 & 0xffc003ff) != (0xf9400000u | (reg << 5) | reg)) continue;

            found++;
            if ((i2 & 0x003ffc00) == want_imm) continue;        /* already correct */

            *(uint32_t *)(text_rw + i + 8) = (i2 & ~0x003ffc00u) | want_imm;
            retargeted++;
        }

        /* Report whenever the module HAS such reads, so found>0/retargeted==0
         * (already correct) is distinguishable from found==0 (none present). */
        if (found)
            dprintf(2, "[teb-tsd] .text %p: found %d static TSD read(s), retargeted %d to offset 0x%x\n",
                    text_rx, found, retargeted, ios_teb_tls_slot_offset);
    }

}
static void good_pass(char *text_rw, size_t text_size, unsigned char *data_map) {
char *text_rx=text_rw;
    if (ios_teb_tls_slot_offset && text_size >= 12)
    {
        const uint32_t want_imm = (uint32_t)(ios_teb_tls_slot_offset / 8) << 10;
        static int announced;
        int found = 0, retargeted = 0;

        /* Liveness beacon: without it, "no retarget line" is ambiguous between
         * "the pass never ran" and "it ran and matched nothing" -- and the
         * first cut of this probe used ERR(), which is muted in this file, so
         * it could not report at all. */
        if (!announced)
        {
            announced = 1;
            dprintf(2, "[teb-tsd] retarget pass armed, offset=0x%x\n", ios_teb_tls_slot_offset);
        }

        for (size_t i = 0; i + 12 <= text_size; i += 4)
        {
            uint32_t i0, i1, i2;
            unsigned reg;

            if (data_map && ((data_map[(i / 4) >> 3] & (1u << ((i / 4) & 7))) ||
                             (data_map[((i + 4) / 4) >> 3] & (1u << (((i + 4) / 4) & 7))) ||
                             (data_map[((i + 8) / 4) >> 3] & (1u << (((i + 8) / 4) & 7)))))
                continue;

            i0 = *(uint32_t *)(text_rw + i);
            if ((i0 & 0xffffffe0) != 0xd53bd060) continue;      /* mrs xN, TPIDRRO_EL0 */
            reg = i0 & 0x1f;

            i1 = *(uint32_t *)(text_rw + i + 4);
            /* and xN, xN, #0xfffffffffffffff8 */
            if (i1 != (0x927df000u | (reg << 5) | reg)) continue;

            i2 = *(uint32_t *)(text_rw + i + 8);
            /* ldr xN, [xN, #imm12*8] -- same register throughout */
            if ((i2 & 0xffc003ff) != (0xf9400000u | (reg << 5) | reg)) continue;

            found++;
            if ((i2 & 0x003ffc00) == want_imm) continue;        /* already correct */

            *(uint32_t *)(text_rw + i + 8) = (i2 & ~0x003ffc00u) | want_imm;
            retargeted++;
        }

        /* Report whenever the module HAS such reads, so found>0/retargeted==0
         * (already correct) is distinguishable from found==0 (none present). */
        if (found)
            dprintf(2, "[teb-tsd] .text %p: found %d static TSD read(s), retargeted %d to offset 0x%x\n",
                    text_rx, found, retargeted, ios_teb_tls_slot_offset);
    }

}

int main(int argc,char **argv){
 assert(argc==3);FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>0);rewind(f);
 char *text=calloc(1,(size_t)n+4),*before=malloc((size_t)n);assert(text&&before);assert(fread(text,1,n,f)==(size_t)n);fclose(f);memcpy(before,text,n);
 unsigned char *map=ios_x18_build_data_map(text,n);assert(map);
 if(!strcmp(argv[2],"negative")){bad_pass(text,n,map);fprintf(stderr,"negative control failed to trigger\n");return 2;}
 good_pass(text,n,map);unsigned changed=0,static_reads=0;
 for(size_t i=0;i+12<=(size_t)n;i+=4){uint32_t *p=(uint32_t*)(before+i);unsigned reg=p[0]&31;
  if((p[0]&0xffffffe0)==0xd53bd060 && p[1]==(0x927df000u|(reg<<5)|reg) && (p[2]&0xffc003ff)==(0xf9400000u|(reg<<5)|reg)){
   static_reads++;
   assert((*(uint32_t*)(text+i+8)&0x003ffc00)==((0x900/8)<<10));
  }
 }
 for(size_t i=0;i<(size_t)n;i++)changed+=text[i]!=before[i];
 printf("triplets=%u changed_bytes=%u\n",static_reads,changed);
 assert(static_reads==6 && changed==6);
 // All three words of a literal-marked sequence are excluded independently.
 for(unsigned word=0;word<3;word++){
  uint32_t probe[]={0xd53bd070,0x927df210,0xf9444e10};unsigned char flags=1u<<word;
  good_pass((char*)probe,sizeof(probe),&flags);assert(probe[2]==0xf9444e10);
 }
 free(map);free(before);free(text);printf("PASS six actual FEX triplets and three literal exclusions; packed bitmap stays in bounds\n");return 0;
}
