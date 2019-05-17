/* simple cache simulation
 *
 * CPSC 3300 / Clemson University / Fall 2013
 *
 * 4 KB four-way set-associative cache, 32 bytes/line
 *
 *   => 128 total lines, 4 banks, 32 lines/bank
 *   => 32-bit address partitioned into
 *         22-bit tag
 *          5-bit index         [ 5 = log2( 32 lines/bank ) ]
 *          5-bit byte offset   [ 5 = log2( 32 bytes/line ) ]
 *
 * index             bank 0         bank 1         bank 2         bank 3
 * (=set) repl     v tag cont     v tag cont     v tag cont     v tag cont
 *        +--+    +-+---+----+   +-+---+----+   +-+---+----+   +-+---+----+
 *   0    |  |    | |   |////|   | |   |////|   | |   |////|   | |   |////|
 *        +--+    +-+---+----+   +-+---+----+   +-+---+----+   +-+---+----+
 *   1    |  |    | |   |////|   | |   |////|   | |   |////|   | |   |////|
 *        +--+    +-+---+----+   +-+---+----+   +-+---+----+   +-+---+----+
 *        ...       ...            ...            ...            ...
 *        +--+    +-+---+----+   +-+---+----+   +-+---+----+   +-+---+----+
 *  31    |  |    | |   |////|   | |   |////|   | |   |////|   | |   |////|
 *        +--+    +-+---+----+   +-+---+----+   +-+---+----+   +-+---+----+
 *
 *       (//// - cache line contents are not represented in this simulation)
 *
 * pseudo-lru replacement using a three-bit state scheme
 *
 *   state represents a binary decision tree with 1 indicating that less
 *   significant side has been more recently referenced
 *
 *                        are all 4 lines valid?
 *                             /       \
 *                           yes        no, use an invalid line
 *                            |
 *                       bit 0 == 0?
 *                        /       \
 *                       y         n
 *                      /           \
 *               bit 1 == 0?    bit 2 == 0?
 *                 /    \          /    \
 *                y      n        y      n
 *               /        \      /        \
 *   replace:  line 0  line 1  line 2  line 3
 *
 *   you can implement a pLRU bank indicator with an 8x2 ROM (input is
 *   3-bit state, and output is bank indicator), and you can implement
 *   the next state with a 32x3 ROM (input is 3-bit state appended with
 *   2-bit bank reference, and output is the next 3-bit state)
 *
 *     state | replace      ref to | next state
 *     ------+--------      -------+-----------
 *      00x  |  line 0      line 0 |    11_
 *      01x  |  line 1      line 1 |    10_
 *      1x0  |  line 2      line 2 |    0_1
 *      1x1  |  line 3      line 3 |    0_0
 *
 *   'x' means don't care; '_' means unchanged
 *
 * program input: 32-bit addresses (read as hex values)
 * program output: cache stats
 *
 */

#include<stdio.h>
#include<stdlib.h>

#define LINES_PER_BANK 32

unsigned int

  plru_state[LINES_PER_BANK],  /* current state for each set    */

  valid[4][LINES_PER_BANK],    /* valid bit for each line       */

  tag[4][LINES_PER_BANK],      /* tag bits for each line        */

                               /* line contents are not tracked */

  plru_bank[8] /* table for bank replacement choice based on state */

                 = { 0, 0, 1, 1, 2, 3, 2, 3 },

  next_state[32] /* table for next state based on state and bank ref */
                 /* index by 5-bit (4*state)+bank [=(state<<2)|bank] */

                                    /*  bank ref  */
                                    /* 0  1  2  3 */

                 /*         0 */  = {  6, 4, 1, 0,
                 /*         1 */       7, 5, 1, 0,
                 /*         2 */       6, 4, 3, 2,
                 /* current 3 */       7, 5, 3, 2,
                 /*  state  4 */       6, 4, 1, 0,
                 /*         5 */       7, 5, 1, 0,
                 /*         6 */       6, 4, 3, 2,
                 /*         7 */       7, 5, 3, 2  };

void cache_init(void){
  int i;
  for(i=0;i<LINES_PER_BANK;i++){
    plru_state[i] = 0;
    valid[0][i] = tag[0][i] = 0;
    valid[1][i] = tag[1][i] = 0;
    valid[2][i] = tag[2][i] = 0;
    valid[3][i] = tag[3][i] = 0;
  }
}

int main(void){

  unsigned int
    addr_tag,    /* tag bits of address     */
    addr_index,  /* index bits of address   */
    bank;        /* bank that hit, or bank chosen for replacement */

    addr_index = (address >> 5) & 0x1f;
    addr_tag = address >> 10;

    /* check bank 0 hit */

    if(valid[0][addr_index] && (addr_tag==tag[0][addr_index])){
      hits++;
      bank = 0;

    /* check bank 1 hit */

    }else if(valid[1][addr_index] && (addr_tag==tag[1][addr_index])){
      hits++;
      bank = 1;

    /* check bank 2 hit */

    }else if(valid[2][addr_index] && (addr_tag==tag[2][addr_index])){
      hits++;
      bank = 2;

    /* check bank 3 hit */

    }else if(valid[3][addr_index] && (addr_tag==tag[3][addr_index])){
      hits++;
      bank = 3;

    /* miss - choose replacement bank */

    }else{
      misses++;

           if(!valid[0][addr_index]) bank = 0;
      else if(!valid[1][addr_index]) bank = 1;
      else if(!valid[2][addr_index]) bank = 2;
      else if(!valid[3][addr_index]) bank = 3;
      else {bank = plru_bank[ plru_state[addr_index] ];
      if (dirty[bank][addr_index] == dirty) {
        writeback+=1;
      }}

      valid[bank][addr_index] = 1;
      tag[bank][addr_index] = addr_tag;
      dirty[bank][addr_index] = notdirty;
    }

    /* update replacement state for this set (i.e., index value) */

    plru_state[addr_index] = next_state[ (plru_state[addr_index]<<2) | bank ];
    if (access == write)
      dirty[bank][addr_index] = dirty;
  }

  printf("%d misses of %d references (%5.2f%% hit rate)\n",misses,hits+misses,
    100.0*(float)hits/(float)(hits+misses));

  return 0;
}
