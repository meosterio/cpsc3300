/**
If you had any errors for Project 1, you should first correct your behavioral
simulation of the simple MIPS-like instruction set. You can then extend your
simulation to pair instructions according to the following rules.

You are modeling an implementation with two pipelines, and the implementation
will try to dynamically issue instructions in pairs in program order. To handle
control hazards, only one control transfer instruction can be issued at a time.
Furthermore, only one of the pipelines can support multiply instructions, and
only one of the pipelines can support load or store instructions. Thus only one
multiply instruction can issue at a time, and only one load or store can issue
at a time. Both pipelines can handle the remaining ALUop instructions. However,
since the instruction-pairing logic operates in program order, if there is an
RAW or WAW register data hazard between the two instructions being considered
for pairing, the issue of the dependent instruction must be delayed. (Since
registers are read for both instructions early in the pipelines, a WAR
dependency between the two instructions will not cause a data hazard.)

The instruction classifications for pairing are:
  Control:  beq, bgtz, blez, bne, hlt, jal, jalr, j, jr
  Load/Store:  lw, sw
  Multiply:  mul
  ALUops:  addu, addiu, and, lui, nor, or, sll, slti, sra, srl, subu, xor, xori

Instruction pairing fails under these three conditions:

Control hazard: A control instruction is picked for issue slot 1.
Structural hazard:
A load/store instruction would be placed in issue slot 2, but there is already a
load/store instruction in issue slot 1.
A multiply instruction would be placed in issue slot 2, but there is already a
multiply instruction in issue slot 1.
Data hazard: The instruction that would be placed in issue slot 2 has a RAW or
WAW register data dependency with the instruction that is already in issue
slot 1.
When pairing fails, the issue logic is reset and the instruction that was being
considered for issue slot 2 is instead placed in a new instance of issue slot 1.
The instruction following it is then considered for issue slot 2.
*/

/****
	compiled with g++
*/

#include <iostream>
#include <iomanip>
#include <map>
#include <string>

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
int issuecount = 0;
int dubissue = 0;
int controlstop = 0;
int structuralstop = 0;
int datadependstop = 0;
int data_structural = 0;
bool dub = false;
bool hlthere = false;
bool flag = true;
unsigned int rsd = 0;
unsigned int rtd = 0;
unsigned int rdd = 0;
unsigned int rs;
unsigned int rt;
unsigned int rd;
unsigned int sign;
unsigned int shift;
string why;

map<int, string> opcode;
map<int, string> func;

void do_opcode(string name, int rs, int rt, int rd, int sign){
	if (name == "addiu") {
		registers[rt] = registers[rs] + sign;
		numAlu+=1;
		cout << "addiu";// - register r[" << rt <<"] now contains 0x" << hex << setw(8)
			//<< setfill('0') << registers[rt];
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
			cout << "beq";//   - branch untaken";
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
			cout << "bgtz";//  - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "bgtz";//   - branch untaken" << setw(8) << setfill('0') << hex <<
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
			cout << "blez";//  - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "blez";//  - branch untaken";
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
			cout << "bne";//   - branch taken to 0x" << setw(8) << setfill('0') << hex <<
			pc;
		}
		else{
			bnottaken++;
			cout << "bne";//   - branch untaken";
			pc = temppc;
			pc++;
		}
	}
	if (name == "j") {
		pc = sign;
		jump++;
		cout << "j";//     - jump to 0x" << setw(8) << setfill('0') << hex << pc;
	}
	if (name == "jal") {
		registers[31] = pc + 1;
		pc = sign;
		jumplink++;
		cout << "jal";//   - jump to 0x" << setw(8) << setfill('0') << hex << pc; //"register r[31] now contains 0x"
		//<< setw(8) << setfill('0') << hex << registers[31];
	}
	if (name == "lui") {
		registers[rt] = sign << 16;
		numAlu++;
		cout << "lui";//   - register r[" << rt << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rt];
		pc++;
	}
	if (name == "lw") {
		registers[rt] = mem[(rs+sign)];
		loads++;
		cout << "lw";//    - register r[" << rt << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rt];
		if (rt == 0) {
			registers[rt] = 0;
			//cout << endl << "***** - register r[0] not allowed to change; reset to 0";
		}
		pc++;
	}
	if (name == "mul") {
		registers[rd] = registers[rs]*registers[rt];
		numAlu++;
		cout << "mul";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "slti") {
		if (registers[rs] < sign)
			registers[rt] = 1;
		else
			registers[rt] = 0;
		numAlu++;
		cout << "slti";//  - register r[" << rt << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rt];
		pc++;
	}
	if (name == "sw") {
		mem[(rs + sign)] = registers[rt];
		stores++;
		cout << "sw";//    - register r[" << rt << "] value stored in memory";
		pc++;
	}
	if (name == "xori") {
		registers[rt] = registers[rs] ^ sign;
		numAlu++;
		cout << "xori";//  - register r[" << rt << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rt];
		pc++;
	}
}

void do_func(string name, int rs, int rt, int rd, int shift) {
	if (name == "addu") {
		registers[rd] = registers[rs] + registers[rt];
		numAlu+=1;
		cout << "addu";//  - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "and") {
		registers[rd] = registers[rs] & registers[rt];
		numAlu++;
		cout << "and";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "sll") {
		registers[rd] = registers[rt] << shift;
		numAlu++;
		cout << "sll";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "jalr") {
		registers[rd] = pc + 1;
		pc = registers[rs];
		jumplink++;
		cout << "jalr";//  - jump to " << hex << pc << "register r[" << rd <<
		//"] now contains 0x" << setw(8) << setfill('0') << hex << registers[rd];
	}
	if (name == "jr") {
		pc = registers[rs];
		jump++;
		cout << "jr";//    - jump to 0x" << setw(8) << setfill('0') << hex << pc;
	}
	if (name == "nor") {
		registers[rd] = ~(registers[rs]|registers[rt]);
		numAlu++;
		cout << "nor";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "or") {
		registers[rd] = (registers[rs]|registers[rt]);
		numAlu++;
		cout << "or";//    - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "sra") {
		registers[rd] = registers[rt] >> shift;
		numAlu++;
		cout << "sra";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "srl") {
		registers[rd] = registers[rt] >> shift;
		numAlu++;
		cout << "srl";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "subu") {
		registers[rd] = registers[rs] - registers[rt];
		numAlu+=1;
		cout << "subu";//  - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
	if (name == "xor") {
		registers[rd] = registers[rs] ^ registers[rt];
		numAlu++;
		cout << "xor";//   - register r[" << rd << "] now contains 0x" << setw(8) <<
		//setfill('0') << hex << registers[rd];
		pc++;
	}
}

bool raw (unsigned int r) {
  if (rsd == r || rtd == r) {
    if (dub) {
      dub = false;
      datadependstop++;
      why = "// data dependency stop";
      return true;
    }
    else {
      why += " (also data dep.)";
      data_structural++;
      return true;
    }
  }
  return false;
}

void waw (unsigned int r, int inst2) {
  int whichone;
  if (func.find(inst2) != func.end() || inst2 == 28) {
    whichone = rdd;
  }
  else {
    whichone = rtd;
  }
  if (whichone == r) {
    if (dub) {
      dub = false;
      datadependstop++;
      why = "// data dependency stop";
    }
    else {
      data_structural++;
      why += " (also data dep.)";
    }
  }
}

void controlHazard(int inst1, int inst2) {
  hlthere = false;
  if (inst1 == 4 || inst1 == 7 || inst1 == 6 || inst1 == 5) {
    dub = false;
    controlstop++;
    hlthere = true;
    why = "// control stop";
  }
  if (flag == true && (inst1 == 2 || inst1 == 3)) {
    dub = false;
    controlstop++;
    hlthere = true;
    why = "// control stop";
  }
  if (flag == false) {
    if (inst1 == 8 || inst1 == 9) {
      dub = false;
      controlstop++;
      hlthere = true;
      why = "// control stop";
    }
  }
}

void structureHazard(int inst1, int inst2) {
  if (inst1 == 35 || inst1 == 43) {
    if (inst2 == 35 || inst2 == 43) {
      dub = false;
      structuralstop++;
      why = "// structural stop";
    }
  }
  if (inst1 == 28 && inst2 == 28) {
    dub = false;
    structuralstop++;
    why = "// structural stop";
  }
}

void dataHazard(int inst1, int inst2) {
  bool f = false;
  if (inst1 != 43) {
    if (inst1 == 28 || func.find(inst1) != func.end()) {
      f = raw(rt);
    }
    else {
      f = raw(rd);
    }
  }
  if (!f) {
    if (inst1 != 43) {
      if (inst2 == 28 || func.find(inst2) != func.end()) {
        waw(rt, inst2);
      }
      else {
        waw(rd, inst2);
      }
    }
  }
}

void duble(int inst1, int inst2) {
  dub = true;
  why = "// -- double issue";
  if (flag == false && inst1 == 0) {
    dub = false;
    controlstop++;
    hlthere = true;
    why = "// control stop";
  }
  controlHazard(inst1, inst2);
  structureHazard(inst1, inst2);
  if (!hlthere) {
    dataHazard(inst1, inst2);
  }
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
  string inst2;

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

  cout << "simple MIPS-like machine with instruction pairing" << endl;
  cout << "  (all values are shown in hexadecimal)" << endl;
  cout << endl;


  cout << "instruction pairing analysis" << endl;

  while (pc<i && mem[pc] != 0) {
    issuecount++;
		ic++;
    cout << setw(3) << setfill('0') << pc << ": ";

		int x = (mem[pc]>>26)&0x3f;
    rs = (mem[pc] >> 21)&0x1f;
    rt = (mem[pc] >> 16)&0x1f;
    rd = (mem[pc] >> 11)&0x1f;
    sign = (mem[pc]&0x0000ffff);
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
        cout << "   " << setw(3) << setfill('0') << pc << ": ";
        cout << "hlt ";
        pc+=100;
      }
			else {
        x = (mem[pc]&0x0000003f);
        inst = func[x];
				do_func(inst, rs, rt, rd, shift);
      }
    }
    if (pc < i) {
      int x2 = (mem[pc]>>26)&0x3f;
      rsd = (mem[pc] >> 21)&0x1f;
      rtd = (mem[pc] >> 16)&0x1f;
      rdd = (mem[pc] >> 11)&0x1f;
      sign = (mem[pc]&0x0000ffff);
      shift = (mem[pc] >> 6)&0x1f;
      duble(x, x2);
      if (dub == true) {
        ic++;
        dub = false;
        dubissue++;
        if (x2 != 0) {
          inst2 = opcode[x2];
          cout << "   " << setw(3) << setfill('0') << pc << ": ";
          do_opcode(inst2, rsd, rtd, rdd, sign);
          cout << " ";
        }
        else {
          if (mem[pc] == 0) {
            cout << "   " << setw(3) << setfill('0') << pc << ": ";
    				cout << "hlt ";
    				pc+=100;
    			}
          else {
            flag = false;
            x2 = (mem[pc]&0x0000003f);
            inst2 = func[x2];
            cout << "   " << setw(3) << setfill('0') << pc << ": ";
            do_func(inst2, rsd, rtd, rdd, shift);
            cout << " ";
          }
        }
      }
      else{
        cout << "             ";
      }
      cout << why << "\r\n";
    }
    flag = true;
  }
  if (why != "// -- double issue") {
    controlstop++; issuecount++;
	  cout << setw(3) << setfill('0') << pc << ": hlt" << "            // control stop" << endl << endl;
  }

	/*cout << "contents of memory" << endl; //print out the memory
	cout << "addr value" << endl;
	for (int j=0; j<i; j++) {
    cout << setw(3) << setfill('0') << j;
    cout << ": " <<setw(8) << setfill('0') << hex << mem[j] << endl;
  }*/
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

  double percentdub = double(dubissue);
  percentdub /= double(issuecount);
  percentdub *= 100;

  cout << "instruction pairing counts (includes hlt instruction)" << endl;
  cout << "  issue cycles        " << issuecount << endl;
  cout << "  double issues       " << dubissue << " ( " << percentdub <<
  " percent of issue cycles)" << endl;
  cout << "  control stops       " << controlstop << endl;
  cout << "  structural stops    " << structuralstop << " ( " << data_structural
  << " of which would also stop on a data dep.)" << endl;
  cout << "  data dep. stops     " << datadependstop << endl;

  return 0;
}
