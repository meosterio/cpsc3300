#include <fstream>
#include <map>
#include <ios>
#include <iostream>
#include <string>
#include <iomanip>
#include <climits>
using namespace std;

#define RAM_SIZE 1024
#define NUM_REGISTERS 32


// Authors: Andrew Weathers and Nicholas Muenchen
// Date: 21 September 2018
// Purpose: Simulate a simplified MIPS-like
//			instruction set

//opcode: 6 bits
//rs: 5 bits
//rt: 5 bits
//rd: 5 bits
//shift: 5 bits
//funct: 6 bits
unsigned int mar,
			 mdr,
			 pc,
 			 ir,
 			 rd,
 			 rs,
 			 rt,
 			 shift,
			 funct,
			 numAlu = 0,
			 numInstFetch = 0,
			 numLoads = 0,
			 numStores = 0,
			 halt = 0,
			 numJumps = 0,
			 numJumpsAndLinks = 0,
			 numTakenBranches = 0,
			 numUnTakenBranches = 0,
			 registerArray[NUM_REGISTERS],
			 ram[RAM_SIZE];

int sign_ext,
			 ram_end = 0;



map<unsigned int, string> opcodeMap;

void initiliazeRam()
{
	for(int i = 0; i < RAM_SIZE; i++)
	{
		ram[i] = UINT_MAX;
	}
}


void fillMap()
{
  opcodeMap[0x00] = "r";
  opcodeMap[0x09] = "i";
}


//Adds the number in rs to the number in rt, then stores in rd
void addu()
{
  registerArray[rd] = registerArray[rs] + registerArray[rt];
  numAlu++;
  cout << (pc - 1) << ": addu  - regiseter r[" << rd << "] now contains " << std::hex << registerArray[rd] << endl;
}

//Adds the number in rs to the immediately given value, then stores in rt
void addiu()
{
  registerArray[rt] = registerArray[rs] + sign_ext;
  numAlu++;
   cout << (pc - 1) << ": addiu  - register r[" << rt << "] now contains " << std::hex << registerArray[rt] << endl;
}

//Performs bitwise AND operation rs*rt, then stores in rd
void _and()
{
	registerArray[rd] = registerArray[rs] & registerArray[rt];
	numAlu++;
	cout << (pc - 1) << ": and  - register r[" << rd << "] now contains " << hex << registerArray[rd] << endl;

//Logically shifts register rt right by shift and stores the result in rd, fills with ones or zeroes depending on s
}


//Branch is rs is equal to rt. Branches to immediate value.
void beq()
{
	if (registerArray[rs] == registerArray[rt])
	{
		pc += sign_ext;
		numTakenBranches++;
		cout << (pc - 1) << ": beq  - branch taken to " << hex << pc << endl;
	}
	else
	{
		numUnTakenBranches++;
		cout << (pc - 1) << ": beq  - branch untaken" << endl;
	}


}

//Branch if r[rs] > 0 to the pc + signed immediate.
void bgtz()
{
	if (int(registerArray[rs]) > 0)
	{
		pc += sign_ext;
		numTakenBranches++;
		cout << (pc - 1) << ": bgtz  - branch taken to " << hex << pc << endl;
	}
	else
	{
		numUnTakenBranches++;
		cout << (pc - 1) << ": bgtz  - branch untaken" << endl;
	}
}

//Branch if r[rs] <= 0 to the pc + signed immediate.
void blez()
{
	if (int(registerArray[rs]) <= 0)
	{
		pc += sign_ext;
		numTakenBranches++;
		cout << (pc - 1) << ": blez  - branch taken to " << hex << pc << endl;

	}
	else
	{
		numUnTakenBranches++;
		cout << (pc - 1) << ": blez  - branch untaken" << endl;
	}
}

//Branch if registerArray[rs] is not equal to registerArray[rt], branch to pc + signed immediate.
void bne()
{
	if (registerArray[rs] != registerArray[rt])
	{
		pc += sign_ext;
		numTakenBranches++;
		cout << (pc - 1) << ": bne  - branch taken to " << hex << pc << endl;

	}
	else
	{
		numUnTakenBranches++;
		cout << (pc - 1) << ": bne  - branch untaken" << endl;
	}
}

//Halts execution
void hlt()
{
	cout << (pc - 1) << ": hlt" << endl;
  return;
}

//Jump to target memory location and store index in pc.
void j()
{
	pc = sign_ext;
	numJumps++;
}

//Jump and link jumps, but also stores pc
// in given register.
void jal()
{
	registerArray[31] = pc;
	pc = sign_ext;
	numJumpsAndLinks++;
}

//Store incremented pc in rd and jump to rs.
//Value in rs is now stored in the pc.
void jalr()
{
	registerArray[rd] = pc;
	pc = registerArray[rs];
	numJumpsAndLinks++;

}

//A given register is jumped to and
// is loaded in the pc
void jr()
{
	pc = registerArray[rs];
	numJumps++;
}

//Shifts immediate value to the upper 16 bits with trailing 0's.
//The result is stored in register rt
void lui()
{
	registerArray[rt] = sign_ext << 16;
	numLoads++;
}

//Load value in rt from memory + any sign_ext which may apply
void lw()
{
	registerArray[rt] = ram[rs+sign_ext];
	numLoads++;
}

//multiplies values in rs and rt and places the result into rd
void mul()
{
	registerArray[rd] = registerArray[rs]*registerArray[rt];
	numAlu++;
}

//nor's register rs and rt and places the result into rd
void nor()
{
	registerArray[rd] = !(registerArray[rs] | registerArray[rt]);
	numAlu++;
}

//or's register rs and register rt and places the result into rd
void _or()
{
	registerArray[rd] = registerArray[rs] | registerArray[rt];
	numAlu++;
}

//Shifts register rt left logically by shift and stores the result in rd
/////////////////////////////////

//Logically shifts register rt right by shift and stores the result in rd, fills with ones or zeroes depending on s
//UNSURE ABOUT THE REGISTERS TO BE USED, ALSO

//Logically shifts register rt right by shift and stores the result in rd, fills with ones or zeroes depending on sUNSURE ABOUT IMPLEMENTATION
/////////////////////////////////
void sll()
{
	registerArray[rd] = registerArray[rt] << shift;
	numAlu++;
}

//If register rs < sign_ext, then set register rt to 1 else set to 0
void stli()
{
	if (registerArray[rs] < sign_ext)
	{
		registerArray[rd] = 1;
	}
	else
	{
		registerArray[rd] = 0;
	}
	numAlu++;
}


//Logically shifts register rt right by shift and stores the result in rd, fills with ones or zeroes depending on sign
/////////////////////////////////
//UNSURE ABOUT THE REGISTERS TO BE USED, ALSO UNSURE ABOUT IMPLEMENTATION
/////////////////////////////////
void sra()
{
	registerArray[rd] = registerArray[rt] >> shift;
	numAlu++;
}


//Logically shifts register rt right by shift and stores the result in rd, fills with zeroes
/////////////////////////////////
//UNSURE ABOUT THE REGISTERS TO BE USED
/////////////////////////////////
void srl()
{
	registerArray[rd] = (unsigned int )(registerArray[rt]) >> shift;
	numAlu++;
}

//Subtract register rt from register rs and save the result into rd
void subu()
{
	registerArray[rd] = registerArray[rs]  - registerArray[rt];
	numAlu++;
}

//Stores the word in r[t] at registerArray[registerArray[rs] + sign_imm
void sw()
{
	ram[registerArray[rs] + sign_ext] = registerArray[rt];
	numStores++;
}

//Exclusive or's registerArray[rs] and registerArray[rt] then stores the result in registerArray[rd]
void _xor()
{
	registerArray[rd] = registerArray[rs] ^ registerArray[rt];
	numAlu++;
}


//Exclusive or's registerArray[rs] with sign_ext and stores the result in registerArray[rt]
void xori()
{
	registerArray[rt] = registerArray[rs] ^ sign_ext;
	numAlu++;
}




//Fetches the next instruction.
void fetch()
{
  mar = pc;
  mdr = registerArray[mar];
  ir = mdr;
  pc++;
  numInstFetch++;
}

/*
rs = (ir >> 21) & 0x1f; // clamps to the 5 bit rs
rt = (ir >> 16) & 0x1f; // clamps to the 5 bit rt
rd = (ir >> 11) & 0x1f; // clamps to the 5 bit rd
shift = (ir >> 6) & 0x1f; // clamps to the 5 bit shift
rs = ir & 0x2f; // clamps to the 6 bit funct
*/


void (*imm_func())()
{
  unsigned int opcode = (ir >> 26) & 0x3f; // clamp to 6-bit opcode field
  rs = (ir >> 21) & 0x1f; // clamp to the 5 bit rs
  rt = (ir >> 16) & 0x1f; // clamp to the 5 bit rt
  sign_ext = (ir) & 0xffff; // clamp to 16 bit immediate value

  if(opcode == 0x09)
  {
    return addiu;
  }
	if(opcode == 0x04)
	{
		return beq;
	}
	if(opcode == 0x07)
	{
		return bgtz;
	}
	if(opcode == 0x06)
	{
		return blez;
	}
	if(opcode == 0x05)
	{
		return bne;
	}
	if(opcode == 0x02)
	{
		return j;
	}
	if(opcode == 0x03)
	{
		return jal;
	}
	if(opcode == 0x0f)
	{
		return lui;
	}
	if(opcode == 0x23)
	{
		return lw;
	}
	if(opcode == 0x0a)
	{
		return stli;
	}
	if(opcode == 0x2b)
	{
		return sw;
	}
	if(opcode == 0x0e)
	{
		return xori;
	}




}

void (*other_func())()
{
  unsigned int opcode = (ir >> 26) & 0x3f; // clamp to 6-bit opcode field
  rs = (ir >> 21) & 0x1f; // clamps to the 5 bit rs
  rt = (ir >> 16) & 0x1f; // clamps to the 5 bit rt
  rd = (ir >> 11) & 0x1f; // clamps to the 5 bit rd
  shift = (ir >> 6) & 0x1f; // clamps to the 5 bit shift
  funct = ir & 0x2f; // clamps to the 6 bit funct

  if(opcode == 0x00)
  {
    if(funct == 0x21)
    {
      return addu;
    }
		if(funct == 0x24)
		{
			return _and;
		}
		if(funct == 0x09)
		{
			return jalr;
		}
		if(funct == 0x08)
		{
			return jr;
		}
		if(funct == 0x27)
		{
			return nor;
		}
		if(funct == 0x25)
		{
			return _or;
		}
		if(funct == 0x00)
		{
			return sll;
		}
		if(funct == 0x03)
		{
			return sra;
		}
		if(funct == 0x02)
		{
			return srl;
		}
		if(funct == 0x23)
		{
			return subu;
		}
		if(funct == 0x26)
		{
			return _xor;
		}
	}
	if(opcode == 0x1c)
	{
	return mul;
	}
}




//Right shift by 26 to isolate the opcode

void ( *decode() )()
{
  unsigned int opcode = (ir >> 26) & 0x3f; // clamp to 6-bit opcode field
	if(ir == 0)
	{
		return hlt;
	}
  cout << opcode << " fff" << endl;
  if(opcodeMap.find(opcode) != opcodeMap.end())
  {
    //Finds if the function is immediate
    if(opcodeMap.find(opcode)->second.compare("r"))
    {
      cout << "imm" << endl;
      return imm_func();
    }
    else
    {
      cout << "rrr" << endl;
      return other_func();
    }
  }
  else
  {
    return hlt;
  }
}

void printMemory()
{
	cout << "contents of memory" << endl;
	cout << "addr value" << endl;
	for(int i = 0; i < RAM_SIZE; i++)
	{
		if(ram[i] != UINT_MAX)
		{
			cout << setw(3) << setfill('0') << i;
			cout << ": " << hex << ram[i] << endl;
		}
	}
}

void writeOutput()
{
	printMemory();
	int numJumpsAndBranches = numTakenBranches + numUnTakenBranches + numJumps + numJumpsAndLinks;
	int numLoadsAndStores = numStores + numLoads;
	int totalInstClassCounts = numAlu + numLoadsAndStores + numJumpsAndBranches;
	int totalMemAccess = numLoadsAndStores + numInstFetch;

	cout << "instruction class counts (omits hlt instruction)" << endl;
	cout << "  alu ops             " << numAlu << endl;
	cout << "  loads/stores        " << numLoadsAndStores << endl;
	cout << "  jumps/branches      " << numJumpsAndBranches << endl;
	cout << "total                 " << totalInstClassCounts << endl << endl;

	cout << "memory access counts (omits hlt instruction)" << endl;
	cout << "  inst. fetches       " << numInstFetch << endl;
	cout << "  loads               " << numLoads << endl;
	cout << "  stores              " << numStores << endl;
	cout << "total                 " << totalMemAccess << endl << endl;

	cout << "transfer of control counts" << endl;
	cout << "  jumps               " << numJumps << endl;
	cout << "  jump-and-links      " << numJumpsAndLinks << endl;
	cout << "  taken branches      " << numTakenBranches << endl;
	cout << "  untaken branches    " << numUnTakenBranches << endl;
	cout << "total                 " << numJumpsAndBranches << endl;



}


//Store terminal input into ram
void gatherInput()
{
	unsigned int input;
	int i=0;
	while(cin >> hex >> input)
	{
		ram[ram_end] = input;
		ram_end++;
	}

}

int main()
{
	initiliazeRam();
	fillMap();
	gatherInput();
	void (* inst)();

  while(halt == 0)
	{
		fetch();
		inst = decode();
		(*inst)();
	}
	writeOutput();


  registerArray[0] = 1;
  registerArray[2] = 3;
  registerArray[3] = 4;
  registerArray[1] = 2;



  return 0;
}
