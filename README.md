# RISC-V Processor Implementation (Single stage currently)

## Overview

### File Structure

```
RISC-V SIM/
├── code/
│   └── main.cpp
│        ├── Classes
│        │  ├── InsMem: Manages instruction memory
│        │  │   └── readInstr(): Reads 4 bytes in BIG-endian format
│        │  ├── DataMem: Manages data memory
│        │  │   ├── readDataMem(): Loads 32-bit words
│        │  │   └── writeDataMem(): Stores 32-bit words
│        │  ├── RegisterFile: 32 registers with R0 constraint
│        │  │   ├── readRF(): Returns register value
│        │  │   └── writeRF(): Updates register
│        │  ├── Core: Base processor class
│        │  │   └── Holds shared state & memory references
│        │  └── SingleStageCore: Actual processor
│        │       └── step(): Execute one cycle (all 5 stages)
│        │
│        ├── Helper Functions
│        │   └── signExtend12/20(): Immediate extension to 32 bits
│        └── main()
│            └── Command-line argument handling
│            └── File I/O setup
│            └── Simulation loop
│            └── Metrics output
├── Test Cases/
│   ├── input/
│   │   ├── testcase0
│   │   ├── testcase1
│   │   └── testcase2  
│   └── output/
│       └── sample outputs for each testcase
├── README.MD
└── SS_Schematic.pdf
```

### Implementation
- **code/main.cpp** - Complete single-stage processor implementation
  - InsMem class: Instruction memory with big-endian support
  - DataMem class: Data memory with read/write operations
  - RegisterFile class: 32-bit register file with R0 constraint
  - SingleStageCore class: Main processor with all 5 stages
  - Performance metrics tracking (# of cycles & instructions, CPI & IPC)

- **IMPORTANT**: main.cpp expects imem.txt and dmem.txt in binary format from a directory, 8 bits (1 byte) per line

### Documentation Files
- **SS_Schematic.md** - Architecture & datapath diagram
   - Block diagram of processor
   - Detailed stage datapaths
   - State machine flowchart

- **README.MD** (this doc) - general overview

### Test Files
- **dmem.txt** - data memory (binary)
- **imem.txt** - instruction memory (binary)
- **code.asm** - corresponding risc-v assembly instructions
- **Test Cases/input** - directory with all test cases containing dmem.txt and imem.txt

## Quick Start

### 1. Compile
```bash
cd code
g++ -std=c++11 -o simulator main.cpp
```

OR

```bash
cd code
g++ main.cpp -o simulator
```

- **NOTE**: ```-std=c++11``` is optional, but recommended for bitset library; known occurences of issues with modern C++ libraries  

### 2. Run

```bash
./simulator.exe ..\Test Cases\input\{your_testcase_dir}      # Windows
./simulator ../Test Cases/input/{your_testcase_dir}         # Linux/Mac
```

### 3. Check Output
- `SS_RFResult.txt` - Register file state
- `StateResult_SS.txt` - Execution state
- `SS_DMEMResult.txt` - Final memory
- `SS_Metrics.txt` - Performance metrics

- **NOTE**: output files are saved in ioDir provided at invokation

## Key Design Decisions

### 1. Single-Cycle Execution
All 5 stages execute within ONE cycle, unlike a traditional 5-stage pipeline. This matches the "single-stage" requirement while still processing through all stages.

### 2. Big-Endian Format
- MSB stored at lowest address (as specified)
- 4 consecutive bytes form 32-bit word
- Properly implemented in both readInstr() and readDataMem()

### 3. Control Signal Generation
- Extracted from opcode bits [6:0]
- Format-specific decoding for each instruction type
- Immediate field extraction based on format

### 4. Performance Metrics
- Counted instruction executions in ID stage
- Total cycles incremented each step()
- CPI = total_cycles / total_instructions (should be ~1.0)


## Testing

The implementation includes:
- Sample imem.txt with test instructions
- Sample dmem.txt with initial memory state
- Automatic execution tracing
- Register state logging after each cycle
- Final memory dump verification

**Expected Results:**
- Program executes all instructions provided from imem.txt
- Total cycles = # of instructions (or similar based on HALT placement)
- CPI ≈ 1.0
- All output files generated in ioDir

## 📚 Resources

- **RISC-V Specification**: https://riscv.org/wp-content/uploads/2019/12/riscv-spec-20191213.pdf
- **Project Documents**: See attachments in submission folder
- **C++ Bitset**: https://en.cppreference.com/w/cpp/utility/bitset
