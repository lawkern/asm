/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

typedef enum {
   ADDRMODE_IMPLIED,
   ADDRMODE_ACCUMULATOR,
   ADDRMODE_IMMEDIATE,
   ADDRMODE_ZEROPAGE,
   ADDRMODE_ZEROPAGEX,
   ADDRMODE_ZEROPAGEY,
   ADDRMODE_ABSOLUTE,
   ADDRMODE_ABSOLUTEX,
   ADDRMODE_ABSOLUTEY,
   ADDRMODE_INDIRECT,
   ADDRMODE_INDIRECTX,
   ADDRMODE_INDIRECTY,
   ADDRMODE_RELATIVE,

   ADDRMODE_COUNT,
} addressing_mode;

typedef enum {
   ADDRMODE_FLAG_IMPLIED     = (1 << ADDRMODE_IMPLIED),
   ADDRMODE_FLAG_ACCUMULATOR = (1 << ADDRMODE_ACCUMULATOR),
   ADDRMODE_FLAG_IMMEDIATE   = (1 << ADDRMODE_IMMEDIATE),
   ADDRMODE_FLAG_ZEROPAGE    = (1 << ADDRMODE_ZEROPAGE),
   ADDRMODE_FLAG_ZEROPAGEX   = (1 << ADDRMODE_ZEROPAGEX),
   ADDRMODE_FLAG_ZEROPAGEY   = (1 << ADDRMODE_ZEROPAGEY),
   ADDRMODE_FLAG_ABSOLUTE    = (1 << ADDRMODE_ABSOLUTE),
   ADDRMODE_FLAG_ABSOLUTEX   = (1 << ADDRMODE_ABSOLUTEX),
   ADDRMODE_FLAG_ABSOLUTEY   = (1 << ADDRMODE_ABSOLUTEY),
   ADDRMODE_FLAG_INDIRECT    = (1 << ADDRMODE_INDIRECT),
   ADDRMODE_FLAG_INDIRECTX   = (1 << ADDRMODE_INDIRECTX),
   ADDRMODE_FLAG_INDIRECTY   = (1 << ADDRMODE_INDIRECTY),
   ADDRMODE_FLAG_RELATIVE    = (1 << ADDRMODE_RELATIVE),
} addressing_mode_flag;

static char *Addressing_Mode_Names[ADDRMODE_COUNT] =
{
   [ADDRMODE_IMPLIED]     = "Implied",
   [ADDRMODE_ACCUMULATOR] = "Accumulator",
   [ADDRMODE_IMMEDIATE]   = "Immediate",
   [ADDRMODE_ZEROPAGE]    = "Zero Page",
   [ADDRMODE_ZEROPAGEX]   = "Zero Page + X",
   [ADDRMODE_ZEROPAGEY]   = "Zero Page + Y",
   [ADDRMODE_ABSOLUTE]    = "Absolute Address",
   [ADDRMODE_ABSOLUTEX]   = "Absolute Address + X",
   [ADDRMODE_ABSOLUTEY]   = "Absolute Address + Y",
   [ADDRMODE_INDIRECT]    = "Indirect Address",
   [ADDRMODE_INDIRECTX]   = "Indirect Address + X",
   [ADDRMODE_INDIRECTY]   = "Indirect Address + Y",
   [ADDRMODE_RELATIVE]    = "Relative Address",
};

typedef struct {
   addressing_mode Addressing_Mode;
   string Label;

   int Length;
   u8 Bytes[2];

   bool Ok;
} instruction_data;

static bool Parse_And_Populate_Data_Bytes(instruction_data *Result, string Number)
{
   parsed_u32 Parsed_Number = Parse_U32(Number);
   if(Parsed_Number.Ok)
   {
      Result->Bytes[0] = (u8)Parsed_Number.Value;
      Result->Bytes[1] = (u8)(Parsed_Number.Value >> 8);
      Result->Length = Result->Bytes[1] ? 2 : 1;
   }

   return(Parsed_Number.Ok);
 }

static instruction_data Parse_Instruction_Data(string Operands, bool Is_Branch)
{
   // TODO: Enforce syntax requirements.

   instruction_data Result = {0};

   if(Operands.Length == 0)
   {
      Result.Addressing_Mode = ADDRMODE_IMPLIED;
      Result.Ok = true;
   }
   else if(Equals(Operands, S("a")))
   {
      Result.Addressing_Mode = ADDRMODE_ACCUMULATOR;
      Result.Ok = true;
   }
   else if(Has_Prefix_Then_Remove(&Operands, S("[[")))
   {
      if(Has_Suffix_Then_Remove(&Operands, S(" + x]]")))
      {
         Result.Addressing_Mode = ADDRMODE_INDIRECTX;
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("] + y]")))
      {
         Result.Addressing_Mode = ADDRMODE_INDIRECTY;
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
      }
   }
   else if(Has_Prefix_Then_Remove(&Operands, S("[")))
   {
      if(Has_Suffix_Then_Remove(&Operands, S(" + x]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEX : ADDRMODE_ZEROPAGEX;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S(" + y]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEY : ADDRMODE_ZEROPAGEY;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTE : ADDRMODE_ZEROPAGE;
      }
   }
   else if(Operands.Data[0] >= '0' && Operands.Data[0] <= '9')
   {
      Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands);
      Result.Addressing_Mode = (Result.Length == 2)
         ? ADDRMODE_ABSOLUTE
         : (Is_Branch) ? ADDRMODE_RELATIVE : ADDRMODE_IMMEDIATE;
   }
   else // Label
   {
      // TODO: Determine if this is actually a relative address.
      if(Is_Branch)
      {
         Result.Addressing_Mode = ADDRMODE_RELATIVE;
         Result.Length = 1;
      }
      else
      {
         Result.Addressing_Mode = ADDRMODE_ABSOLUTE;
         Result.Length = 2;
      }

      // TODO: Populate bytes with computed label address.
      Result.Bytes[0] = 0xBE;
      Result.Bytes[1] = 0xEF;
   }

   printf("%-20s | ", Addressing_Mode_Names[Result.Addressing_Mode]);

   return(Result);
}

typedef enum {
   // Group 1.
   OPCODE_ORA = 0x0,
   OPCODE_AND = 0x1,
   OPCODE_EOR = 0x2,
   OPCODE_ADC = 0x3,
   OPCODE_STA = 0x4,
   OPCODE_LDA = 0x5,
   OPCODE_CMP = 0x6,
   OPCODE_SBC = 0x7,

   // Group 2.
   OPCODE_ASL = 0x0,
   OPCODE_ROL = 0x1,
   OPCODE_LSR = 0x2,
   OPCODE_ROR = 0x3,
   OPCODE_STX = 0x4,
   OPCODE_LDX = 0x5,
   OPCODE_DEC = 0x6,
   OPCODE_INC = 0x7,

   // Group 3.
   OPCODE_BIT    = 0x1,
   OPCODE_JMP    = 0x2,
   OPCODE_JMPABS = 0x3,
   OPCODE_STY    = 0x4,
   OPCODE_LDY    = 0x5,
   OPCODE_CPY    = 0x6,
   OPCODE_CPX    = 0x7,

   // Branches.
   OPCODE_BPL = 0x10,
   OPCODE_BMI = 0x30,
   OPCODE_BVC = 0x50,
   OPCODE_BVS = 0x70,
   OPCODE_BCC = 0x90,
   OPCODE_BCS = 0xB0,
   OPCODE_BNE = 0xD0,
   OPCODE_BEQ = 0xF0,
} opcode_pattern;

// TODO: Mark illegal indices with unused value.
static u8 Group1_Addr_Patterns[ADDRMODE_COUNT] =
{
   [ADDRMODE_INDIRECTX]   = 0x0,
   [ADDRMODE_ZEROPAGE]    = 0x1,
   [ADDRMODE_IMMEDIATE]   = 0x2,
   [ADDRMODE_ABSOLUTE]    = 0x3,
   [ADDRMODE_INDIRECTY]   = 0x4,
   [ADDRMODE_ZEROPAGEX]   = 0x5,
   [ADDRMODE_ABSOLUTEY]   = 0x6,
   [ADDRMODE_ABSOLUTEX]   = 0x7,
};

static u8 Group2_Addr_Patterns[ADDRMODE_COUNT] =
{
   [ADDRMODE_IMMEDIATE]   = 0x0,
   [ADDRMODE_ZEROPAGE]    = 0x1,
   [ADDRMODE_ACCUMULATOR] = 0x2,
   [ADDRMODE_ABSOLUTE]    = 0x3,
   [ADDRMODE_ZEROPAGEX]   = 0x5,
   [ADDRMODE_ABSOLUTEX]   = 0x7,
};

static u8 Group3_Addr_Patterns[ADDRMODE_COUNT] =
{
   [ADDRMODE_IMMEDIATE] = 0x0,
   [ADDRMODE_ZEROPAGE]  = 0x1,
   [ADDRMODE_ABSOLUTE]  = 0x3,
   [ADDRMODE_ZEROPAGEX] = 0x5,
   [ADDRMODE_ABSOLUTEX] = 0x7,
};

static machine_instruction Generate_Group1(opcode_pattern Opcode, string Operands)
{
   // Byte layout: aaabbbcc (aaa = GROUP1_OPCODE, bbb = GROUP1_ADDRMODE, cc = 01)

   instruction_data Data = Parse_Instruction_Data(Operands, false);
   u8 Addressing_Code = Group1_Addr_Patterns[Data.Addressing_Mode];
   u8 CC = 0x01;

   machine_instruction Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = (Opcode << 5) | (Addressing_Code << 2) | CC;
   Result.Bytes[1] = Data.Bytes[0];
   Result.Bytes[2] = Data.Bytes[1];

   return(Result);
}

static machine_instruction Generate_Group2(opcode_pattern Opcode, string Operands)
{
   // Byte layout: aaabbbcc (aaa = GROUP2_OPCODE, bbb = GROUP2_ADDRMODE, cc = 10)

   instruction_data Data = Parse_Instruction_Data(Operands, false);
   u8 Addressing_Code = Group2_Addr_Patterns[Data.Addressing_Mode];
   u8 CC = 0x02;

   machine_instruction Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = (Opcode << 5) | (Addressing_Code << 2) | CC;
   Result.Bytes[1] = Data.Bytes[0];
   Result.Bytes[2] = Data.Bytes[1];

   return(Result);
}

static machine_instruction Generate_Group3(opcode_pattern Opcode, string Operands)
{
   // Byte layout: aaabbbcc (aaa = GROUP3_OPCODE, bbb = GROUP3_ADDRMODE, cc = 00)

   instruction_data Data = Parse_Instruction_Data(Operands, false);
   u8 Addressing_Code = Group3_Addr_Patterns[Data.Addressing_Mode];
   u8 CC = 0x00;
   if(Opcode == OPCODE_JMP && Data.Addressing_Mode == ADDRMODE_ABSOLUTE)
   {
      Opcode = OPCODE_JMPABS;
   }

   machine_instruction Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = (Opcode << 5) | (Addressing_Code << 2) | CC;
   Result.Bytes[1] = Data.Bytes[0];
   Result.Bytes[2] = Data.Bytes[1];

   return(Result);
}

static machine_instruction Generate_Branch(opcode_pattern Opcode, string Operands)
{
   instruction_data Data = Parse_Instruction_Data(Operands, true);

   machine_instruction Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = Opcode;
   Result.Bytes[1] = Data.Bytes[0];

   return(Result);
}

static GENERATE_MACHINE_INSTRUCTION(Generate_Machine_Instruction)
{
   machine_instruction Result = {0};
   printf("%-16.*s | ", Line.Instruction.Length, Line.Instruction.Data);

   // TODO: Collapse all this down into a hash table or something.

   // TODO: Support other whitespace characters.
   cut Instruction_Operands = Cut(Line.Instruction, ' ');
   if(Instruction_Operands.Found)
   {
      string Mnemonic = Instruction_Operands.Before;
      string Operands = Trim_Left(Instruction_Operands.After);

      // Group 1 instructions.
      if     (Equals(Mnemonic, S("ora"))) Result = Generate_Group1(OPCODE_ORA, Operands);
      else if(Equals(Mnemonic, S("and"))) Result = Generate_Group1(OPCODE_AND, Operands);
      else if(Equals(Mnemonic, S("eor"))) Result = Generate_Group1(OPCODE_EOR, Operands);
      else if(Equals(Mnemonic, S("adc"))) Result = Generate_Group1(OPCODE_ADC, Operands);
      else if(Equals(Mnemonic, S("sta"))) Result = Generate_Group1(OPCODE_STA, Operands);
      else if(Equals(Mnemonic, S("lda"))) Result = Generate_Group1(OPCODE_LDA, Operands);
      else if(Equals(Mnemonic, S("cmp"))) Result = Generate_Group1(OPCODE_CMP, Operands);
      else if(Equals(Mnemonic, S("sbc"))) Result = Generate_Group1(OPCODE_SBC, Operands);

      // Group 2 instructions.
      else if(Equals(Mnemonic, S("asl"))) Result = Generate_Group2(OPCODE_ASL, Operands);
      else if(Equals(Mnemonic, S("rol"))) Result = Generate_Group2(OPCODE_ROL, Operands);
      else if(Equals(Mnemonic, S("lsr"))) Result = Generate_Group2(OPCODE_LSR, Operands);
      else if(Equals(Mnemonic, S("ror"))) Result = Generate_Group2(OPCODE_ROR, Operands);
      else if(Equals(Mnemonic, S("stx"))) Result = Generate_Group2(OPCODE_STX, Operands);
      else if(Equals(Mnemonic, S("ldx"))) Result = Generate_Group2(OPCODE_LDX, Operands);
      else if(Equals(Mnemonic, S("dec"))) Result = Generate_Group2(OPCODE_DEC, Operands);
      else if(Equals(Mnemonic, S("inc"))) Result = Generate_Group2(OPCODE_INC, Operands);

      // Group 3 instructions.
      else if(Equals(Mnemonic, S("bit"))) Result = Generate_Group3(OPCODE_BIT, Operands);
      else if(Equals(Mnemonic, S("jmp"))) Result = Generate_Group3(OPCODE_JMP, Operands);
      else if(Equals(Mnemonic, S("sty"))) Result = Generate_Group3(OPCODE_STY, Operands);
      else if(Equals(Mnemonic, S("ldy"))) Result = Generate_Group3(OPCODE_LDY, Operands);
      else if(Equals(Mnemonic, S("cpy"))) Result = Generate_Group3(OPCODE_CPY, Operands);
      else if(Equals(Mnemonic, S("cpx"))) Result = Generate_Group3(OPCODE_CPX, Operands);

      // Branch instructions.
      else if(Equals(Mnemonic, S("bpl"))) Result = Generate_Branch(OPCODE_BPL, Operands);
      else if(Equals(Mnemonic, S("bmi"))) Result = Generate_Branch(OPCODE_BMI, Operands);
      else if(Equals(Mnemonic, S("bvc"))) Result = Generate_Branch(OPCODE_BVC, Operands);
      else if(Equals(Mnemonic, S("bvs"))) Result = Generate_Branch(OPCODE_BVS, Operands);
      else if(Equals(Mnemonic, S("bcc"))) Result = Generate_Branch(OPCODE_BCC, Operands);
      else if(Equals(Mnemonic, S("bcs"))) Result = Generate_Branch(OPCODE_BCS, Operands);
      else if(Equals(Mnemonic, S("bne"))) Result = Generate_Branch(OPCODE_BNE, Operands);
      else if(Equals(Mnemonic, S("beq"))) Result = Generate_Branch(OPCODE_BEQ, Operands);
   }
   else
   {
      // Single-byte instructions.
      if     (Equals(Line.Instruction, S("brk"))) Result.Bytes[Result.Length++] = 0x00;
      else if(Equals(Line.Instruction, S("rti"))) Result.Bytes[Result.Length++] = 0x40;
      else if(Equals(Line.Instruction, S("rts"))) Result.Bytes[Result.Length++] = 0x60;

      else if(Equals(Line.Instruction, S("php"))) Result.Bytes[Result.Length++] = 0x08;
      else if(Equals(Line.Instruction, S("plp"))) Result.Bytes[Result.Length++] = 0x28;
      else if(Equals(Line.Instruction, S("pha"))) Result.Bytes[Result.Length++] = 0x48;
      else if(Equals(Line.Instruction, S("pla"))) Result.Bytes[Result.Length++] = 0x68;
      else if(Equals(Line.Instruction, S("dey"))) Result.Bytes[Result.Length++] = 0x88;
      else if(Equals(Line.Instruction, S("tay"))) Result.Bytes[Result.Length++] = 0xA8;
      else if(Equals(Line.Instruction, S("iny"))) Result.Bytes[Result.Length++] = 0xC8;
      else if(Equals(Line.Instruction, S("inx"))) Result.Bytes[Result.Length++] = 0xE8;

      else if(Equals(Line.Instruction, S("clc"))) Result.Bytes[Result.Length++] = 0x18;
      else if(Equals(Line.Instruction, S("sec"))) Result.Bytes[Result.Length++] = 0x38;
      else if(Equals(Line.Instruction, S("cli"))) Result.Bytes[Result.Length++] = 0x58;
      else if(Equals(Line.Instruction, S("sei"))) Result.Bytes[Result.Length++] = 0x78;
      else if(Equals(Line.Instruction, S("tya"))) Result.Bytes[Result.Length++] = 0x98;
      else if(Equals(Line.Instruction, S("clv"))) Result.Bytes[Result.Length++] = 0xB8;
      else if(Equals(Line.Instruction, S("cld"))) Result.Bytes[Result.Length++] = 0xD8;
      else if(Equals(Line.Instruction, S("sed"))) Result.Bytes[Result.Length++] = 0xF8;

      else if(Equals(Line.Instruction, S("txa"))) Result.Bytes[Result.Length++] = 0x8A;
      else if(Equals(Line.Instruction, S("txs"))) Result.Bytes[Result.Length++] = 0x9A;
      else if(Equals(Line.Instruction, S("tax"))) Result.Bytes[Result.Length++] = 0xAA;
      else if(Equals(Line.Instruction, S("tsx"))) Result.Bytes[Result.Length++] = 0xBA;
      else if(Equals(Line.Instruction, S("dex"))) Result.Bytes[Result.Length++] = 0xCA;
      else if(Equals(Line.Instruction, S("nop"))) Result.Bytes[Result.Length++] = 0xEA;

      printf("%-20s | ", Addressing_Mode_Names[ADDRMODE_IMPLIED]);
   }

   return(Result);
}
