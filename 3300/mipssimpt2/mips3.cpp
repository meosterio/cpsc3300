/* simple cache simulation
 *
 * CPSC 3300 / Clemson University / Fall 2018
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


/****	Alex Moore
			alex9@clemson.edu
			compiled with g++

			*/

#include <iostream>
#include <iomanip>
#include <map>
#include <string>

using namespace std;

#define maxReg 32
#define maxMem 16384
#define LINES_PER_BANK 32
int registers[maxReg];
unsigned int mem[maxMem];
int pc = 0;
int temppc=0;
int numAlu=0;
int btaken=0;
int bnottaken=0;
int jump=0;
int jumplink=0;
int halt = 0;
int loads = 0;
int stores = 0;
int ic = 0;
unsigned int rs;
unsigned int rt;
unsigned int rd;
unsigned int sign;
unsigned int shift;
unsigned int hits = 0;
unsigned int misses = 0;
unsigned int writeback = 0;
unsigned int here = 0;
int dirt = 1;
int notdirty = 0;
int read = 0;
int write = 1;

map<int, string> opcode;
map<int, string> func;

void cache(unsigned int, unsigned int);

void do_opcode(string name, int rs, int rt, int rd, int sign){
	if (name == "addiu") {
		registers[rt] = registers[rs] + sign;
		numAlu+=1;
		pc++;
	}
	if (name == "beq") {
		temppc = pc;
		if (registers[rs] == registers[rt]) {
			pc+=1;
			pc += sign;
			pc = pc&0xffff;
			btaken++;
			cout << "beq";//   - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			pc = temppc;
			pc++;
		}
	}
	if (name == "bgtz") {
		temppc = pc;
		if (int(registers[rs]) > 0) {
			pc+=1;
			pc += sign;
			pc = pc&0xffff;
			btaken++;
		}
		else{
			bnottaken++;
			pc++;
		}
	}
	if (name == "blez") {
		temppc = pc;
		if (int(registers[rs]) <= 0) {
			pc+=1;
			pc += sign;
			pc = pc&0xffff;
			btaken++;
		}
		else{
			bnottaken++;
			pc++;
		}
	}
	if (name == "bne") {
		temppc = pc;
		if (registers[rs] != registers[rt]) {
			pc+=1;
			pc += sign;
			pc = pc&0xffff;
			btaken++;
			pc;
		}
		else{
			bnottaken++;
			pc = temppc;
			pc++;
		}
	}
	if (name == "j") {
		pc = sign;
		jump++;
	}
	if (name == "jal") {
		registers[31] = pc + 1;
		pc = sign;
		jumplink++;
	}
	if (name == "lui") {
		registers[rt] = sign << 16;
		numAlu++;
		pc++;
	}
	if (name == "lw") {
		registers[rt] = mem[(registers[rs]+sign)];
		loads++;
		if (rt == 0) {
			registers[rt] = 0;
		}
    here = (registers[rs] << 2) + sign;
    cache(here, read);
		pc++;
	}
	if (name == "mul") {
		registers[rd] = registers[rs]*registers[rt];
		numAlu++;
		pc++;
	}
	if (name == "slti") {
		if (registers[rs] < sign)
			registers[rt] = 1;
		else
			registers[rt] = 0;
		numAlu++;
		pc++;
	}
	if (name == "sw") {
		mem[(registers[rs] + sign)] = registers[rt];
		stores++;
    here = (registers[rs] << 2) + sign;
    cache(here, write);
		pc++;
	}
	if (name == "xori") {
		registers[rt] = registers[rs] ^ sign;
		numAlu++;
		pc++;
	}
}

void do_func(string name, int rs, int rt, int rd, int shift) {
	if (name == "addu") {
		registers[rd] = registers[rs] + registers[rt];
		numAlu+=1;
		pc++;
	}
	if (name == "and") {
		registers[rd] = registers[rs] & registers[rt];
		numAlu++;
		pc++;
	}
	if (name == "sll") {
		registers[rd] = registers[rt] << shift;
		numAlu++;
		pc++;
	}
	if (name == "jalr") {
		registers[rd] = pc + 1;
		pc = registers[rs];
		jumplink++;
	}
	if (name == "jr") {
		pc = registers[rs];
		jump++;
	}
	if (name == "nor") {
		registers[rd] = ~(registers[rs]|registers[rt]);
		numAlu++;
		pc++;
	}
	if (name == "or") {
		registers[rd] = (registers[rs]|registers[rt]);
		numAlu++;
		pc++;
	}
	if (name == "sra") {
		registers[rd] = registers[rt]>>shift;
		numAlu++;
		pc++;
	}
	if (name == "srl") {
		registers[rd] = registers[rt] >> shift;
		numAlu++;
		pc++;
	}
	if (name == "subu") {
		registers[rd] = registers[rs] - registers[rt];
		numAlu+=1;
		pc++;
	}
	if (name == "xor") {
		registers[rd] = registers[rs] ^ registers[rt];
		numAlu++;
		pc++;
	}
}


unsigned int

  dirty[4][LINES_PER_BANK],

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
    valid[0][i] = tag[0][i] = 0; dirty[0][i] = notdirty;
    valid[1][i] = tag[1][i] = 0; dirty[1][i] = notdirty;
    valid[2][i] = tag[2][i] = 0; dirty[2][i] = notdirty;
    valid[3][i] = tag[3][i] = 0; dirty[3][i] = notdirty;
  }
}

void cache(unsigned int address, unsigned int access) {
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
      if (dirty[bank][addr_index] == dirt) {
        writeback+=1;
      }}

      valid[bank][addr_index] = 1;
      tag[bank][addr_index] = addr_tag;
      dirty[bank][addr_index] = notdirty;
    }

    /* update replacement state for this set (i.e., index value) */
    plru_state[addr_index] = next_state[ (plru_state[addr_index]<<2) | bank ];
    if (access == write)
      dirty[bank][addr_index] = dirt;
}

int main (void) {
  opcode[9] = "addiu"; opcode[4] = "beq"; opcode[7] = "bgtz";
  opcode[6] = "blez"; opcode[5] = "bne"; opcode[2] = "j";
  opcode[3] = "jal"; opcode[15] = "lui"; opcode[35] = "lw";
  opcode[28] = "mul"; opcode[10] = "slti"; opcode[43] = "sw";
  opcode[14] = "xori";

  func[33] = "addu"; func[36] = "and"; func[0] = "sll";
  func[9] = "jalr"; func[8] = "jr"; func[39] = "nor";
  func[37] = "or"; func[3] = "sra"; func[2] = "srl";
  func[35] = "subu"; func[38] = "xor";


	string inst; //string to hold the opcode name

  for (int i = 0; i<maxReg; i++) { //initialize the array
    registers[i] = 0x0;
  }

	unsigned int input;
  int i = 0;
  while (cin >> hex >> input) { //read in the values
    mem[i] = input;
    i++;
  }

  cout << "contents of memory" << endl; //print out the memory
  cout << "addr value" << endl;
  for (int j=0; j<i; j++) {
    cout << setw(3) << setfill('0') << j;
    cout << ": " <<setw(8) << setfill('0') << hex << mem[j] << endl;
  }
  cout << endl;

  cout << "behavioral simulation of simple MIPS-like machine" << endl;
  cout << "  (all values are shown in hexadecimal)" << endl;
  cout << endl;

  cache_init();

  while (pc<i && mem[pc] != 0) {
		ic++;

		int x = (mem[pc]>>26)&0x3f;
    rs = (mem[pc] >> 21)&0x1f;
    rt = (mem[pc] >> 16)&0x1f;
    rd = (mem[pc] >> 11)&0x1f;
    sign = (mem[pc]&0x0000ffff);
    if ((sign >> 15) & 1) {
      sign = 0xFFFF - sign + 1;
      sign = sign * -1;
    }
    shift = (mem[pc] >> 6)&0x1f;
    if (x != 0) {
      inst = opcode[x];
			if (pc == (i-1))
				cout << "hlt" << endl;
			else
				do_opcode(inst, rs, rt, rd, sign);
    }

    else{
      if (mem[pc] == 0) {
        pc+=100;
      }
			else {
        x = (mem[pc]&0x0000003f);
        inst = func[x];
				do_func(inst, rs, rt, rd, shift);
      }
    }
    if (rs == 0 || rt == 0 || rd == 0) {
      registers[0] = 0;
    }
  }

	cout << endl;

	int jumpsandbranch = jump + jumplink + btaken + bnottaken;
	int loadsandstore = loads + stores;
	int instcount = numAlu + jumpsandbranch + loadsandstore;
	int memcount = loadsandstore + (ic);

	cout << "instruction class counts (omits hlt instruction)" << endl;
	cout << "  alu ops             " << dec << numAlu << endl;
	cout << "  loads/stores        " << loadsandstore << endl;
	cout << "  jumps/branches      " << jumpsandbranch << endl;
	cout << "total                 " << instcount << endl;
	cout << endl;

	cout << "memory access counts (omits hlt instruction)" << endl;
	cout << "  inst. fetches       " << (ic) << endl;
	cout << "  loads               " << loads << endl;
	cout << "  stores              " << stores << endl;
	cout << "total                 " << memcount << endl;
	cout << endl;

	cout << "transfer of control counts" << endl;
	cout << "  jumps               " << jump << endl;
	cout << "  jump-and-links      " << jumplink << endl;
	cout << "  taken branches      " << btaken << endl;
	cout << "  untaken branches    " << bnottaken << endl;
	cout << "total                 " << jumpsandbranch << endl;
  cout << endl;


  cout << "data cache counts" << endl;
  cout << "  hits                " << hits << endl;
  cout << "  misses              " << misses << endl;
  cout << "  write backs         " << writeback << endl;

  return 0;
}
