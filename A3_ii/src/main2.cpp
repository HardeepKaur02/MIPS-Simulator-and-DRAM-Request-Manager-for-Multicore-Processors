#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include<algorithm>
#include<map>
#include<sstream>
#include<iomanip>
using namespace std;

#define MemSize 262144 // word addressable memory
#define word bitset<32>

/////////////////////////////////////////////////////////////////     MINOR EXAM  : PART 2  //////////////////////////////////////////////////////////////////////////////

int row_access_delay;
int col_access_delay;
int cycle_num;
int buffer_updates;

//////////////// map from register names to register numbers ///////////////////////
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

class cycle_ins{
public:
  int ins_num;
  string to;
  string from;
  cycle_ins(int inst_num){
    ins_num = inst_num;
  }
  cycle_ins(int inst_num, string to_location, string from_location){
    ins_num = inst_num;
    to = to_location;
    from = from_location;
  }
  void print(){
    cout<<"Instruction num : "<<ins_num<<endl;
  }
};

vector<vector<cycle_ins>> cycles;

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
  bool is_active;
  vector<word> row_buffer;

public:
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
      cout<<"Row "<<row_num<<" would be activated\n";
  }
  void writeback(int row_num, int on_cycle){
      DRAM[row_num] = row_buffer;
      row_buffer.clear();
      is_active = false;
      cout<<"Buffer Row "<<row_num<<" would be written back\n";
      if(writeback_pending){
          printDataMem(on_cycle);
      }
  }
  word col_req1(int col_num){     ///load word
      word temp = row_buffer[col_num];
      return temp;
  }
  void col_req2(int row_num, int col_num, word store_it){
    row_buffer[col_num] = store_it;
    buffer_updates++;
    cout<<"Row bufffer would be updated\n";
  }

  void printDataMem(int on_cycle){                       /////////////// writes the state of DRAM to a file dataMem_final.txt
    cout<<"Data of memory would be written to dataMem_final.txt\n";
    ofstream rfout;
    rfout.open("dataMem_final.txt",std::ios_base::app);
    if (rfout.is_open())
    {
      rfout<<"Data stored in DRAM after cycle "<<on_cycle<<" : "<<endl;
      for (int i = 0 ; i<256 ; i++)
      {
        for(int j = 0; j<256; j++)
        {
          if(DRAM[i][j]!=0){
            int location = (256*i + j)*4;
            rfout<<location<<" : "<<(int)(DRAM[i][j]).to_ulong()<<endl;
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
		infile.open("input.txt");
		if(infile.is_open()){
			int num_inst = 0;
			while(getline(infile >> std::ws,line)){
        try{
  				parseResult p = parse(line);
  				if(p.isInstruction){
  					p.i.Inst_num = num_inst;
  					insMem.push_back(p.i);
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

	int ReadWrite(string Address,bitset<5> ReadReg, bitset<5> WrtReg, bitset<1> ReadEnable, bitset<1> WrtEnable, RF& regFile)
	{
		//////////// ReadReg : register to which read value is stored
		//////////// WrtReg : register whose data is copied to memory
		int to_write = (int)(WrtEnable.to_ulong());
		int to_read = (int)(ReadEnable.to_ulong());

    int addr = (stoi(Address))/4;
    int row_num = addr/256;
    int col_num = addr%256;

    int total_cycles = 0;

    if(!is_active){
      activate(row_num);
      total_cycles += row_access_delay;
    }
    else if(is_active and active_row != row_num){
      writeback(active_row, cycle_num+ row_access_delay);
      writeback_pending = false;
      activate(row_num);
      total_cycles += (2*row_access_delay);
    }

		if(to_read)
		{
			int Rdest = (int)(ReadReg.to_ulong());
      word word_to_store = col_req1(col_num);
      total_cycles += col_access_delay;
      regFile.loadWord(Rdest, word_to_store);

		}
		else if(to_write)
		{
			int RSrc = (int)(WrtReg.to_ulong());
			word store_it = regFile.storeWord(RSrc);
      col_req2(row_num, col_num, store_it);
      total_cycles += col_access_delay;
      writeback_pending = true;
		}
    return total_cycles;
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

void simulate(ALU& my_ALU, Memory& my_mem, RF& RegFile, vector<int>& ins_counts){
	int programCounter = 0;
	while(programCounter < my_mem.insMem.size()){
		Instruction curr_inst = my_mem.insMem.at(programCounter);
    int ins_num = curr_inst.Inst_num;
		int operator_temp = curr_inst.operator_code;
    ins_counts.at(operator_temp-1)++;       ///// since operator_code start from 1
		if(operator_temp<4){      ////////// corresponds to add,sub, mul
			vector<string> temp = curr_inst.operands;
      bitset<5> Rdest = regCode(temp.at(0));
			bitset<5> Rsrc1 = regCode(temp.at(1));
      string Src2 = temp.at(2);
      bool imm_val = is_imm(Src2);
			word op1,op2;
			if(imm_val){
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

      cycle_ins ins = cycle_ins(ins_num);

      if(cycle_num < cycles.size()){
        cycle_ins dram_ins = cycles[cycle_num][0];
        bool seperate = false;
        bool to_to_conflict = (temp[0] == dram_ins.to);
        bool to_from_conflict = (temp[0] == dram_ins.from);
        bool from_to_conflict = (temp[1] == dram_ins.to);
        if(to_to_conflict or to_from_conflict or from_to_conflict){
          seperate = true;
        }
        if(!imm_val and (temp[2] == dram_ins.to)){
          seperate = true;
        }
        if(seperate){
          while(cycle_num < cycles.size()){
            cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
            cout<<"INSTRUCTION : DRAM request\n";
            if(cycle_num == cycles.size()-1){
              RegFile.OutputRF();
            }
            cycle_num++;
          }
          cycles.push_back({ins});
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(operator_temp), op1, op2);
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
        else{
          cycles[cycle_num].push_back(ins);
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION : DRAM ACCESS and "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(operator_temp), op1, op2);
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
      }
      else{
        cycles.push_back({ins});
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
        ALUresult res = my_ALU.ALUOperation(bitset<4>(operator_temp), op1, op2);
  			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
        RegFile.OutputRF();
        cycle_num++;
      }
      programCounter++;
    }
		else if(operator_temp==4){     ///////// corresonds to addi
			vector<string> temp = curr_inst.operands;
			bitset<5> Rdest = regCode(temp.at(0));
			bitset<5> Rsrc = regCode(temp.at(1));
			operands ops = RegFile.ReadWrite(Rsrc, bitset<5>(0), bitset<5>(0), word(0), bitset<1>(0));
			word op1 = ops.op1;
			word op2 = word(stoi(temp.at(2)));

      cycle_ins ins = cycle_ins(ins_num);

      if(cycle_num < cycles.size()){
        cycle_ins dram_ins = cycles[cycle_num][0];
        bool seperate = false;
        bool to_to_conflict = (temp[0] == dram_ins.to);
        bool to_from_conflict = (temp[0] == dram_ins.from);
        bool from_to_conflict = (temp[1] == dram_ins.to);
        if(to_to_conflict or to_from_conflict or from_to_conflict){
          seperate = true;
        }
        if(seperate){
          while(cycle_num < cycles.size()){
            cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
            cout<<"INSTRUCTION : DRAM request\n";
            if(cycle_num == cycles.size()-1){
              RegFile.OutputRF();
            }
            cycle_num++;
          }
          cycles.push_back({ins});
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(1), op1, op2);    //////////// send the instruction to add using opcode = 1
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
        else{
          cycles[cycle_num].push_back(ins);
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION : DRAM ACCESS and "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(1), op1, op2);    //////////// send the instruction to add using opcode = 1
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
      }
      else{
        cycles.push_back({ins});
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
        ALUresult res = my_ALU.ALUOperation(bitset<4>(1), op1, op2);    //////////// send the instruction to add using opcode = 1
        RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
        RegFile.OutputRF();
        cycle_num++;
      }

			programCounter++;

		}
		else if(operator_temp==5){     ///// beq
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
      cycle_ins ins = cycle_ins(ins_num);
      while(cycle_num < cycles.size()){
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM request\n";
        if(cycle_num == cycles.size()-1){
          RegFile.OutputRF();
        }
        cycle_num++;
      }
      cycles.push_back({ins});
      cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : BEQ\n";
			ALUresult res = my_ALU.ALUOperation(bitset<4>(2), op1, op2);    //////////// send the instruction to sub using opcode = 2
      cout<<"Comparison performed in ALU for beq .\n";
			if(res.isZero.any()){
				auto pc  = my_mem.labels.find(go_to);
				programCounter = pc->second ;
			}
			else{
				programCounter++;
			}
      cycle_num++;
		}
		else if(operator_temp==6){     /// bne
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
      while(cycle_num < cycles.size()){
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM request\n";
        if(cycle_num == cycles.size()-1){
          RegFile.OutputRF();
        }
        cycle_num++;
      }
      cycle_ins ins = cycle_ins(ins_num);
      cycles.push_back({ins});
      cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : BNE\n";
      ALUresult res = my_ALU.ALUOperation(bitset<4>(2), op1, op2);    //////////// send the instruction to sub using opcode = 2
      cout<<"Comparison performed in ALU for bne .\n";
			if(!res.isZero.any()){
				auto pc  = my_mem.labels.find(go_to);
				programCounter = pc->second ;
			}
			else{
				programCounter++;
			}
      cycle_num++;
		}
		else if(operator_temp==7){     //// slt
			vector<string> temp = curr_inst.operands;
			bitset<5> Rdest = regCode(temp.at(0));
			bitset<5> Rsrc1 = regCode(temp.at(1));
			string Src2 = temp.at(2);
      bool imm_val = is_imm(Src2);
			word op1,op2;
			if(imm_val){
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

      cycle_ins ins = cycle_ins(ins_num);

      if(cycle_num < cycles.size()){
        cycle_ins dram_ins = cycles[cycle_num][0];
        bool seperate = false;
        bool to_to_conflict = (temp[0] == dram_ins.to);
        bool to_from_conflict = (temp[0] == dram_ins.from);
        bool from_to_conflict = (temp[1] == dram_ins.to);
        if(to_to_conflict or to_from_conflict or from_to_conflict){
          seperate = true;
        }
        if(!imm_val and (temp[2] == dram_ins.to)){
          seperate = true;
        }
        if(seperate){
          while(cycle_num < cycles.size()){
            cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
            cout<<"INSTRUCTION : DRAM request\n";
            if(cycle_num == cycles.size()-1){
              RegFile.OutputRF();
            }
            cycle_num++;
          }
          cycles.push_back({ins});
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(7), op1, op2);
          cout<<"Comparison performed in ALU for slt .\n";
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
        else{
          cycles[cycle_num].push_back(ins);
          cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
          cout<<"INSTRUCTION : DRAM ACCESS and "<<name(operator_temp)<<endl;
          ALUresult res = my_ALU.ALUOperation(bitset<4>(7), op1, op2);
          cout<<"Comparison performed in ALU for slt .\n";
    			RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
          RegFile.OutputRF();
          cycle_num++;
        }
      }
      else{
        cycles.push_back({ins});
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : "<<name(operator_temp)<<endl;
        ALUresult res = my_ALU.ALUOperation(bitset<4>(7), op1, op2);
        cout<<"Comparison performed in ALU for slt .\n";
        RegFile.ReadWrite(bitset<5>(0),bitset<5>(0),Rdest, res.CalcResult, bitset<1>(1));
        RegFile.OutputRF();
        cycle_num++;
      }
			programCounter++;
		}
		else if(operator_temp==8){       /// load word
			vector<string> temp = curr_inst.operands;
			bitset<5> ReadReg = regCode(temp.at(0));
			string Address = temp.at(1);
      while(cycle_num < cycles.size()){
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM request\n";
        if(cycle_num == cycles.size()-1){
          RegFile.OutputRF();
        }
        cycle_num++;
      }
      cycle_ins ins = cycle_ins(ins_num, temp[0], temp[1]);
      cycles.push_back({ins});
      cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : LW\n";
      int c1 = my_mem.ReadWrite(Address,ReadReg, bitset<5>(0), bitset<1>(1), bitset<1>(0),RegFile );
      cycle_num++;
      for(int j = 0; j<c1; j++){     //// extra cycles DRAM request will take
        cycles.push_back({ins});
      }
			programCounter++;
		}
		else if(operator_temp==9){      //// store word
			vector<string> temp = curr_inst.operands;
			bitset<5> WrtReg = regCode(temp.at(0));
			string Address = temp.at(1);
      while(cycle_num < cycles.size()){
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM request\n";
        if(cycle_num == cycles.size()-1){
          RegFile.OutputRF();
        }
        cycle_num++;
      }
      cycle_ins ins = cycle_ins(ins_num, temp[1], temp[0]);
      cycles.push_back({ins});
      cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : SW\n";
      int c1 = my_mem.ReadWrite(Address,bitset<5>(0),WrtReg, bitset<1>(0), bitset<1>(1),RegFile );
      cycle_num++;
      for(int j = 0; j<c1; j++){
        cycles.push_back({ins});
      }
			programCounter++;
		}
		else if(operator_temp==10){     ////// j
			string go_to = curr_inst.operands.at(0);
			auto pc = my_mem.labels.find(go_to);
			programCounter = pc->second ;
      while(cycle_num < cycles.size()){
        cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
        cout<<"INSTRUCTION : DRAM request\n";
        if(cycle_num == cycles.size()-1){
          RegFile.OutputRF();
        }
        cycle_num++;
      }
      cycle_ins ins = cycle_ins(ins_num);
      cycles.push_back({ins});
      cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
      cout<<"INSTRUCTION : J\n";
      cycle_num++;
		}
	}
}

int main(int argc, char** argv){
  if(argc<2){
    cout<<"Kindly run the executable using row_access_delay and col_access_delay as command line arguments.";
  }
  row_access_delay = atoi(argv[1]);
  col_access_delay = atoi(argv[2]);
  cycle_ins zero_entry = cycle_ins(-1);
  cycles.push_back({zero_entry});
  cycle_num = 1;


  remove("RF_final.txt");          //// deletes the files if present initially
  remove("dataMem_final.txt");

  regCodes_init();
  ALU my_ALU;
  Memory my_mem;
  RF RegFile = RF();
  my_mem.writeback_pending = false;

  try{
    my_mem.writeInsMem();
  }
  catch(int x){
    cout<<"Please ensure input is in correct format\n";
    return 0;
  }


  vector<string> operations = {"add", "sub", "mul", "addi", "beq" , "bne", "slt", "lw", "sw", "j"};
  vector<int> ins_counts = {0,0,0,0,0,0,0,0,0,0};
  simulate(my_ALU, my_mem, RegFile,ins_counts);
  while(cycle_num < cycles.size()){
    cout<<endl<<"CYCLE NUM : "<<cycle_num<<endl;
    cout<<"INSTRUCTION : DRAM request\n";
    if(cycle_num == cycles.size()-1){
      RegFile.OutputRF();
    }
    cycle_num++;
  }
  int final_writeback = cycle_num;
  cycle_num-=1;

  if(my_mem.writeback_pending){
    cycle_num += row_access_delay;
    cout<<endl<<"CYCLE NUM : "<<final_writeback<<" - "<<cycle_num<<endl;
    cout<<"PERFORMING THE FINAL WRITEBACK\n";
    my_mem.writeback(my_mem.active_row, cycle_num);
  }

  cout<<endl<<"STATISTICS"<<endl;
  cout<<"No. of clock cycles : Actual clock cycles + final writeback = "<<cycle_num<<endl;
  cout<<"No. of buffer updates : "<<buffer_updates<<endl;
  for(int k = 0; k<10; k++){
    int num = ins_counts.at(k);
    if(num >0){
      cout<<"No. of "<<operations.at(k)<<" instructions : "<<num<<endl;
    }
  }
  return 0;
}
