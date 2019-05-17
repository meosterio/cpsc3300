/* behaviorial simulation of the simple machine described by Richard Eckert 
 * in "Micro-programmed versus hardwired control units: How computers really
 * work" - URL is http://www.cs.binghamton.edu/~reckert/hardwire3new.html
 *
 * used as example simulator code in CPSC 3300 at Clemson
 *
 * this code simulates the instruction set of Eckert's simple machine but
 * does not show the cycle-by-cycle operations
 *
 * the instructions and data for the simple machine are read as hex values
 * from a file named "ram.txt" in the current directory
 *
 * instructions implemented:
 *   lda - load accumulator, coded as 1xx, where xx is the operand address
 *   sta - store accumulator, coded as 2xx, where xx is the destination
 *           address
 *   add - add value in b register to accumulator, coded as 3yy, where yy
 *           is ignored
 *   sub - subtract value in b register from accumulator, coded as 4yy,
 *           where yy is ignored
 *   mba - make b register same as accumulator (i.e., copy the value in the
 *           accumulator into the b register), coded as 5yy, where yy is
 *           ignored
 *   jmp - unconditional jump, coded as 6xx, where xx is the branch target
 *           address
 *   jn (jneg) - conditional jump, coded as 7xx, where xx is the branch
 *                 target address
 *   hlt - halt, coded as any value 800 to fff, inclusive
 *
 * the program starts execution at address zero
 *
 * an opcode of 0 generates an error message and an exit from the simulator
 *
 * the contents of memory are echoed as they are read in before the simulation
 * begins; the contents are also displayed when a halt instruction is executed
 * so that the changes to memory words caused by store instructions can be
 * verified
 *
 * a simple program to find the difference of two numbers, c = a - b, is:
 *   107    // 0: lda b
 *   500    // 1: mba
 *   106    // 2: lda a
 *   400    // 3: sub
 *   208    // 4: sta c
 *   800    // 5: hlt
 *   5      // 6: a: word 5
 *   2      // 7: b: word 2
 *   0      // 8: c: word 0 - should end up as 3
 */


#include<stdio.h>
#include<stdlib.h>

/* registers and memory - represented by 32-bit int data type even though */
/*   most registers and memory have a 12-bit word size; note that the pc  */
/*   and mar hold 8-bit addresses only */

int halt     = 0, /* halt flag to halt the simulation */
    pc       = 0, /* program counter register, abbreviated as p */
    mar      = 0, /* memory address register, abbreviated as m */
    ram[256],     /* main memory to hold instructions and data */
    mdr      = 0, /* memory data register, abbreviated as d */
    acc      = 0, /* accumulator register, abbreviated as a */
    alu_tmp  = 0, /* called "ALU" register in the paper, abbreviated as u */
    b        = 0, /* b register to hold second operand for add/subtract */
    ir       = 0; /* instruction register, abbreviated as i */

int word_count; /* indicates how many memory words to display at end */


/* initialization routine to read in memory contents */

void load_ram(){
  int i = 0;
  FILE *fp;
  
  if( ( fp = fopen( "ram.txt", "r" ) ) == NULL ){
    printf( "error in opening ram file\n" );
    exit( -1 );
  }
  printf( "contents of RAM memory\n" );
  printf( "addr value\n" );
  while( fscanf( fp, "%x", &ram[i] ) != EOF ){
    if( i >= 256 ){
      printf( "ram.txt program file overflows available RAM\n" );
      exit( -1 );
    }
    ram[i] = ram[i] & 0xfff; /* clamp to 12-bit word size */
    printf( " %2x:  %03x\n", i, ram[i] );
    i++;
  }
  word_count = i;
  for( ; i < 256; i++ ){
    ram[i] = 0;
  }
  printf( "\n" );
  fclose( fp );
}


/* instruction fetch routine */

void fetch(){
  mar = pc;
  mdr = ram[ mar ];
  ir = mdr;
  pc++;
}


/* set of instruction execution routines - these use the step-by-step    */
/*   register transfers shown in the paper rather than single assignment */
/*   statements as would be typical for behavioral simulation */

void inv(){
  printf( "invalid opcode\n" );
  exit( -1 );
}

void lda(){
  mar = ir & 0xff; /* clamp to 8-bit address */
  mdr = ram[ mar ];
  acc = mdr;
}

void sta(){
  mar = ir & 0xff; /* clamp to 8-bit address */
  mdr = acc;
  ram[ mar ] = mdr;
}

void add(){
  alu_tmp = ( acc + b ) & 0xfff; /* clamp to 12-bit word size */
  acc = alu_tmp;
}

void sub(){
  alu_tmp = ( acc - b ) & 0xfff; /* clamp to 12-bit word size */
  acc = alu_tmp;
}

void mba(){
  b = acc;
}

void jmp(){
  pc = ir & 0xff; /* clamp to 8-bit address */
}

void jneg(){
  if( acc >> 11 ){
    pc = ir & 0xff; /* clamp to 8-bit address */
  }
}

void hlt(){
  halt = 1;
}


/* instruction decode routine - uses an array of function pointers with  */
/*   the opcode as the index into the array; an alternate approach is to */
/*   use a switch statement, perhaps in the main program itself with the */
/*   case labels and break statements bracketing instruction execution   */
/*   statements - thereby avoiding function calls */

void ( * fnp[8] )() = { inv, lda, sta, add, sub, mba, jmp, jneg };

void ( *decode() ) (){
  int opcode = ( ir >> 8 ) & 0xf; /* clamp to 4-bit opcode field */
  if( ( opcode >= 0 ) && ( opcode <= 7 ) ){
    return fnp[opcode];
  }else{
    return hlt;
  }
}


/* main program */

int main(){
  void ( *inst )();
  int i;

  printf( "\nbehaviorial simulation of Eckert's simple machine\n" );
  printf( "(all values are shown in hexadecimal)\n\n" );

  load_ram();

  printf( "intial register values\n" );
  printf( " pc mar mdr acc alu   b  ir\n" );
  printf( "%3x %3x %3x %3x %3x %3x %3x\n\n",
    pc, mar, mdr, acc, alu_tmp, b, ir );

  printf( "register values after each instruction has been executed\n" );
  printf( " pc mar mdr acc alu   b  ir\n" );

  while( !halt ){

    fetch();

    inst = decode();

    ( *inst )();

    printf( "%3x %3x %3x %3x %3x %3x %3x\n",
      pc, mar, mdr, acc, alu_tmp, b, ir );
  }

  printf( "\ncontents of RAM memory\n" );
  printf( "addr value\n" );
  for( i = 0; i < word_count; i++ ){
    printf( " %2x:  %03x\n", i, ram[i] );
  }
  printf( "\n" );

  return 0;
}
