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

// static char *Addressing_Mode_Names[ADDRMODE_COUNT] =
// {
//    [ADDRMODE_IMPLIED]     = "Implied",
//    [ADDRMODE_ACCUMULATOR] = "Accumulator",
//    [ADDRMODE_IMMEDIATE]   = "Immediate",
//    [ADDRMODE_ZEROPAGE]    = "Zero Page",
//    [ADDRMODE_ZEROPAGEX]   = "Zero Page + X",
//    [ADDRMODE_ZEROPAGEY]   = "Zero Page + Y",
//    [ADDRMODE_ABSOLUTE]    = "Absolute Address",
//    [ADDRMODE_ABSOLUTEX]   = "Absolute Address + X",
//    [ADDRMODE_ABSOLUTEY]   = "Absolute Address + Y",
//    [ADDRMODE_INDIRECT]    = "Indirect Address",
//    [ADDRMODE_INDIRECTX]   = "Indirect Address + X",
//    [ADDRMODE_INDIRECTY]   = "Indirect Address + Y",
//    [ADDRMODE_RELATIVE]    = "Relative Address",
// };

typedef struct {
   addressing_mode Addressing_Mode;
   string Label_Operand;

   int Length;
   u8 Bytes[2];

   bool Ok;
} instruction_data;

static bool Parse_And_Populate_Data_Bytes(instruction_data *Result, string Operands, bool Is_Branch)
{
   bool Ok = false;

   if(Operands.Length && Operands.Data[0] >= '0' && Operands.Data[0] <= '9')
   {
      // Number literal
      parsed_u32 Parsed_Number = Parse_U32(Operands);
      if(Parsed_Number.Ok)
      {
         Result->Bytes[0] = (u8)Parsed_Number.Value;
         Result->Bytes[1] = (u8)(Parsed_Number.Value >> 8);
         Result->Length = Result->Bytes[1] ? 2 : 1;
         Ok = true;
      }
   }
   else
   {
      // Label (to be populated later)
      Result->Bytes[0] = 0xBE;
      Result->Bytes[1] = 0xEF;
      Result->Label_Operand = Operands;
      Result->Length = (Is_Branch) ? 1 : 2;
   }

   return(Ok);
 }

static instruction_data Parse_Operands(string Operands, bool Is_Branch, bool Is_Jump)
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
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("] + y]")))
      {
         Result.Addressing_Mode = ADDRMODE_INDIRECTY;
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
      }
   }
   else if(Has_Prefix_Then_Remove(&Operands, S("[")))
   {
      if(Has_Suffix_Then_Remove(&Operands, S(" + x]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEX : ADDRMODE_ZEROPAGEX;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S(" + y]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEY : ADDRMODE_ZEROPAGEY;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
         Result.Addressing_Mode = (Result.Length == 2)
            ? (Is_Jump) ? ADDRMODE_INDIRECT : ADDRMODE_ABSOLUTE
            : ADDRMODE_ZEROPAGE;
      }
   }
   else if(Operands.Data[0] >= '0' && Operands.Data[0] <= '9')
   {
      Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
      Result.Addressing_Mode = (Result.Length == 2)
         ? ADDRMODE_ABSOLUTE
         : (Is_Branch) ? ADDRMODE_RELATIVE : ADDRMODE_IMMEDIATE;
   }
   else // Label
   {
      Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch);
      Result.Addressing_Mode = (Is_Branch) ? ADDRMODE_RELATIVE : ADDRMODE_ABSOLUTE;
   }

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

   // Special case.
   OPCODE_JSR = 0x20,
} opcode_pattern;

typedef enum {
   INSTRCODE_GROUP3 = 0x00,
   INSTRCODE_GROUP1 = 0x01,
   INSTRCODE_GROUP2 = 0x02,
   INSTRCODE_BRANCH = 0x03, // Only for identification, not part of encoding.
   INSTRCODE_JSR    = 0x04  // Only for identification, not part of encoding.
} instruction_group_code;

// TODO: Mark illegal indices with unused value.
static u8 Addressing_Mode_Patterns[][ADDRMODE_COUNT] =
{
   [INSTRCODE_GROUP1] =
   {
      [ADDRMODE_INDIRECTX]   = 0x0,
      [ADDRMODE_ZEROPAGE]    = 0x1,
      [ADDRMODE_IMMEDIATE]   = 0x2,
      [ADDRMODE_ABSOLUTE]    = 0x3,
      [ADDRMODE_INDIRECTY]   = 0x4,
      [ADDRMODE_ZEROPAGEX]   = 0x5,
      [ADDRMODE_ABSOLUTEY]   = 0x6,
      [ADDRMODE_ABSOLUTEX]   = 0x7,
   },
   [INSTRCODE_GROUP2] =
   {
      [ADDRMODE_IMMEDIATE]   = 0x0,
      [ADDRMODE_ZEROPAGE]    = 0x1,
      [ADDRMODE_ACCUMULATOR] = 0x2,
      [ADDRMODE_ABSOLUTE]    = 0x3,
      [ADDRMODE_ZEROPAGEX]   = 0x5,
      [ADDRMODE_ABSOLUTEX]   = 0x7,
   },
   [INSTRCODE_GROUP3] =
   {
      [ADDRMODE_IMMEDIATE] = 0x0,
      [ADDRMODE_ZEROPAGE]  = 0x1,
      [ADDRMODE_ABSOLUTE]  = 0x3,
      [ADDRMODE_ZEROPAGEX] = 0x5,
      [ADDRMODE_ABSOLUTEX] = 0x7,
   },
   [INSTRCODE_BRANCH] = {0},
};

static machine_instruction Encode(opcode_pattern Opcode, string Operands, instruction_group_code Group_Code)
{
   // Group 1 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 01)
   // Group 2 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 10)
   // Group 3 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 00)
   // Branch layout:  xxy10000 (xx = FLAG, y = TAKEN)

   // NOTE: Instruction jsr is a special case that doesn't follow the other
   // group encoding patterns, so we just use opcode directly.

   bool Is_Branch = (Group_Code == INSTRCODE_BRANCH);
   bool Is_Jump = (Group_Code == INSTRCODE_GROUP3 && Opcode == OPCODE_JMP);

   instruction_data Data = Parse_Operands(Operands, Is_Branch, Is_Jump);
   u8 Addressing_Code = Addressing_Mode_Patterns[Group_Code][Data.Addressing_Mode];
   // printf("%-20s | ", Addressing_Mode_Names[Data.Addressing_Mode]);

   machine_instruction Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = (Is_Branch || Group_Code == INSTRCODE_JSR)
      ? Opcode
      : (Opcode << 5) | (Addressing_Code << 2) | Group_Code;
   Result.Bytes[1] = Data.Bytes[0];
   Result.Bytes[2] = Data.Bytes[1];
   Result.Label_Operand = Data.Label_Operand;

   return(Result);
}

static GENERATE_MACHINE_INSTRUCTION(Generate_Machine_Instruction)
{
   machine_instruction Result = {0};

   // TODO: Collapse all this down into a hash table or something.
   // TODO: Support other whitespace characters.
   cut Instruction_Operands = Cut(Instruction, ' ');
   if(Instruction_Operands.Found)
   {
      string Mnemonic = Instruction_Operands.Before;
      string Operands = Trim_Left(Instruction_Operands.After);

      // Group 1 instructions.
      if     (Equals(Mnemonic, S("ora"))) Result = Encode(OPCODE_ORA, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("and"))) Result = Encode(OPCODE_AND, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("eor"))) Result = Encode(OPCODE_EOR, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("adc"))) Result = Encode(OPCODE_ADC, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("sta"))) Result = Encode(OPCODE_STA, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("lda"))) Result = Encode(OPCODE_LDA, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("cmp"))) Result = Encode(OPCODE_CMP, Operands, INSTRCODE_GROUP1);
      else if(Equals(Mnemonic, S("sbc"))) Result = Encode(OPCODE_SBC, Operands, INSTRCODE_GROUP1);

      // Group 2 instructions.
      else if(Equals(Mnemonic, S("asl"))) Result = Encode(OPCODE_ASL, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("rol"))) Result = Encode(OPCODE_ROL, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("lsr"))) Result = Encode(OPCODE_LSR, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("ror"))) Result = Encode(OPCODE_ROR, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("stx"))) Result = Encode(OPCODE_STX, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("ldx"))) Result = Encode(OPCODE_LDX, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("dec"))) Result = Encode(OPCODE_DEC, Operands, INSTRCODE_GROUP2);
      else if(Equals(Mnemonic, S("inc"))) Result = Encode(OPCODE_INC, Operands, INSTRCODE_GROUP2);

      // Group 3 instructions.
      else if(Equals(Mnemonic, S("bit"))) Result = Encode(OPCODE_BIT, Operands, INSTRCODE_GROUP3);
      else if(Equals(Mnemonic, S("jmp"))) Result = Encode(OPCODE_JMP, Operands, INSTRCODE_GROUP3);
      else if(Equals(Mnemonic, S("sty"))) Result = Encode(OPCODE_STY, Operands, INSTRCODE_GROUP3);
      else if(Equals(Mnemonic, S("ldy"))) Result = Encode(OPCODE_LDY, Operands, INSTRCODE_GROUP3);
      else if(Equals(Mnemonic, S("cpy"))) Result = Encode(OPCODE_CPY, Operands, INSTRCODE_GROUP3);
      else if(Equals(Mnemonic, S("cpx"))) Result = Encode(OPCODE_CPX, Operands, INSTRCODE_GROUP3);

      // Branch instructions.
      else if(Equals(Mnemonic, S("bpl"))) Result = Encode(OPCODE_BPL, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bmi"))) Result = Encode(OPCODE_BMI, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bvc"))) Result = Encode(OPCODE_BVC, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bvs"))) Result = Encode(OPCODE_BVS, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bcc"))) Result = Encode(OPCODE_BCC, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bcs"))) Result = Encode(OPCODE_BCS, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("bne"))) Result = Encode(OPCODE_BNE, Operands, INSTRCODE_BRANCH);
      else if(Equals(Mnemonic, S("beq"))) Result = Encode(OPCODE_BEQ, Operands, INSTRCODE_BRANCH);

      else if(Equals(Mnemonic, S("jsr"))) Result = Encode(OPCODE_JSR, Operands, INSTRCODE_JSR);
   }
   else
   {
      // Single-byte instructions.
      if     (Equals(Instruction, S("brk"))) Result.Bytes[Result.Length++] = 0x00;
      else if(Equals(Instruction, S("rti"))) Result.Bytes[Result.Length++] = 0x40;
      else if(Equals(Instruction, S("rts"))) Result.Bytes[Result.Length++] = 0x60;

      else if(Equals(Instruction, S("php"))) Result.Bytes[Result.Length++] = 0x08;
      else if(Equals(Instruction, S("plp"))) Result.Bytes[Result.Length++] = 0x28;
      else if(Equals(Instruction, S("pha"))) Result.Bytes[Result.Length++] = 0x48;
      else if(Equals(Instruction, S("pla"))) Result.Bytes[Result.Length++] = 0x68;
      else if(Equals(Instruction, S("dey"))) Result.Bytes[Result.Length++] = 0x88;
      else if(Equals(Instruction, S("tay"))) Result.Bytes[Result.Length++] = 0xA8;
      else if(Equals(Instruction, S("iny"))) Result.Bytes[Result.Length++] = 0xC8;
      else if(Equals(Instruction, S("inx"))) Result.Bytes[Result.Length++] = 0xE8;

      else if(Equals(Instruction, S("clc"))) Result.Bytes[Result.Length++] = 0x18;
      else if(Equals(Instruction, S("sec"))) Result.Bytes[Result.Length++] = 0x38;
      else if(Equals(Instruction, S("cli"))) Result.Bytes[Result.Length++] = 0x58;
      else if(Equals(Instruction, S("sei"))) Result.Bytes[Result.Length++] = 0x78;
      else if(Equals(Instruction, S("tya"))) Result.Bytes[Result.Length++] = 0x98;
      else if(Equals(Instruction, S("clv"))) Result.Bytes[Result.Length++] = 0xB8;
      else if(Equals(Instruction, S("cld"))) Result.Bytes[Result.Length++] = 0xD8;
      else if(Equals(Instruction, S("sed"))) Result.Bytes[Result.Length++] = 0xF8;

      else if(Equals(Instruction, S("txa"))) Result.Bytes[Result.Length++] = 0x8A;
      else if(Equals(Instruction, S("txs"))) Result.Bytes[Result.Length++] = 0x9A;
      else if(Equals(Instruction, S("tax"))) Result.Bytes[Result.Length++] = 0xAA;
      else if(Equals(Instruction, S("tsx"))) Result.Bytes[Result.Length++] = 0xBA;
      else if(Equals(Instruction, S("dex"))) Result.Bytes[Result.Length++] = 0xCA;
      else if(Equals(Instruction, S("nop"))) Result.Bytes[Result.Length++] = 0xEA;

      // printf("%-20s | ", Addressing_Mode_Names[ADDRMODE_IMPLIED]);
   }

   return(Result);
}

static PATCH_LABEL_ADDRESS(Patch_Label_Address)
{
   if(Result->Length == 2)
   {
      // Relative address.
      Result->Bytes[1] = (u8)(Label_Address - Instruction_Address);
   }
   else
   {
      // Absolute address.
      assert(Result->Length == 3);
      Result->Bytes[1] = (u8)(Label_Address >> 0);
      Result->Bytes[2] = (u8)(Label_Address >> 8);
   }
}
