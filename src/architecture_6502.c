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
} parsed_instruction_data;

static bool Parse_And_Populate_Data_Bytes(parsed_instruction_data *Result, string Operands, bool Is_Branch, bool Is_Jump)
{
   bool Ok = false;

   if(Operands.Length && Operands.Data[0] >= '0' && Operands.Data[0] <= '9')
   {
      // Number literal
      parsed_integer Parsed_Number = Parse_Integer(Operands);
      if(Parsed_Number.Ok)
      {
         // TODO: Negative values?
         Result->Length = (Is_Jump || Parsed_Number.Value > 0xFF) ? 2 : 1;
         Result->Bytes[0] = (u8)Parsed_Number.Value;
         Result->Bytes[1] = (u8)(Parsed_Number.Value >> 8);
         Ok = true;
      }
   }
   else
   {
      lookup_result Constant = Lookup(Context.Constants, Operands);
      if(Constant.Found)
      {
         // TODO: Negative values?
         Result->Length = (Is_Jump || Constant.Value > 0xFF) ? 2 : 1;
         Result->Bytes[0] = (u8)Constant.Value;
         Result->Bytes[1] = (u8)(Constant.Value >> 8);
      }
      else
      {
         // Label (to be populated later)
         Result->Length = (Is_Branch) ? 1 : 2;
         Result->Bytes[0] = 0xBE;
         Result->Bytes[1] = 0xEF;
         Result->Label_Operand = Operands;
      }
   }

   return(Ok);
}

static parsed_instruction_data Parse_Operands(string Operands, bool Is_Branch, bool Is_Jump)
{
   // TODO: Enforce syntax requirements.

   parsed_instruction_data Result = {0};

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
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("] + y]")))
      {
         Result.Addressing_Mode = ADDRMODE_INDIRECTY;
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
      }
   }
   else if(Has_Prefix_Then_Remove(&Operands, S("[")))
   {
      if(Has_Suffix_Then_Remove(&Operands, S(" + x]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEX : ADDRMODE_ZEROPAGEX;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S(" + y]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
         Result.Addressing_Mode = (Result.Length == 2) ? ADDRMODE_ABSOLUTEY : ADDRMODE_ZEROPAGEY;
      }
      else if(Has_Suffix_Then_Remove(&Operands, S("]")))
      {
         Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
         Result.Addressing_Mode = (Result.Length == 2)
            ? (Is_Jump) ? ADDRMODE_INDIRECT : ADDRMODE_ABSOLUTE
            : ADDRMODE_ZEROPAGE;
      }
   }
   else if(Operands.Data[0] >= '0' && Operands.Data[0] <= '9')
   {
      Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
      Result.Addressing_Mode = (Result.Length == 2)
         ? ADDRMODE_ABSOLUTE
         : (Is_Branch) ? ADDRMODE_RELATIVE : ADDRMODE_IMMEDIATE;
   }
   else // Label
   {
      Result.Ok = Parse_And_Populate_Data_Bytes(&Result, Operands, Is_Branch, Is_Jump);
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

static machine_code Encode(opcode_pattern Opcode, string Operands, instruction_group_code Group_Code)
{
   // Group 1 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 01)
   // Group 2 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 10)
   // Group 3 layout: aaabbbcc (aaa = OPCODE, bbb = ADDRMODE, cc = 00)
   // Branch layout:  xxy10000 (xx = FLAG, y = TAKEN)

   // NOTE: Instruction jsr is a special case that doesn't follow the other
   // group encoding patterns, so we just use opcode directly.

   bool Is_Branch = (Group_Code == INSTRCODE_BRANCH);
   bool Is_Jump = (Group_Code == INSTRCODE_GROUP3 && Opcode == OPCODE_JMP);

   parsed_instruction_data Data = Parse_Operands(Operands, Is_Branch, Is_Jump);
   u8 Addressing_Code = Addressing_Mode_Patterns[Group_Code][Data.Addressing_Mode];
   // printf("%-20s | ", Addressing_Mode_Names[Data.Addressing_Mode]);

   machine_code Result = {0};
   Result.Length = 1 + Data.Length;
   Result.Bytes[0] = (Is_Branch || Group_Code == INSTRCODE_JSR)
      ? Opcode
      : (Opcode << 5) | (Addressing_Code << 2) | Group_Code;
   Result.Bytes[1] = Data.Bytes[0];
   Result.Bytes[2] = Data.Bytes[1];
   Result.Label_Operand = Data.Label_Operand;

   return(Result);
}

#define MNEMONICS_LIST                          \
   X(ora) X(and) X(eor) X(adc)                  \
   X(sta) X(lda) X(cmp) X(sbc)                  \
                                                \
   X(asl) X(rol) X(lsr) X(ror)                  \
   X(stx) X(ldx) X(dec) X(inc)                  \
                                                \
   X(bit) X(jmp) X(sty) X(ldy)                  \
   X(cpy) X(cpx)                                \
                                                \
   X(bpl) X(bmi) X(bvc) X(bvs)                  \
   X(bcc) X(bcs) X(bne) X(beq)                  \
                                                \
   X(jsr)                                       \
                                                \
   X(brk) X(rti) X(rts) X(php)                  \
   X(plp) X(pha) X(pla) X(dey)                  \
   X(tay) X(iny) X(inx) X(clc)                  \
   X(sec) X(cli) X(sei) X(tya)                  \
   X(clv) X(cld) X(sed) X(txa)                  \
   X(txs) X(tax) X(tsx) X(dex)                  \
   X(nop)

enum
{
#define X(M) MNEMONIC_##M,
   MNEMONICS_LIST
#undef X
   MNEMONIC_COUNT,
};

typedef struct {
   // NOTE: Full value for non-group encodings.
   u8 Actual_Value;

   // NOTE: Bit patterns for group-based encodings.
   opcode_pattern Opcode;
   instruction_group_code Group_Code;
} encoding_data;

static encoding_data Encoding_Table[] =
{
   // Group 1
   [MNEMONIC_ora] = {0, OPCODE_ORA, INSTRCODE_GROUP1},
   [MNEMONIC_and] = {0, OPCODE_AND, INSTRCODE_GROUP1},
   [MNEMONIC_eor] = {0, OPCODE_EOR, INSTRCODE_GROUP1},
   [MNEMONIC_adc] = {0, OPCODE_ADC, INSTRCODE_GROUP1},
   [MNEMONIC_sta] = {0, OPCODE_STA, INSTRCODE_GROUP1},
   [MNEMONIC_lda] = {0, OPCODE_LDA, INSTRCODE_GROUP1},
   [MNEMONIC_cmp] = {0, OPCODE_CMP, INSTRCODE_GROUP1},
   [MNEMONIC_sbc] = {0, OPCODE_SBC, INSTRCODE_GROUP1},

   // Group 2
   [MNEMONIC_asl] = {0, OPCODE_ASL, INSTRCODE_GROUP2},
   [MNEMONIC_rol] = {0, OPCODE_ROL, INSTRCODE_GROUP2},
   [MNEMONIC_lsr] = {0, OPCODE_LSR, INSTRCODE_GROUP2},
   [MNEMONIC_ror] = {0, OPCODE_ROR, INSTRCODE_GROUP2},
   [MNEMONIC_stx] = {0, OPCODE_STX, INSTRCODE_GROUP2},
   [MNEMONIC_ldx] = {0, OPCODE_LDX, INSTRCODE_GROUP2},
   [MNEMONIC_dec] = {0, OPCODE_DEC, INSTRCODE_GROUP2},
   [MNEMONIC_inc] = {0, OPCODE_INC, INSTRCODE_GROUP2},

   // Group 3
   [MNEMONIC_bit] = {0, OPCODE_BIT, INSTRCODE_GROUP3},
   [MNEMONIC_jmp] = {0, OPCODE_JMP, INSTRCODE_GROUP3},
   [MNEMONIC_sty] = {0, OPCODE_STY, INSTRCODE_GROUP3},
   [MNEMONIC_ldy] = {0, OPCODE_LDY, INSTRCODE_GROUP3},
   [MNEMONIC_cpy] = {0, OPCODE_CPY, INSTRCODE_GROUP3},
   [MNEMONIC_cpx] = {0, OPCODE_CPX, INSTRCODE_GROUP3},

   // Branches
   [MNEMONIC_bpl] = {0, OPCODE_BPL, INSTRCODE_BRANCH},
   [MNEMONIC_bmi] = {0, OPCODE_BMI, INSTRCODE_BRANCH},
   [MNEMONIC_bvc] = {0, OPCODE_BVC, INSTRCODE_BRANCH},
   [MNEMONIC_bvs] = {0, OPCODE_BVS, INSTRCODE_BRANCH},
   [MNEMONIC_bcc] = {0, OPCODE_BCC, INSTRCODE_BRANCH},
   [MNEMONIC_bcs] = {0, OPCODE_BCS, INSTRCODE_BRANCH},
   [MNEMONIC_bne] = {0, OPCODE_BNE, INSTRCODE_BRANCH},
   [MNEMONIC_beq] = {0, OPCODE_BEQ, INSTRCODE_BRANCH},

   [MNEMONIC_jsr] = {0, OPCODE_JSR, INSTRCODE_JSR},

   // Single-byte
   [MNEMONIC_brk] = {0x00},
   [MNEMONIC_rti] = {0x40},
   [MNEMONIC_rts] = {0x60},

   [MNEMONIC_php] = {0x08},
   [MNEMONIC_plp] = {0x28},
   [MNEMONIC_pha] = {0x48},
   [MNEMONIC_pla] = {0x68},
   [MNEMONIC_dey] = {0x88},
   [MNEMONIC_tay] = {0xA8},
   [MNEMONIC_iny] = {0xC8},
   [MNEMONIC_inx] = {0xE8},

   [MNEMONIC_clc] = {0x18},
   [MNEMONIC_sec] = {0x38},
   [MNEMONIC_cli] = {0x58},
   [MNEMONIC_sei] = {0x78},
   [MNEMONIC_tya] = {0x98},
   [MNEMONIC_clv] = {0xB8},
   [MNEMONIC_cld] = {0xD8},
   [MNEMONIC_sed] = {0xF8},

   [MNEMONIC_txa] = {0x8A},
   [MNEMONIC_txs] = {0x9A},
   [MNEMONIC_tax] = {0xAA},
   [MNEMONIC_tsx] = {0xBA},
   [MNEMONIC_dex] = {0xCA},
   [MNEMONIC_nop] = {0xEA},
};

static map *Encoding_Map;

static GENERATE_MACHINE_INSTRUCTION(Generate_Machine_Instruction)
{
   machine_code Result = {0};

   if(!Encoding_Map)
   {
      assert(Array_Count(Encoding_Table) == MNEMONIC_COUNT);
#define X(M) Insert(&Context.Arena, &Encoding_Map, S(#M), MNEMONIC_##M);
      MNEMONICS_LIST;
#undef X
   }

   cut Instruction_Operands = Cut_Whitespace(Instruction);
   string Mnemonic = Instruction_Operands.Before;
   string Operands = Trim_Left(Instruction_Operands.After);

   lookup_result Mnemonic_Lookup = Lookup(Encoding_Map, Mnemonic);
   if(Mnemonic_Lookup.Found)
   {
      encoding_data Data = Encoding_Table[Mnemonic_Lookup.Value];
      if(Instruction_Operands.Found)
      {
         Result = Encode(Data.Opcode, Operands, Data.Group_Code);
      }
      else
      {
         Result.Bytes[Result.Length++] = Data.Actual_Value;
      }
   }
   else
   {
      Report_Error("Did not recognize mnemonic \"%.*s\".\n", (int)Mnemonic.Length, Mnemonic.Data);
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
