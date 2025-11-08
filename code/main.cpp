#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdint>

using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32> PC;
    bool nop;
};

struct IDStruct {
    bitset<32> Instr;
    bool nop;
};

struct EXStruct {
    bitset<32> Read_data1;
    bitset<32> Read_data2;
    bitset<16> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type;
    bool rd_mem;
    bool wrt_mem;
    bool alu_op;
    bool wrt_enable;
    bool nop;
};

struct MEMStruct {
    bitset<32> ALUresult;
    bitset<32> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;
    bool nop;
};

struct WBStruct {
    bitset<32> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct {
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

class InsMem {
    public:
        string id, ioDir;
        
    InsMem(string name, string ioDir) {
        id = name;
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open(ioDir + "\\imem.txt");
        if (imem.is_open()) {
            while (getline(imem, line)) {
                IMem[i] = bitset<8>(line);
                i++;
            }
        } else {
            cout << "Unable to open IMEM input file." << endl;
        }
        imem.close();
    }

        bitset<32> readInstr(bitset<32> ReadAddress) {
            bitset<32> instr;
            unsigned int addr = ReadAddress.to_ulong();
            instr = (IMem[addr].to_ulong() << 24) | 
                    (IMem[addr + 1].to_ulong() << 16) | 
                    (IMem[addr + 2].to_ulong() << 8) | 
                    (IMem[addr + 3].to_ulong());
            return instr;
        }

    private:
        vector<bitset<8>> IMem;
};

class DataMem {
public:
    string id, opFilePath, ioDir;
    DataMem(string name, string ioDir) : id{name}, ioDir{ioDir} {
        DMem.resize(MemSize);
        opFilePath = ioDir + "\\" + name + "_DMEMResult.txt";
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open(ioDir + "\\dmem.txt");
        if (dmem.is_open()) {
            while (getline(dmem, line)) {
                DMem[i] = bitset<8>(line);
                i++;
            }
        } else {
            cout << "Unable to open DMEM input file." << endl;
        }
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address) {
        unsigned int addr = Address.to_ulong();
        bitset<32> data;
        data = (DMem[addr].to_ulong() << 24) |
               (DMem[addr + 1].to_ulong() << 16) |
               (DMem[addr + 2].to_ulong() << 8) |
               (DMem[addr + 3].to_ulong());
        return data;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
        unsigned int addr = Address.to_ulong();
        DMem[addr] = bitset<8>((WriteData.to_ulong() >> 24) & 0xFF);
        DMem[addr + 1] = bitset<8>((WriteData.to_ulong() >> 16) & 0xFF);
        DMem[addr + 2] = bitset<8>((WriteData.to_ulong() >> 8) & 0xFF);
        DMem[addr + 3] = bitset<8>(WriteData.to_ulong() & 0xFF);
    }

    void outputDataMem() {
        ofstream dmemout;
        dmemout.open(opFilePath, std::ios_base::trunc);
        if (dmemout.is_open()) {
            for (int j = 0; j < 1000; j++) {
                dmemout << DMem[j] << endl;
            }
            dmemout.close();
        }
    }

private:
    vector<bitset<8>> DMem;
};

class RegisterFile {
public:
    string outputFile;
    RegisterFile(string ioDir) : outputFile{ioDir + "RFResult.txt"} {
        Registers.resize(32);
        for (int i = 0; i < 32; i++) {
            Registers[i] = bitset<32>(0);
        }
    }

    bitset<32> readRF(bitset<5> Reg_addr) {
        unsigned int addr = Reg_addr.to_ulong();
        if (addr == 0) {
            return bitset<32>(0);
        }
        return Registers[addr];
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
        unsigned int addr = Reg_addr.to_ulong();
        if (addr != 0) {
            Registers[addr] = Wrt_reg_data;
        }
    }

    void outputRF(int cycle) {
        ofstream rfout;
        if (cycle == 0) {
            rfout.open(outputFile, std::ios_base::trunc);
        } else {
            rfout.open(outputFile, std::ios_base::app);
        }
        if (rfout.is_open()) {
            rfout << "State of RF after executing cycle:  " << cycle << endl;
            for (int i = 0; i < 32; i++) {
                rfout << Registers[i] << endl;
            }
            rfout.close();
        }
    }

private:
    vector<bitset<32>> Registers;
};

class Core {
public:
    RegisterFile myRF;
    uint32_t cycle;
    bool halted;
    string ioDir;
    struct stateStruct state, nextState;
    InsMem ext_imem;
    DataMem ext_dmem;
    uint32_t total_cycles;
    uint32_t total_instructions;

    Core(string ioDir, InsMem &imem, DataMem &dmem)
        : myRF(ioDir), cycle(0), halted(false), ioDir{ioDir},
          ext_imem{imem}, ext_dmem{dmem}, total_cycles(0), total_instructions(0) {
        state.IF.PC = bitset<32>(0);
        state.IF.nop = false;
        state.ID.nop = true;
        state.EX.nop = true;
        state.MEM.nop = true;
        state.WB.nop = true;
    }

    virtual void step() {}
    virtual void printState() {}
};

class SingleStageCore : public Core {
public:
    string opFilePath;
    
    SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem)
        : Core(ioDir + "\\SS_", imem, dmem),
          opFilePath(ioDir + "\\StateResult_SS.txt") {}

    void step() {
        
        if (state.IF.nop && cycle > 0) {
            myRF.outputRF(cycle);
            printState(state, cycle);
            total_cycles++;
            cycle++;
            halted = true;
            return;
        }
        
        // IF
        bitset<32> current_pc = state.IF.PC;
        bitset<32> instr = ext_imem.readInstr(current_pc);
        
        // ID
        unsigned int opcode = 0;
        for (int i = 0; i < 7; i++) {
            if (instr[i]) opcode |= (1 << i);
        }

        bitset<5> rd, rs1, rs2;
        for (int i = 0; i < 5; i++) {
            rd[i] = instr[7 + i];
            rs1[i] = instr[15 + i];
            rs2[i] = instr[20 + i];
        }

        bitset<32> read_data1 = myRF.readRF(rs1);
        bitset<32> read_data2 = myRF.readRF(rs2);
        
        // EX - Execute
        bitset<32> alu_result;
        bitset<32> store_data = read_data2;
        bool rd_mem = false, wrt_mem = false, wrt_enable = false;
        
        if (opcode == 0x03) {
            bitset<12> imm;

            for (int i = 0; i < 12; i++) 
                imm[i] = instr[20 + i];
            
            int32_t offset = imm.to_ulong();
            
            if (imm[11]) 
                offset |= 0xFFFFF000;
            
            alu_result = bitset<32>(read_data1.to_ulong() + offset);
            rd_mem = true;
            wrt_enable = true;
        } else if (opcode == 0x13) {
            bitset<12> imm;

            for (int i = 0; i < 12; i++) 
                imm[i] = instr[20 + i];
            
            int32_t imm_val = imm.to_ulong();
            
            if (imm[11]) 
                imm_val |= 0xFFFFF000;
            
            alu_result = bitset<32>(read_data1.to_ulong() + imm_val);
            wrt_enable = true;
        } else if (opcode == 0x23) { 
            bitset<12> s_imm;

            for (int i = 0; i < 5; i++) 
                s_imm[i] = instr[7 + i];

            for (int i = 5; i < 12; i++) 
                s_imm[i] = instr[25 + i - 5];
            
            int32_t offset = s_imm.to_ulong();

            if (s_imm[11]) 
                offset |= 0xFFFFF000;
            
            alu_result = bitset<32>(read_data1.to_ulong() + offset);
            wrt_mem = true;
        } else if (opcode == 0x33) {
            bitset<7> funct7;

            for (int i = 0; i < 7; i++) 
                funct7[i] = instr[25 + i];
            
            if (funct7.to_ulong() == 0x20) { 
                alu_result = bitset<32>(read_data1.to_ulong() - read_data2.to_ulong());
            } else {
                alu_result = bitset<32>(read_data1.to_ulong() + read_data2.to_ulong());
            }
            wrt_enable = true;
        }
        
        // MEM
        bitset<32> wb_data;
        if (rd_mem) {
            wb_data = ext_dmem.readDataMem(alu_result);
        } else if (wrt_mem) {
            ext_dmem.writeDataMem(alu_result, store_data);
            wb_data = alu_result;
        } else {
            wb_data = alu_result;
        }
        
        // WB
        if (wrt_enable && opcode != 0x7F) {
            myRF.writeRF(rd, wb_data);
        }
        
        // update PC
        if (opcode == 0x7F) {  // HALT
            state.IF.nop = true;
        } else {
            state.IF.PC = bitset<32>(current_pc.to_ulong() + 4);
        }
        
        myRF.outputRF(cycle);
        printState(state, cycle);
        
        total_cycles++;
        total_instructions++;
        cycle++;
        
    }

    void printState(stateStruct state, int cycle) {
        ofstream printstate;
        if (cycle == 0) {
            printstate.open(opFilePath, std::ios_base::trunc);
        } else {
            printstate.open(opFilePath, std::ios_base::app);
        }
        if (printstate.is_open()) {
            printstate << "----------------------------------------------------------------------" << endl;
            printstate << "State after executing cycle: " << cycle << endl;
            printstate << "IF.PC: " << state.IF.PC.to_ulong() << endl;
            printstate << "IF.nop: " << (state.IF.nop ? "True" : "False") << endl;
            printstate.close();
        }
    }
};

int main(int argc, char* argv[]) {
    string ioDir;

    if (argc != 2) {
        cout << "Usage: ./simulator.exe <ioDirectory>" << endl;
        return -1;
    }

    ioDir = argv[1];
    cout << "IO Directory: " << ioDir << endl;

    InsMem imem = InsMem("Imem", ioDir);
    DataMem dmem_ss = DataMem("SS", ioDir);

    SingleStageCore SSCore(ioDir, imem, dmem_ss);

    while (!SSCore.halted) {
        SSCore.step();
    }

    dmem_ss.outputDataMem();

    ofstream metrics;
    metrics.open(ioDir + "\\SS_Metrics.txt", std::ios_base::trunc);
    if (metrics.is_open()) {
        double cpi = (double)SSCore.total_cycles / (SSCore.total_instructions > 0 ? SSCore.total_instructions : 1);
        metrics << "Total Cycles: " << SSCore.total_cycles << endl;
        metrics << "Total Instructions: " << SSCore.total_instructions << endl;
        metrics << "Average CPI: " << fixed << setprecision(2) << cpi << endl;
        metrics << "Instructions Per Cycle: " << fixed << setprecision(2) << 1.0 / cpi << endl;
        metrics.close();
    }

    cout << "Simulation completed!" << endl;
    cout << "Total Cycles: " << SSCore.total_cycles << endl;
    cout << "Total Instructions: " << SSCore.total_instructions << endl;
    if (SSCore.total_instructions > 0) {
        cout << "Average CPI: " << fixed << setprecision(2) << (double)SSCore.total_cycles / SSCore.total_instructions << endl;
    }

    return 0;
}
