/**
You should program a behavioral simulation of a simple MIPS-like instruction set

The MIPS instruction set is covered in your textbook. We will use a subset of
the instructions and the same instruction formats. However, there are multiple
simplifications:

32-bit memory words with word addressability;
a limited memory size of 1024 words;
branch offsets and targets are not shifted before use;
no jump or branch delay slots (i.e., jumps and branches have immediate effect);
the program starts execution at address zero; and,
no traps/exceptions/interrupts.
*/

/****	Alex Moore
			alex9@clemson.edu
			compiled with g++

			*/

#include <iostream>
#include <iomanip>
#include <map>

using namespace std;

#define maxReg 32
#define maxMem 1024
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

void do_opcode(string name, int rs, int rt, int rd, int sign){
	if (name == "addiu") {
		registers[rt] = registers[rs] + sign;
		numAlu+=1;
		cout << "addiu - register r[" << rt <<"] now contains 0x" << hex << setw(8)
			<< setfill('0') << registers[rt];
			pc++;
	}
	if (name == "beq") {
		temppc = pc;
		if (registers[rs] == registers[rt]) {
			pc+=1;
			pc += sign;
			pc = pc&0xffff;
			btaken++;
			cout << "beq   - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "beq   - branch untaken";
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
			cout << "bgtz  - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "bgtz   - branch untaken" << setw(8) << setfill('0') << hex <<
			pc;
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
			cout << "blez  - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "blez  - branch untaken";
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
			cout << "bne   - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "bne   - branch untaken";
			pc = temppc;
			pc++;
		}
	}
	if (name == "j") {
		pc = sign;
		jump++;
		cout << "j     - jump to 0x" << setw(8) << setfill('0') << hex << pc;
	}
	if (name == "jal") {
		registers[31] = pc + 1;
		pc = sign;
		jumplink++;
		cout << "jal   - jump to 0x" << setw(8) << setfill('0') << hex << pc; //"register r[31] now contains 0x"
		//<< setw(8) << setfill('0') << hex << registers[31];
	}
	if (name == "lui") {
		registers[rt] = sign << 16;
		numAlu++;
		cout << "lui   - register r[" << rt << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rt];
		pc++;
	}
	if (name == "lw") {
		registers[rt] = mem[(rs+sign)];
		loads++;
		cout << "lw    - register r[" << rt << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rt];
		if (rt == 0) {
			registers[rt] = 0;
			cout << endl << "***** - register r[0] not allowed to change; reset to 0";
		}
		pc++;
	}
	if (name == "mul") {
		registers[rd] = registers[rs]*registers[rt];
		numAlu++;
		cout << "mul   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "slti") {
		if (registers[rs] < sign)
			registers[rt] = 1;
		else
			registers[rt] = 0;
		numAlu++;
		cout << "slti  - register r[" << rt << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rt];
		pc++;
	}
	if (name == "sw") {
		mem[(rs + sign)] = registers[rt];
		stores++;
		cout << "sw    - register r[" << rt << "] value stored in memory";
		pc++;
	}
	if (name == "xori") {
		registers[rt] = registers[rs] ^ sign;
		numAlu++;
		cout << "xori  - register r[" << rt << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rt];
		pc++;
	}
}

void do_func(string name, int rs, int rt, int rd, int shift) {
	if (name == "addu") {
		registers[rd] = registers[rs] + registers[rt];
		numAlu+=1;
		cout << "addu  - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "and") {
		registers[rd] = registers[rs] & registers[rt];
		numAlu++;
		cout << "and   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "sll") {
		registers[rd] = registers[rt] << shift;
		numAlu++;
		cout << "sll   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "jalr") {
		registers[rd] = pc + 1;
		pc = registers[rs];
		jumplink++;
		cout << "jalr  - jump to " << hex << pc << "register r[" << rd <<
		"] now contains 0x" << setw(8) << setfill('0') << hex << registers[rd];
	}
	if (name == "jr") {
		pc = registers[rs];
		jump++;
		cout << "jr    - jump to 0x" << setw(8) << setfill('0') << hex << pc;
	}
	if (name == "nor") {
		registers[rd] = ~(registers[rs]|registers[rt]);
		numAlu++;
		cout << "nor   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "or") {
		registers[rd] = (registers[rs]|registers[rt]);
		numAlu++;
		cout << "or    - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "sra") {
		registers[rd] = registers[rt] >> shift;
		numAlu++;
		cout << "sra   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "srl") {
		registers[rd] = registers[rt] >> shift;
		numAlu++;
		cout << "srl   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "subu") {
		registers[rd] = registers[rs] - registers[rt];
		numAlu+=1;
		cout << "subu  - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "xor") {
		registers[rd] = registers[rs] ^ registers[rt];
		numAlu++;
		cout << "xor   - register r[" << rd << "] now contains 0x" << setw(8) <<
		setfill('0') << hex << registers[rd];
		pc++;
	}
}

map<int, string> opcode;
map<int, string> func;

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


  cout << "pc   result of instruction at that location" << endl;

  while (pc<i && mem[pc] != 0) {
		ic++;
    cout << setw(3) << setfill('0') << pc << ": ";

		int x = (mem[pc]>>26)&0x3f;
    if (x != 0) {
      inst = opcode[x];
			unsigned int rs = (mem[pc] >> 21)&0x1f;
			unsigned int rt = (mem[pc] >> 16)&0x1f;
			unsigned int rd = (mem[pc] >> 11)&0x1f;
			unsigned int sign = (mem[pc]&0x0000ffff);

			if (pc == (i-1))
				cout << "hlt" << endl;
			else
				do_opcode(inst, rs, rt, rd, sign);
    }

    else{
      x = (mem[pc]&0x0000003f);
      inst = func[x];
			unsigned int rs = (mem[pc] >> 21)&0x1f;
			unsigned int rt = (mem[pc] >> 16)&0x1f;
			unsigned int rd = (mem[pc] >> 11)&0x1f;
			unsigned int shift = (mem[pc] >> 6)&0x1f;

			if (pc == (i-1) | mem[pc] == 0) {
				cout << "hlt" << endl;
				pc++;
			}
			else
				do_func(inst, rs, rt, rd, shift);
    }

		cout << endl;
  }
	cout << setw(3) << setfill('0') << pc << ": hlt" << endl << endl;

	cout << "contents of memory" << endl; //print out the memory
	cout << "addr value" << endl;
	for (int j=0; j<i; j++) {
    cout << setw(3) << setfill('0') << j;
    cout << ": " <<setw(8) << setfill('0') << hex << mem[j] << endl;
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

  return 0;

}
