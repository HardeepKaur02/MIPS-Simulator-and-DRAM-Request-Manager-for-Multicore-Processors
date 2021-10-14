/******************************************************************************

                              ASSIGNMENT 4

          The new code has been seperated from the previous code using comments.

*******************************************************************************/

#include <iostream>
#include<vector>
#include<bitset>
#include<string>
#include<map>
#include<algorithm>
#include<sstream>
#include<iomanip>
#include<fstream>
using namespace std;
#define word bitset<32>
#define MEMSIZE 1048576
///////////////////////////////////////////////////// from previous code ////////////////////////////////////////////////////////

//// throw 0 for wrong input format
//// throw 1 for invalid memory access/ memory exceed

int row_access_delay;
int col_access_delay;
int buffer_updates;
int cycle_num;
int programCounter;
string filename;

map<string, bitset<5> > reg_2_bit;

void regCodes_init(){

    reg_2_bit.insert(pair<string, bitset<5>>("$zero",bitset<5>(0)));
    reg_2_bit.insert(pair<string, bitset<5>>("$at",bitset<5>(1)));
    reg_2_bit.insert(pair<string, bitset<5>>("$v0",bitset<5>(2)));
    reg_2_bit.insert(pair<string, bitset<5>>("$v1",bitset<5>(3)));
    reg_2_bit.insert(pair<string, bitset<5>>("$a0",bitset<5>(4)));
    reg_2_bit.insert(pair<string, bitset<5>>("$a1",bitset<5>(5)));
    reg_2_bit.insert(pair<string, bitset<5>>("$a2",bitset<5>(6)));
    reg_2_bit.insert(pair<string, bitset<5>>("$a3",bitset<5>(7)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t0",bitset<5>(8)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t1",bitset<5>(9)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t2",bitset<5>(10)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t3",bitset<5>(11)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t4",bitset<5>(12)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t5",bitset<5>(13)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t6",bitset<5>(14)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t7",bitset<5>(15)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s0",bitset<5>(16)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s1",bitset<5>(17)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s2",bitset<5>(18)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s3",bitset<5>(19)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s4",bitset<5>(20)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s5",bitset<5>(21)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s6",bitset<5>(22)));
    reg_2_bit.insert(pair<string, bitset<5>>("$s7",bitset<5>(23)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t8",bitset<5>(24)));
    reg_2_bit.insert(pair<string, bitset<5>>("$t9",bitset<5>(25)));
    ////// registers 26-31 are reserved/ not to be used in this assignment, hence it is assumed that they won't be entered
}

bitset<5> regCode(string regName){       ///// gives 5 bit representation of register
	auto itr = reg_2_bit.find(regName);
  if(itr == reg_2_bit.end()){
    cout<<"Invalid register name "<<regName<<endl;
    throw 0;
  }
	return itr->second;
}

string bit_2_hex(bitset<32> in_word){   ///// converts 32 bit data to hexadecimal format
    int a = (int)(in_word.to_ulong());
    stringstream ss;
    ss << hex << setw(8) << setfill('0') <<a;
    string res = ss.str();
    string prefix = "0x";
    return prefix.append(res);
}

struct operands{
				  bitset<32> op1 ;
				  bitset<32> op2;
			  };

struct ALUresult{
	bitset<32> CalcResult;
	bitset<1> isZero;
};

class Instruction{
  public:
  int Inst_num;
  int operator_code;
  vector<string> operands;
  Instruction(){
      Inst_num=0;
      operator_code = 0;
  }
  Instruction(int num, int oper, vector<string> opnds){
      Inst_num= num;
      operator_code = oper;
      for(int i =0; i< opnds.size(); i++ ){
          operands.push_back(opnds.at(i));
      }
  }
};

struct parseResult{
	bool isInstruction;
	Instruction i;
  parseResult(bool is, Instruction k){
    isInstruction = is;
    i =k;
  }
};

int code(string op){
    if(op == "add"){
        return 1;
    }
    if(op == "sub"){
        return 2;
    }
    if(op == "mul"){
        return 3;
    }
    if(op == "addi"){
        return 4;
    }
    if(op == "beq"){
        return 5;
    }
    if(op == "bne"){
        return 6;
    }
    if(op == "slt"){
        return 7;
    }
    if(op == "lw"){
        return 8;
    }
    if(op == "sw"){
        return 9;
    }
    if(op == "j"){
        return 10;
    }
    return 11; /// label
}

string name(int op){
    if(op == 1){
        return "ADD";
    }
    if(op == 2){
        return "SUB";
    }
    if(op == 3){
        return "MUL";
    }
    if(op == 4){
        return "ADDI";
    }
    if(op == 5){
        return "BEQ";
    }
    if(op == 6){
        return "BNE";
    }
    if(op == 7){
        return "SLT";
    }
    if(op == 8){
        return "LW";
    }
    if(op == 9){
        return "SW";
    }
    if(op == 10){
        return "J";
    }
    return "LABEL"; /// label
}

vector<string> get(string s){
  vector<string> tokens;
    int l = s.length();
    int beg = 0 ;
    for (int i = 0; i < l; i++) {
        if (s[i] == ','){
            tokens.push_back(s.substr(beg,i-beg));
            beg = i+1;
          }
    }
    tokens.push_back(s.substr(beg,l-beg));
    return tokens;
}

parseResult parse(string line){

    vector<string> temp_tokens = get(line);
    vector<string> tokens;
    string token, operation;
    int op_code;
    for(int i = 0; i<temp_tokens.size();i++){
      istringstream my_stream(temp_tokens.at(i));
      if(i==0){
        my_stream >> operation;
        op_code = code(operation);
      }
      else{
        my_stream >> token;
        tokens.push_back(token);
      }
      if(i==0 and op_code!=11){
        if(my_stream >> token){
          tokens.push_back(token);
        }
        else{
          throw 0;
        }
      }
      if(my_stream >> token){
        throw 0;
      }
    }
    Instruction j;
    int exp_len;
    if(op_code<=7){
        exp_len = 3;
    }
    else if(op_code<=9){
        exp_len = 2;
    }
    else if(op_code == 10){
        exp_len = 1;
    }
    else{
        if(operation.at(operation.length()-1) != ':'){
          cout<<"Label should be followed by a :\n";
          throw 0;
        }
        operation = operation.substr(0,operation.length()-1);  //// removing : from label
        tokens.push_back(operation);
    }
    if(op_code!=11){
      if(tokens.size()!=exp_len){
        throw 0;
      }
      j = Instruction(0,op_code,tokens);
      parseResult p = parseResult(true,j);
      return p;
    }
    else{
        j = Instruction(0,0,tokens);
        parseResult p = parseResult(false,j);
        return p;
    }

}

bool is_imm(string str)
{
    int start = 0;
    if(str[0] == '-'){
        start = 1;
    }
    for(int j = start; j<str.length(); j++){
        if(!isdigit(str[j])){
            return false;
        }
    }
    return true;
}

class RF
{
  public:

      bitset<32> ReadData1, ReadData2;
     	RF() // Reading Register File for getting initial state of registers
    	{
          Registers.resize(32);
          Registers[0] = bitset<32> (0);  // Initializing Register 0 as "0"
  	  		ifstream registerfile;
          string line;
          int i=0;
          registerfile.open("RF_init.txt");
          cout<<"Started reading register file initial state\n";
          if (registerfile.is_open())
          {
          while (getline(registerfile,line))
               {
                 int int_line = stoi(line);
                 Registers[i] = bitset<32>(int_line);
                 i++;
               }
          }
          else cout<<"Unable to open file\n";
          registerfile.close();
        }

      operands ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
      {

  		   int a, b, c , to_write;
  	     operands op;

  		   a = (int)(RdReg1.to_ulong());
  		   b = (int)(RdReg2.to_ulong());
  		   c = (int)(WrtReg.to_ulong());

  		   op.op1 = Registers[a] ;
  		   op.op2= Registers[b] ;

         to_write = (int)(WrtEnable.to_ulong());
         if (to_write){
           Registers[c] = WrtData;
         }
  		   return op;
		  }

      void OutputRF()
      {
       ofstream rfout;
        rfout.open("RF_final.txt",std::ios_base::app);
        if (rfout.is_open())
        {
          cout<<"Writing register state to RF_final.txt\n";
          rfout<<"State of RF after cycle : "<<cycle_num<<endl;
        for (int j = 0; j<32; j++)
            {
              rfout<<bit_2_hex(Registers[j])<<endl;
            }

        }
        else cout<<"Unable to open file\n";
        rfout.close();

       }

       void loadWord(int reg_no, word word_to_store){
      	   Registers[reg_no] = word_to_store;
       }

       word storeWord(int reg_no){
           return Registers[reg_no];
       }

       private:
          vector<bitset<32> >Registers;

};

class Memory
{
  vector<vector<word>> DRAM;
  vector<word> row_buffer;

public:
  bool is_active;
  int active_row;
  bool writeback_pending;
	vector<Instruction> insMem;
	map<string, int> labels;

  Memory(){
      for(int i=0 ; i<256 ; i++){
         DRAM.push_back(vector<word>(256, word(0)));
      }
      writeDRAM();
      is_active = false;
      active_row = -1;
      writeback_pending = false;
  }

  void writeDRAM(){
      cout<<"Reading data from dataMem.txt\n";
      ifstream mem_info;
      string line;
      mem_info.open("dataMem.txt");
      if(mem_info.is_open())
      {
        while(getline(mem_info,line))
        {
          istringstream my_stream(line);
          int z1; ////////////////////store the memory address here
          int z2; ///////////////////// store the .word value  here
          my_stream >> z1;
          my_stream >> z2;
          z1 = z1/4; //////////// byte to word
          int r_num = z1/256;
          int c_num = z1%256;
          DRAM[r_num][c_num] = word(z2);
        }
        cout<<"Data read successfully\n";
      }
      else{
        cout<<"Unable to open file\n";
      }
      mem_info.close();
    }

  void activate(int row_num){
      buffer_updates++;
      is_active = true;
      active_row = row_num;
      row_buffer = DRAM[row_num];
      cout<<"Row bufffer updated\n";
  }

  void writeback(int row_num){
      DRAM[row_num] = row_buffer;
      row_buffer.clear();
      is_active = false;
      active_row = -1;
      if(writeback_pending){
        printDataMem();
      }
  }

  word col_req1(int col_num){     ///load word
      word temp = row_buffer[col_num];
      return temp;
  }

  void col_req2(int row_num, int col_num, word store_it){  // store word
    row_buffer[col_num] = store_it;
    buffer_updates++;
    cout<<"Row bufffer updated\n";
  }

  void printDataMem(){                       /////////////// writes the state of DRAM to a file dataMem_final.txt
    cout<<"Writing data of memory to dataMem_final.txt\n";
    ofstream rfout;
    rfout.open("dataMem_final.txt",std::ios_base::app);
    if (rfout.is_open())
    {
      rfout<<"Data stored in DRAM after cycle :"<<cycle_num<<endl;
      for (int i = 0 ; i<256 ; i++)
      {
        for(int j = 0; j<256; j++)
        {
          if(DRAM[i][j]!=0 ){
            int location = (256*i + j)*4;
            if(location >= 1000){
                rfout<<location<<" : "<<(int)(DRAM[i][j]).to_ulong()<<endl;
            }

          }
        }
      }
    }
    else cout<<"Unable to open file\n";
    rfout.close();

  }

  void writeInsMem(){
    cout<<"Storing instructions in Instruction Memory\n";
		ifstream infile;
		string line;
		infile.open(filename);
		if(infile.is_open()){
			int num_inst = 0;
			while(getline(infile >> std::ws,line)){
        try{
  				parseResult p = parse(line);
  				if(p.isInstruction){
  					p.i.Inst_num = num_inst;
  					insMem.push_back(p.i);
            int DRAM_row = num_inst/256;
            int DRAM_col = num_inst%256;
            DRAM[DRAM_row][DRAM_col] = num_inst;
  					num_inst++;
  				}
  				else{
  					string label1 = p.i.operands.at(0);
  					labels.insert(pair<string,int>(label1,num_inst));
  				}
        }
        catch(int x){
          cout<<"Wrong format of input received.\n";
          throw 0;
        }
			}
      cout<<"Instructions stored successfully\n";
		}
		else{
			cout<<"Unable to open file\n";
      throw 1;
		}
	}

  void showInsMem(){
    for(int p = 0; p< insMem.size();p++){
      Instruction temp = insMem.at(p);
      cout<<"op_code : "<<temp.operator_code<<endl;
      for(auto itr = temp.operands.begin(); itr!= temp.operands.end(); itr++){
        cout<<*itr<<" ";
      }
      cout<<endl;
    }
  }

	void ReadWrite(string Address,bitset<5> ReadReg, bitset<5> WrtReg, bitset<1> ReadEnable, bitset<1> WrtEnable, RF& regFile){
		//////////// ReadReg : register to which read value is stored
		//////////// WrtReg : register whose data is copied to memory
		int to_write = (int)(WrtEnable.to_ulong());
		int to_read = (int)(ReadEnable.to_ulong());

    int addr = (stoi(Address))/4;
    int row_num = addr/256;
    int col_num = addr%256;
    if(!is_active){
      activate(row_num);
    }
    else if(is_active and active_row != row_num){
      writeback(active_row);
      writeback_pending = false;
      activate(row_num);
    }

		if(to_read)
		{
			int Rdest = (int)(ReadReg.to_ulong());
      word word_to_store = col_req1(col_num);
      regFile.loadWord(Rdest, word_to_store);

		}
		else if(to_write)
		{
			int RSrc = (int)(WrtReg.to_ulong());
			word store_it = regFile.storeWord(RSrc);
      col_req2(row_num, col_num, store_it);
      writeback_pending = true;
		}
	}
};

class ALU
{
      public:
             ALUresult result;
             ALUresult ALUOperation (bitset<4> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
             {
        				 int operand1 = (int)(oprand1.to_ulong());
        				 /*cout << "operand  1 is " ;
        				 cout << operand1<<endl;*/

        				 int operand2 = (int)(oprand2.to_ulong());
        				 /*cout << "operand  2 is ";
        				 cout << operand2<<endl;*/
        				 int Aluoper = (int)(ALUOP.to_ulong());
        				 int intResult;
        				 switch (Aluoper){
        					 // R Type Operations    add,sub,mul,addi
        					 case 1:
									 			 intResult = operand1 + operand2;
        							   result.CalcResult = bitset<32>(intResult);
												 if(intResult==0){
													 result.isZero[0] = 1;
												 }
												 else{
													 result.isZero[0] = 0;
												 }

        							   /*cout << "ALU OUTPUT : ";
        							   cout << intResult<<endl;*/
        							   break;
        				   case 2:
									       intResult = operand1 - operand2;
        							   result.CalcResult = bitset<32>(intResult);
												 if(intResult==0){
													 result.isZero[0] = 1;
												 }
												 else{
													 result.isZero[0] = 0;
												 }
        							   /*cout << "ALU OUTPUT : ";
        							   cout << intResult<<endl;*/
        							   break;
        					 case 3:
        							   intResult = operand1*operand2 ;
												 result.CalcResult = bitset<32>(intResult);
												 if(intResult==0){
													 result.isZero[0] = 1;
												 }
												 else{
													 result.isZero[0] = 0;
												 }
        							   /*cout << "ALU OUTPUT : ";
        							   cout << intResult<<endl;*/
        							   break;
								  case 7:
												intResult = operand1<operand2 ? 1 : 0 ;
												result.CalcResult = bitset<32>(intResult);
												if(intResult==0){
													result.isZero[0] = 1;
												}
												else{
													result.isZero[0] = 0;
												}
												/*cout << "ALU OUTPUT : ";
												cout << intResult<<endl;*/
												break;

        				 }
                         return result;
               }
};

//////////////////////////////////////////////// till here from previous code /////////////////////////////////////////////////////
//////////////////////////////////// Following is the code modified or added for assignment 4 ////////////////////////////////////////////////

Memory my_mem;
ALU my_ALU;
RF RegFile;

bool DRAM_on, DRAM_alone;
vector<int> cycles;

class DRAM_req{
  public:
  Instruction ins;
  int w_cycle, a_cycle, c_cycle;
  string Address;
  int id,status,to_reg,from_reg,row_num;
  DRAM_req(){
      id = -1;
  }
  DRAM_req(Instruction i, string my_addr, int my_id, int to_reg1, int from_reg1){
      ins = i;
      Address = my_addr;
      id = my_id;
      to_reg = to_reg1;
      from_reg = from_reg1;
      row_num = stoi(Address)/1024;
      status = -1;
      w_cycle = -1;
      a_cycle = -1;
      c_cycle = -1;
  }
};

DRAM_req DRAM_curr_req;
vector<DRAM_req> DRAM_queue;
vector<int> helper;

int action(){
  if(cycle_num == DRAM_curr_req.a_cycle){
      cout<<"DRAM REQUEST : ACTIVATE ROW NUM  "<<DRAM_curr_req.row_num<<endl;
      my_mem.activate(DRAM_curr_req.row_num);
      return 1;
  }
  if(cycle_num == DRAM_curr_req.w_cycle){
      cout<<"DRAM REQUEST : WRITEBACK ROW NUM  "<<my_mem.active_row<<endl;
      my_mem.writeback(my_mem.active_row);
      return 1;
  }
  if(cycle_num == DRAM_curr_req.c_cycle){
      cout<<"DRAM REQUEST : COL ACCESS"<<endl;
      if(DRAM_curr_req.id == 0) //lw
      {
          my_mem.ReadWrite(DRAM_curr_req.Address,bitset<5>(DRAM_curr_req.to_reg), bitset<5>(0), bitset<1>(1), bitset<1>(0),RegFile );
      }
      else{       //sw
          my_mem.ReadWrite(DRAM_curr_req.Address,bitset<5>(0),bitset<5>(DRAM_curr_req.from_reg), bitset<1>(0), bitset<1>(1),RegFile );
      }
      if(DRAM_queue.size() > 0){

          vector<DRAM_req> :: iterator it;
          vector<int> :: iterator it1;
          it1 = std::find(helper.begin(),helper.end(),my_mem.active_row);
          int posn = 0;
          if(it1!= helper.end()){
            posn = it1 - helper.begin();
          }
          else{
            it1 = helper.begin();
          }
          it = DRAM_queue.begin();
          advance(it,posn);
          DRAM_curr_req = DRAM_queue[posn];
          DRAM_queue.erase(it);
          helper.erase(it1);
          DRAM_curr_req.status = 1;
          if(!my_mem.is_active){
            DRAM_curr_req.a_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + row_access_delay + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
          }
          else if(my_mem.active_row != DRAM_curr_req.row_num){
            DRAM_curr_req.w_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.a_cycle = cycle_num + 2*row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + 2*row_access_delay + col_access_delay;
          }
          else{
            DRAM_curr_req.c_cycle = cycle_num + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
            DRAM_curr_req.a_cycle = -1;
          }
      }
      else{
          DRAM_on = false;
          DRAM_alone = false;
      }
      return 1;
  }
  return 0;
}

bitset<5> getOffset(int& offset,string Address){
  string offs = "";
  int i;
  for(i = 0; i<Address.length() ;i++){
    if(Address[i]!='('){
      offs+=Address[i];
    }
    else{
      break;
    }
  }
  if(i == Address.length()){
    cout<<"Wrong format of memory address received\n";
    throw 0;
  }

  if(offs == ""){
    offset = 0;
  }
  else{
    offset = stoi(offs);
  }
  i+=1;
  while(Address[i] == ' '){
      i++;
  }
  string reg_name = "";
  for(int j = i; j<Address.length(); j++){
      if(Address[j]==')' or Address[j] == ' '){
        return regCode(reg_name);
      }
      else{
          reg_name+=Address[j];
      }
  }
  ///////////////////////////////////// throw error///////////////////////
}

bool independent(DRAM_req req1, DRAM_req req2){
    if(req1.status == 1){ ///// req1 is DRAM_curr_req
      return true;
    }
    else{  ///// req1 lies in DRAM_queue

        if(req1.id ==1 && req2.id ==1)   ////// sw-sw
        {
            return true;
            //return (!(req1.Address == req2.Address));
        }
        if(req2.id == 1 ){              ////// lw-sw
            if(req1.to_reg == req2.to_reg || req1.to_reg == req2.from_reg ){
                return false;
            }
            return true;
        }
        if(req1.id == 1)              //////  sw-lw
        {
            if(req1.to_reg == req2.to_reg || req1.from_reg == req2.to_reg ){
                return false;
            }
            return true;
        }
        else{                        ////// lw-lw
            if(req1.to_reg == req2.to_reg || req1.to_reg == req2.from_reg || req1.from_reg == req2.to_reg){
                return false;
            }
            return true;
        }
    }
}

bool independent(DRAM_req request, Instruction curr_inst){

    int op_code = curr_inst.operator_code;

    if(op_code == 10){
        return true;
    }
    int t, f, to_reg, from1, from2;
    if(op_code < 4 || op_code == 7){  //// add,sub,mul,slt
        t = 1;
        to_reg = (int)regCode(curr_inst.operands[0]).to_ulong();
        f = 2;
        from1 = (int)((regCode(curr_inst.operands[1])).to_ulong());
        from2 = (int)((regCode(curr_inst.operands[2])).to_ulong());
    }
    if(op_code == 4){    //// addi
        t = 1;
        to_reg = (int)regCode(curr_inst.operands[0]).to_ulong();
        f =1;
        from1 = (int)((regCode(curr_inst.operands[1])).to_ulong());
    }
    if(op_code == 5 || op_code == 6){   //// beq, bne
        t = 0;
        f = 2;
        from1 = (int)((regCode(curr_inst.operands[0])).to_ulong());
        from2 = (int)((regCode(curr_inst.operands[1])).to_ulong());
    }
    if(op_code == 8 || op_code == 9){
        ///////////// code for lw, sw ////////////////
        vector<string> temp = curr_inst.operands;
        bitset<5> ReadReg = regCode(temp.at(0));
        string Address = temp.at(1);
        int offset;
        bitset<5> reg_num = getOffset(offset,Address);
        operands ops = RegFile.ReadWrite(reg_num, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
        word baseAddr = ops.op1;
        ALUresult Addr = my_ALU.ALUOperation(bitset<4>(1), bitset<32>(offset),baseAddr);
        Address = to_string((int)((Addr.CalcResult).to_ulong()));
        DRAM_req curr_req;
        if(op_code == 8){
            curr_req = DRAM_req(curr_inst,Address, 0 , (int)ReadReg.to_ulong(), (int)reg_num.to_ulong());
        }
        if(op_code == 9){
            curr_req = DRAM_req(curr_inst,Address, 1 , (int)reg_num.to_ulong(), (int)ReadReg.to_ulong());
        }
        return independent(request, curr_req);

    }
    if(request.status == 1){   //// DRAM_curr_req
        if(request.id == 0){   //// lw
            if(t == 1 && request.to_reg == to_reg){  //// to-to conflict
                return false;
            }
            if(request.to_reg == from1){     ///// to (lw) - from (ins) conflict
                return false;
            }
            if(f ==2 && from2 == request.to_reg){
                return false;
            }
        }
        else{ ////  DRAM_curr_req is sw
            if(t ==1 && request.from_reg == to_reg){
                return false;
            }
        }
    }
    else{ //// any DRAM_req in DRAM_queue
        if(request.id == 0){  ///// lw
            if(t == 1 && to_reg == request.to_reg){  //// to-to conflict
                return false;
            }
            if(from1 == request.to_reg){
                return false;
            }
            if(f == 2 && from2 == request.to_reg){
                return false;
            }
        }
        else{  ///// sw
            if(t == 1 && ( to_reg == request.from_reg)){
                return false;
            }
        }

    }
}

bool independent(Instruction curr_inst){

    if(!DRAM_on && (DRAM_queue.size() == 0)){
        return true;
    }
    bool ind = true;
    if(independent(DRAM_curr_req, curr_inst)){        //////// this if is not needed

        for(int i = 0; i< DRAM_queue.size(); i++){
            if(!independent(DRAM_queue[i], curr_inst)){
                ind = false;
                break;
            }
        }
    }
    else{
        ind = false;
    }
    return ind;
}

void execute(Instruction curr_inst,ALU& my_ALU, Memory& my_mem, RF& RegFile){
  int operator_temp = curr_inst.operator_code;
  if(operator_temp<4){            ///// add,sub, mul
    vector<string> temp = curr_inst.operands;
    if(temp[0] == "$zero"){
      cout<<"WARNING : Instruction involves modifying register $zero\n";
    }
    bitset<5> Rdest = regCode(temp.at(0));
    bitset<5> Rsrc1 = regCode(temp.at(1));
    string Src2 = temp.at(2);
    word op1,op2;
    if(is_imm(Src2)){
      operands ops = RegFile.ReadWrite(Rsrc1, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = word(stoi(Src2));
    }
    else{
      bitset<5> Rsrc2 = regCode(Src2);
      operands ops = RegFile.ReadWrite(Rsrc1,Rsrc2, bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = ops.op2;
    }
    ALUresult res = my_ALU.ALUOperation(bitset<4>(operator_temp), op1, op2);
    RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
    programCounter++;
    RegFile.OutputRF();
  }
  else if(operator_temp==4){      ///// addi
    vector<string> temp = curr_inst.operands;
    if(temp[0] == "$zero"){
      cout<<"WARNING : Instruction involves modifying register $zero\n";
    }
    bitset<5> Rdest = regCode(temp.at(0));
    bitset<5> Rsrc = regCode(temp.at(1));
    operands ops = RegFile.ReadWrite(Rsrc, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
    word op1 = ops.op1;
    word op2 = word(stoi(temp.at(2)));
    ALUresult res = my_ALU.ALUOperation(bitset<4>(1), op1, op2);    //////////// send the instruction to add using opcode = 1
    RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
    programCounter++;
    RegFile.OutputRF();
  }
  else if(operator_temp==5){      ///// beq
    vector<string> temp = curr_inst.operands;
    bitset<5> Rsrc1 = regCode(temp.at(0));
    string Src2 = temp.at(1);
    word op1,op2;
    if(is_imm(Src2)){
      operands ops = RegFile.ReadWrite(Rsrc1, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = word(stoi(Src2));
    }
    else{
      bitset<5> Rsrc2 = regCode(Src2);
      operands ops = RegFile.ReadWrite(Rsrc1,Rsrc2, bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = ops.op2;
    }
    string go_to = temp.at(2);
    ALUresult res = my_ALU.ALUOperation(bitset<4>(2), op1, op2);    //////////// send the instruction to sub using opcode = 2
    cout<<"Comparison performed in ALU for beq .\n";
    if(res.isZero.any()){
      auto pc  = my_mem.labels.find(go_to);
      if(pc == my_mem.labels.end()){
        cout<<"Label "<<go_to<<" was not declared\n";
        throw 0;
      }
      programCounter = pc->second ;
    }
    else{
      programCounter++;
    }
    RegFile.OutputRF();
  }
  else if(operator_temp==6){      ///// bne
    vector<string> temp = curr_inst.operands;
    bitset<5> Rsrc1 = regCode(temp.at(0));
    string Src2 = temp.at(1);
    word op1,op2;
    if(is_imm(Src2)){
      operands ops = RegFile.ReadWrite(Rsrc1, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = word(stoi(Src2));
    }
    else{
      bitset<5> Rsrc2 = regCode(Src2);
      operands ops = RegFile.ReadWrite(Rsrc1,Rsrc2, bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = ops.op2;
    }
    string go_to = temp.at(2);
    ALUresult res = my_ALU.ALUOperation(bitset<4>(2), op1, op2);    //////////// send the instruction to sub using opcode = 2
    if(!res.isZero.any()){
      auto pc  = my_mem.labels.find(go_to);
      if(pc == my_mem.labels.end()){
        cout<<"Label "<<go_to<<" was not declared\n";
        throw 0;
      }
      programCounter = pc->second ;
    }
    else{
      programCounter++;
    }
    RegFile.OutputRF();
  }
  else if(operator_temp==7){      ///// slt
    vector<string> temp = curr_inst.operands;
    if(temp[0] == "$zero"){
      cout<<"WARNING : Instruction involves modifying register $zero\n";
    }
    bitset<5> Rdest = regCode(temp.at(0));
    bitset<5> Rsrc1 = regCode(temp.at(1));
    string Src2 = temp.at(2);
    word op1,op2;
    if(is_imm(Src2)){
      operands ops = RegFile.ReadWrite(Rsrc1, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = word(stoi(Src2));
    }
    else{
      bitset<5> Rsrc2 = regCode(Src2);
      operands ops = RegFile.ReadWrite(Rsrc1,Rsrc2, bitset<5>(0), word(0), bitset<1>(0));
      op1 = ops.op1;
      op2 = ops.op2;
    }
    ALUresult res = my_ALU.ALUOperation(bitset<4>(7), op1, op2);
    cout<<"Comparison performed in ALU for slt .\n";
    RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
    programCounter++;
    RegFile.OutputRF();
  }
  else if(operator_temp==8){      ///// lw
    vector<string> temp = curr_inst.operands;
    if(temp[0] == "$zero"){
      cout<<"WARNING : Instruction involves modifying register $zero\n";
    }
    bitset<5> ReadReg = regCode(temp.at(0));
    string Address = temp.at(1);
    int offset;
    bitset<5> reg_num = getOffset(offset,Address);
    operands ops = RegFile.ReadWrite(reg_num, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
    word baseAddr = ops.op1;
    ALUresult Addr = my_ALU.ALUOperation(bitset<4>(1), bitset<32>(offset),baseAddr);
    int int_addr = (int)((Addr.CalcResult).to_ulong());
    if(int_addr >= MEMSIZE){
      cout<<"Memory Limit Exceeded\n";
      throw 1;
    }
    Address = to_string(int_addr);
    DRAM_req curr_req(curr_inst,Address, 0 , (int)ReadReg.to_ulong(), (int)reg_num.to_ulong());
    if(DRAM_on){
      curr_req.status = 0;
      DRAM_queue.push_back(curr_req);
      helper.push_back(curr_req.row_num);
    }
    else{
        DRAM_on = true;
        DRAM_curr_req = curr_req;
        DRAM_curr_req.status = 1;
        if(!my_mem.is_active){
            DRAM_curr_req.a_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + row_access_delay + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
        }
        else if(my_mem.active_row != DRAM_curr_req.row_num){
            DRAM_curr_req.w_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.a_cycle = cycle_num + 2*row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + 2*row_access_delay + col_access_delay;
        }
        else{
            DRAM_curr_req.c_cycle = cycle_num + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
            DRAM_curr_req.a_cycle = -1;
        }
    }
    programCounter++;
    RegFile.OutputRF();
  }
  else if(operator_temp==9){      ///// sw
    vector<string> temp = curr_inst.operands;
    bitset<5> WrtReg = regCode(temp.at(0));
    string Address = temp.at(1);
    int offset;
    bitset<5> reg_num = getOffset(offset,Address);
    operands ops = RegFile.ReadWrite(reg_num, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
    word baseAddr = ops.op1;
    ALUresult Addr = my_ALU.ALUOperation(bitset<4>(1), bitset<32>(offset),baseAddr);
    int int_addr = (int)((Addr.CalcResult).to_ulong());
    if(int_addr >= MEMSIZE){
      cout<<int_addr<<endl;
      cout<<"Memory Limit Exceeded\n";
      throw 1;
    }
    Address = to_string(int_addr);
    DRAM_req curr_req(curr_inst,Address, 1 , (int)reg_num.to_ulong(), (int)WrtReg.to_ulong());
    if(DRAM_on){
      DRAM_queue.push_back(curr_req);
      helper.push_back(curr_req.row_num);
    }
    else{
        DRAM_on = true;
        DRAM_curr_req = curr_req;
        DRAM_curr_req.status = 1;
        if(!my_mem.is_active){
            DRAM_curr_req.a_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + row_access_delay + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
        }
        else if(my_mem.active_row != DRAM_curr_req.row_num){
            DRAM_curr_req.w_cycle = cycle_num + row_access_delay;
            DRAM_curr_req.a_cycle = cycle_num + 2*row_access_delay;
            DRAM_curr_req.c_cycle = cycle_num + 2*row_access_delay + col_access_delay;
        }
        else{
            DRAM_curr_req.c_cycle = cycle_num + col_access_delay;
            DRAM_curr_req.w_cycle = -1;
            DRAM_curr_req.a_cycle = -1;
        }
    }
    programCounter++;
    RegFile.OutputRF();
  }
  else if(operator_temp==10){     ///// j
    string go_to = curr_inst.operands.at(0);
    auto pc = my_mem.labels.find(go_to);
    programCounter = pc->second ;
    RegFile.OutputRF();
  }
}

void interpret(ALU& my_ALU, Memory& my_mem, RF& RegFile, vector<int>& ins_counts){
	programCounter = 0;
    DRAM_on = false;
    DRAM_alone = false;
    cycle_num = 1;
	while(true){
	    if(programCounter >= my_mem.insMem.size() && DRAM_queue.size()==0 && !DRAM_on){
	        break;
	    }
      if(programCounter >= my_mem.insMem.size()){///////////////////////////////////////////////////////////////////////////////////////
        DRAM_alone = true;
      }
	    RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),bitset<5>(0), bitset<32>(0), bitset<1>(1) );
	    if(!DRAM_alone){
	        Instruction curr_ins = my_mem.insMem.at(programCounter);
	        int operator_temp = curr_ins.operator_code;
	        if(DRAM_on){
	            if(independent(curr_ins)){
                  cout<<"CYCLE NUM : "<<cycle_num<<endl;
                  cout<<"INSTRUCTION : "<<name(operator_temp)<<" and DRAM \n";
	                execute(curr_ins, my_ALU, my_mem, RegFile);
	                ins_counts.at(operator_temp-1)++;       ///// since operator_code start from 1
	                cycles.push_back(operator_temp);

	            }
	            else{
	                DRAM_alone = true;
	                cout<<"CYCLE NUM : "<<cycle_num<<endl;
	                cout<<"INSTRUCTION : DRAM "<<endl;
	                cycles.push_back(11);
	            }
	        }
	        else{
              cout<<"CYCLE NUM : "<<cycle_num<<endl;
              cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
	            execute(curr_ins, my_ALU, my_mem, RegFile);
	            ins_counts.at(operator_temp-1)++;       ///// since operator_code start from 1
	            cycles.push_back(operator_temp);

	        }

	    }
      else{
        cout<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM REQUEST\n";
      }
      action();
	    cycle_num++;

	}
}

int main(int argc, char** argv){
  if(argc<3){
    cout<<"Kindly run the executable using row_access_delay and col_access_delay as command line arguments.\n"<<argc;
    return -1;
  }
  row_access_delay = atoi(argv[1]);
  col_access_delay = atoi(argv[2]);
  if(row_access_delay == 0 ){
    cout<<"Kindly enter a non zero integer as row access delay\n";
    return -1;
  }
  if(col_access_delay == 0){
    cout<<"Kindly enter a non zero integer as column access delay\n";
    return -1;
  }
  buffer_updates = 0;
  if(argc == 4){
    filename = (string)argv[3] + ".txt";
  }
  else{
    filename = "input.txt";
  }
  remove("RF_final.txt");          //// deletes the files if present initially
  remove("dataMem_final.txt");
  regCodes_init();
  my_mem.is_active = false;
  try{
    my_mem.writeInsMem();
  }
  catch(int e){
    if(e==0){
      cout<<"PROGRAM TERMINATED DUE TO WRONG INPUT FORMAT\n";
      return -1;
    }
    if(e==1){
      cout<<"Kindly ensure that the file \""<<filename<<"\" is present in the same directory.\n";
      return -1;
    }
  }

  //my_mem.showInsMem();
  vector<string> operations = {"add", "sub", "mul", "addi", "beq" , "bne", "slt", "lw", "sw", "j"};
  vector<int> ins_counts = {0,0,0,0,0,0,0,0,0,0};
  try{
      interpret(my_ALU, my_mem, RegFile,ins_counts);
  }
  catch(std::invalid_argument& e){
  // if no conversion could be performed
  cout<<"PROGRAM TERMINATED : WRONG INPUT FORMAT - CONVERSION COULDN'T BE PERFORMED\n";
  return -1;
  }
  catch(std::out_of_range& e){
  // if the converted value would fall out of the range of the result type
  // or if the underlying function (std::strtol or std::strtoull) sets errno
  // to ERANGE.
  cout<<"PROGRAM TERMINATED: DATA OUT OF RANGE\n";
  return -1;
  }
  catch(int e){
    if(e==0){
      cout<<"PROGRAM TERMINATED DUE TO WRONG INPUT FORMAT\n";
      return -1;
    }
    if(e==1){
      cout<<"PROGRAM TERMINATED DUE TO MEMORY LIMIT EXCEED\n";
      return -1;
    }
  }
  cycle_num--;   // it was incremented before exitting
  if(my_mem.writeback_pending){
    cycle_num++;
    for(int i = 0; i < row_access_delay - 1; i++){
      cout<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : DRAM REQUEST\n";
      cycle_num++;
    }
    cout<<"CYCLE NUM : "<<cycle_num<<endl;
    cout<<"INSTRUCTION : FINAL WRITEBACK - ROW "<<my_mem.active_row<<endl;
    my_mem.writeback(my_mem.active_row);
  }
  cout<<endl<<"STATISTICS"<<endl;
  cout<<"No. of clock cycles : "<<cycle_num<<endl;
  cout<<"No. of buffer updates : "<<buffer_updates<<endl;
  for(int k = 0; k<10; k++){
    int num = ins_counts.at(k);
    if(num >0){
      cout<<"No. of "<<operations.at(k)<<" instructions : "<<num<<endl;
    }
  }
  return 0;
}
