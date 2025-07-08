/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

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
#  define X(M) MNEMONIC_##M,
   MNEMONICS_LIST
#  undef X
   MNEMONIC_COUNT,
};

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

typedef struct {
   u8 Opcode;
   u8 Encoding_Length;
} opcode_data;

static opcode_data Encoding_Table[][ADDRMODE_COUNT] =
{
   // Group 1 Instructions
   [MNEMONIC_ora] =
   {
      [ADDRMODE_IMMEDIATE] = {0x09, 2},
      [ADDRMODE_ZEROPAGE]  = {0x05, 2},
      [ADDRMODE_ZEROPAGEX] = {0x15, 2},
      [ADDRMODE_ABSOLUTE]  = {0x0D, 3},
      [ADDRMODE_ABSOLUTEX] = {0x1D, 3},
      [ADDRMODE_ABSOLUTEY] = {0x19, 3},
      [ADDRMODE_INDIRECTX] = {0x01, 2},
      [ADDRMODE_INDIRECTY] = {0x11, 2},
   },
   [MNEMONIC_and] =
   {
      [ADDRMODE_IMMEDIATE] = {0x29, 2},
      [ADDRMODE_ZEROPAGE]  = {0x25, 2},
      [ADDRMODE_ZEROPAGEX] = {0x35, 2},
      [ADDRMODE_ABSOLUTE]  = {0x2D, 3},
      [ADDRMODE_ABSOLUTEX] = {0x3D, 3},
      [ADDRMODE_ABSOLUTEY] = {0x39, 3},
      [ADDRMODE_INDIRECTX] = {0x21, 2},
      [ADDRMODE_INDIRECTY] = {0x31, 2},
   },
   [MNEMONIC_eor] =
   {
      [ADDRMODE_IMMEDIATE] = {0x49, 2},
      [ADDRMODE_ZEROPAGE]  = {0x45, 2},
      [ADDRMODE_ZEROPAGEX] = {0x55, 2},
      [ADDRMODE_ABSOLUTE]  = {0x4D, 3},
      [ADDRMODE_ABSOLUTEX] = {0x5D, 3},
      [ADDRMODE_ABSOLUTEY] = {0x59, 3},
      [ADDRMODE_INDIRECTX] = {0x41, 2},
      [ADDRMODE_INDIRECTY] = {0x51, 2},
   },
   [MNEMONIC_adc] =
   {
      [ADDRMODE_IMMEDIATE] = {0x69, 2},
      [ADDRMODE_ZEROPAGE]  = {0x65, 2},
      [ADDRMODE_ZEROPAGEX] = {0x75, 2},
      [ADDRMODE_ABSOLUTE]  = {0x6D, 3},
      [ADDRMODE_ABSOLUTEX] = {0x7D, 3},
      [ADDRMODE_ABSOLUTEY] = {0x79, 3},
      [ADDRMODE_INDIRECTX] = {0x61, 2},
      [ADDRMODE_INDIRECTY] = {0x71, 2},
   },
   [MNEMONIC_sta] =
   {
      [ADDRMODE_ZEROPAGE]  = {0x85, 2},
      [ADDRMODE_ZEROPAGEX] = {0x95, 2},
      [ADDRMODE_ABSOLUTE]  = {0x8D, 3},
      [ADDRMODE_ABSOLUTEX] = {0x9D, 3},
      [ADDRMODE_ABSOLUTEY] = {0x99, 3},
      [ADDRMODE_INDIRECTX] = {0x81, 2},
      [ADDRMODE_INDIRECTY] = {0x91, 2},
   },
   [MNEMONIC_lda] =
   {
      [ADDRMODE_IMMEDIATE] = {0xA9, 2},
      [ADDRMODE_ZEROPAGE]  = {0xA5, 2},
      [ADDRMODE_ZEROPAGEX] = {0xB5, 2},
      [ADDRMODE_ABSOLUTE]  = {0xAD, 3},
      [ADDRMODE_ABSOLUTEX] = {0xBD, 3},
      [ADDRMODE_ABSOLUTEY] = {0xB9, 3},
      [ADDRMODE_INDIRECTX] = {0xA1, 2},
      [ADDRMODE_INDIRECTY] = {0xB1, 2},
   },
   [MNEMONIC_cmp] =
   {
      [ADDRMODE_IMMEDIATE] = {0xC9, 2},
      [ADDRMODE_ZEROPAGE]  = {0xC5, 2},
      [ADDRMODE_ZEROPAGEX] = {0xD5, 2},
      [ADDRMODE_ABSOLUTE]  = {0xCD, 3},
      [ADDRMODE_ABSOLUTEX] = {0xDD, 3},
      [ADDRMODE_ABSOLUTEY] = {0xD9, 3},
      [ADDRMODE_INDIRECTX] = {0xC1, 2},
      [ADDRMODE_INDIRECTY] = {0xD1, 2},
   },
   [MNEMONIC_sbc] = {
      [ADDRMODE_IMMEDIATE] = {0xE9, 2},
      [ADDRMODE_ZEROPAGE]  = {0xE5, 2},
      [ADDRMODE_ZEROPAGEX] = {0xF5, 2},
      [ADDRMODE_ABSOLUTE]  = {0xED, 3},
      [ADDRMODE_ABSOLUTEX] = {0xFD, 3},
      [ADDRMODE_ABSOLUTEY] = {0xF9, 3},
      [ADDRMODE_INDIRECTX] = {0xE1, 2},
      [ADDRMODE_INDIRECTY] = {0xF1, 2},
   },

   // Group 2 Instructions
   [MNEMONIC_asl] =
   {
      [ADDRMODE_ACCUMULATOR] = {0x0A, 1},
      [ADDRMODE_ZEROPAGE]    = {0x06, 2},
      [ADDRMODE_ZEROPAGEX]   = {0x16, 2},
      [ADDRMODE_ABSOLUTE]    = {0x0E, 3},
      [ADDRMODE_ABSOLUTEX]   = {0x1E, 3},
   },
   [MNEMONIC_rol] =
   {
      [ADDRMODE_ACCUMULATOR] = {0x2A, 1},
      [ADDRMODE_ZEROPAGE]    = {0x26, 2},
      [ADDRMODE_ZEROPAGEX]   = {0x36, 2},
      [ADDRMODE_ABSOLUTE]    = {0x2E, 3},
      [ADDRMODE_ABSOLUTEX]   = {0x3E, 3},
   },
   [MNEMONIC_lsr] =
   {
      [ADDRMODE_ACCUMULATOR] = {0x4A, 1},
      [ADDRMODE_ZEROPAGE]    = {0x46, 2},
      [ADDRMODE_ZEROPAGEX]   = {0x56, 2},
      [ADDRMODE_ABSOLUTE]    = {0x4E, 3},
      [ADDRMODE_ABSOLUTEX]   = {0x5E, 3},
   },
   [MNEMONIC_ror] =
   {
      [ADDRMODE_ACCUMULATOR] = {0x6A, 1},
      [ADDRMODE_ZEROPAGE]    = {0x66, 2},
      [ADDRMODE_ZEROPAGEX]   = {0x76, 2},
      [ADDRMODE_ABSOLUTE]    = {0x6E, 3},
      [ADDRMODE_ABSOLUTEX]   = {0x7E, 3},
   },
   [MNEMONIC_stx] =
   {
      [ADDRMODE_ZEROPAGE]  = {0x86, 2},
      [ADDRMODE_ZEROPAGEY] = {0x96, 2},
      [ADDRMODE_ABSOLUTE]  = {0x8E, 3},
   },
   [MNEMONIC_ldx] =
   {
      [ADDRMODE_IMMEDIATE] = {0xA2, 2},
      [ADDRMODE_ZEROPAGE]  = {0xA6, 2},
      [ADDRMODE_ZEROPAGEY] = {0xB6, 2},
      [ADDRMODE_ABSOLUTE]  = {0xAE, 3},
      [ADDRMODE_ABSOLUTEY] = {0xBE, 3},
   },
   [MNEMONIC_dec] =
   {
      [ADDRMODE_ZEROPAGE]  = {0xC6, 2},
      [ADDRMODE_ZEROPAGEX] = {0xD6, 2},
      [ADDRMODE_ABSOLUTE]  = {0xCE, 3},
      [ADDRMODE_ABSOLUTEX] = {0xDE, 3},
   },
   [MNEMONIC_inc] =
   {
      [ADDRMODE_ZEROPAGE]  = {0xE6, 2},
      [ADDRMODE_ZEROPAGEX] = {0xF6, 2},
      [ADDRMODE_ABSOLUTE]  = {0xEE, 3},
      [ADDRMODE_ABSOLUTEX] = {0xFE, 3},
   },

   // Group 3
   [MNEMONIC_bit] =
   {
      [ADDRMODE_ZEROPAGE] = {0x24, 2},
      [ADDRMODE_ABSOLUTE] = {0x2C, 3},
   },
   [MNEMONIC_jmp] =
   {
      [ADDRMODE_ABSOLUTE] = {0x4C, 3},
      [ADDRMODE_INDIRECT] = {0x6C, 3},
   },
   [MNEMONIC_sty] =
   {
      [ADDRMODE_ZEROPAGE]  = {0x84, 2},
      [ADDRMODE_ZEROPAGEX] = {0x94, 2},
      [ADDRMODE_ABSOLUTE]  = {0x8C, 3},
   },
   [MNEMONIC_ldy] =
   {
      [ADDRMODE_IMMEDIATE] = {0xA0, 2},
      [ADDRMODE_ZEROPAGE]  = {0xA4, 2},
      [ADDRMODE_ZEROPAGEX] = {0xB4, 2},
      [ADDRMODE_ABSOLUTE]  = {0xAC, 3},
      [ADDRMODE_ABSOLUTEX] = {0xBC, 3},
   },
   [MNEMONIC_cpy] =
   {
      [ADDRMODE_IMMEDIATE] = {0xC0, 2},
      [ADDRMODE_ZEROPAGE]  = {0xC4, 2},
      [ADDRMODE_ABSOLUTE]  = {0xCC, 3},
   },
   [MNEMONIC_cpx] =
   {
      [ADDRMODE_IMMEDIATE] = {0xE0, 2},
      [ADDRMODE_ZEROPAGE]  = {0xE4, 2},
      [ADDRMODE_ABSOLUTE]  = {0xEC, 3},
   },

   // // Branches
   [MNEMONIC_bpl] = {[ADDRMODE_RELATIVE] = {0x10, 2}},
   [MNEMONIC_bmi] = {[ADDRMODE_RELATIVE] = {0x30, 2}},
   [MNEMONIC_bvc] = {[ADDRMODE_RELATIVE] = {0x50, 2}},
   [MNEMONIC_bvs] = {[ADDRMODE_RELATIVE] = {0x70, 2}},
   [MNEMONIC_bcc] = {[ADDRMODE_RELATIVE] = {0x90, 2}},
   [MNEMONIC_bcs] = {[ADDRMODE_RELATIVE] = {0xB0, 2}},
   [MNEMONIC_bne] = {[ADDRMODE_RELATIVE] = {0xD0, 2}},
   [MNEMONIC_beq] = {[ADDRMODE_RELATIVE] = {0xF0, 2}},

   [MNEMONIC_jsr] = {[ADDRMODE_ABSOLUTE] = {0x20, 3}},

   // Single-byte
   [MNEMONIC_brk] = {[ADDRMODE_IMPLIED] = {0x00, 1}},
   [MNEMONIC_rti] = {[ADDRMODE_IMPLIED] = {0x40, 1}},
   [MNEMONIC_rts] = {[ADDRMODE_IMPLIED] = {0x60, 1}},

   [MNEMONIC_php] = {[ADDRMODE_IMPLIED] = {0x08, 1}},
   [MNEMONIC_plp] = {[ADDRMODE_IMPLIED] = {0x28, 1}},
   [MNEMONIC_pha] = {[ADDRMODE_IMPLIED] = {0x48, 1}},
   [MNEMONIC_pla] = {[ADDRMODE_IMPLIED] = {0x68, 1}},
   [MNEMONIC_dey] = {[ADDRMODE_IMPLIED] = {0x88, 1}},
   [MNEMONIC_tay] = {[ADDRMODE_IMPLIED] = {0xA8, 1}},
   [MNEMONIC_iny] = {[ADDRMODE_IMPLIED] = {0xC8, 1}},
   [MNEMONIC_inx] = {[ADDRMODE_IMPLIED] = {0xE8, 1}},

   [MNEMONIC_clc] = {[ADDRMODE_IMPLIED] = {0x18, 1}},
   [MNEMONIC_sec] = {[ADDRMODE_IMPLIED] = {0x38, 1}},
   [MNEMONIC_cli] = {[ADDRMODE_IMPLIED] = {0x58, 1}},
   [MNEMONIC_sei] = {[ADDRMODE_IMPLIED] = {0x78, 1}},
   [MNEMONIC_tya] = {[ADDRMODE_IMPLIED] = {0x98, 1}},
   [MNEMONIC_clv] = {[ADDRMODE_IMPLIED] = {0xB8, 1}},
   [MNEMONIC_cld] = {[ADDRMODE_IMPLIED] = {0xD8, 1}},
   [MNEMONIC_sed] = {[ADDRMODE_IMPLIED] = {0xF8, 1}},

   [MNEMONIC_txa] = {[ADDRMODE_IMPLIED] = {0x8A, 1}},
   [MNEMONIC_txs] = {[ADDRMODE_IMPLIED] = {0x9A, 1}},
   [MNEMONIC_tax] = {[ADDRMODE_IMPLIED] = {0xAA, 1}},
   [MNEMONIC_tsx] = {[ADDRMODE_IMPLIED] = {0xBA, 1}},
   [MNEMONIC_dex] = {[ADDRMODE_IMPLIED] = {0xCA, 1}},
   [MNEMONIC_nop] = {[ADDRMODE_IMPLIED] = {0xEA, 1}},
};

typedef struct {
   string Unresolved_Label;
   index Length;
   s16 Value;
} parsed_operand_data;

typedef struct {
   addressing_mode Addressing_Mode;
   parsed_operand_data Data;
} parsed_operand;

static index Required_Byte_Count(opcode_data *Addressing_Modes, s16 Value)
{
   bool Requires_Two_Bytes = (Value > 127 || Value < -128 ||
                  !(Addressing_Modes[ADDRMODE_IMMEDIATE].Encoding_Length ||
                    Addressing_Modes[ADDRMODE_RELATIVE].Encoding_Length ||
                    Addressing_Modes[ADDRMODE_ZEROPAGE].Encoding_Length ||
                    Addressing_Modes[ADDRMODE_ZEROPAGEX].Encoding_Length ||
                    Addressing_Modes[ADDRMODE_ZEROPAGEY].Encoding_Length));

   index Result = (Requires_Two_Bytes) ? 2 : 1;
   return(Result);
}

static parsed_operand_data Parse_Operand_Data(assembler_context *Context, string String, opcode_data *Addressing_Modes, map *Constants)
{
   parsed_operand_data Result = {0};

   if(String.Length && String.Data[0] >= '0' && String.Data[0] <= '9')
   {
      // NOTE: A number literal will be encoded in either one or two bytes. Jump
      // instructions (jmp and jsr) will always use two bytes.
      parsed_integer Parsed_Number = Parse_Integer(String);
      if(Parsed_Number.Ok)
      {
         // TODO: Report overflow.
         Result.Value = (s16)Parsed_Number.Value;
         Result.Length = Required_Byte_Count(Addressing_Modes, Result.Value);
      }
      else
      {
         Report_Error(Context, "Could not parse number literal \"%.*s\".", SF(String));
      }
   }
   else
   {
      lookup_result Constant = Lookup(Constants, String);
      if(Constant.Found)
      {
         // TODO: Report overflow.
         Result.Value = (s16)Constant.Value;
         Result.Length = Required_Byte_Count(Addressing_Modes, Result.Value);
      }
      else
      {
         // Label (to be populated later)
         Result.Unresolved_Label = String;
      }
   }


   return(Result);
}

static parsed_operand Parse_Operand(assembler_context *Context, string Operand, opcode_data *Addressing_Modes, map *Constants)
{
   parsed_operand Result = {0};

   string Full_Operand = Operand;
   parsed_operand_data Data = {0};
   addressing_mode Addressing_Mode = 0;

   if(Operand.Length == 0)
   {
      Addressing_Mode = ADDRMODE_IMPLIED;
   }
   else if(Equals(Operand, S("a")))
   {
      Addressing_Mode = ADDRMODE_ACCUMULATOR;
   }
   else if(Has_Prefix_Then_Remove(&Operand, S("[[")))
   {
      if(Has_Suffix_Then_Remove(&Operand, S(" + x]]")))
      {
         Addressing_Mode = ADDRMODE_INDIRECTX;
         Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
      }
      else if(Has_Suffix_Then_Remove(&Operand, S("] + y]")))
      {
         Addressing_Mode = ADDRMODE_INDIRECTY;
         Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
      }
      else
      {
         Report_Error(Context, "Unterminated double bracket in \"%.*s\".", SF(Full_Operand));
      }
   }
   else if(Has_Prefix_Then_Remove(&Operand, S("[")))
   {
      if(Has_Suffix_Then_Remove(&Operand, S(" + x]")))
      {
         Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
         Addressing_Mode = (Data.Length == 1)
            ? ADDRMODE_ZEROPAGEX
            : ADDRMODE_ABSOLUTEX;
      }
      else if(Has_Suffix_Then_Remove(&Operand, S(" + y]")))
      {
         Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
         Addressing_Mode = (Data.Length == 1)
            ? ADDRMODE_ZEROPAGEY
            : ADDRMODE_ABSOLUTEY;
      }
      else if(Has_Suffix_Then_Remove(&Operand, S("]")))
      {
         Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
         Addressing_Mode = (Data.Length == 1)
            ? ADDRMODE_ZEROPAGE
            : (Addressing_Modes[ADDRMODE_INDIRECT].Encoding_Length) ? ADDRMODE_INDIRECT : ADDRMODE_ABSOLUTE;
      }
      else
      {
         Report_Error(Context, "Unterminated bracket in \"%.*s\".", SF(Full_Operand));
      }
   }
   else if(Operand.Data[0] >= '0' && Operand.Data[0] <= '9')
   {
      Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
      Addressing_Mode = (Data.Length == 1)
         ? (Addressing_Modes[ADDRMODE_RELATIVE].Encoding_Length) ? ADDRMODE_RELATIVE : ADDRMODE_IMMEDIATE
         : ADDRMODE_ABSOLUTE;
   }
   else // Label/Constant
   {
      Data = Parse_Operand_Data(Context, Operand, Addressing_Modes, Constants);
      Addressing_Mode = (Addressing_Modes[ADDRMODE_RELATIVE].Encoding_Length) ? ADDRMODE_RELATIVE : ADDRMODE_ABSOLUTE;
   }

   if(Addressing_Modes[Addressing_Mode].Encoding_Length)
   {
      Result.Addressing_Mode = Addressing_Mode;
      Result.Data = Data;
   }
   else
   {
      Report_Error(Context, "Unsupported addressing mode \"%.*s\".", SF(Full_Operand));
   }

   return(Result);
}

static map *Encoding_Map;

static INITIALIZE_ARCHITECTURE(Initialize_Architecture)
{
   assert(Array_Count(Encoding_Table) == MNEMONIC_COUNT);

#  define X(M) Insert(&Context->Arena, &Encoding_Map, S(#M), MNEMONIC_##M);
   MNEMONICS_LIST;
#  undef X
}

static ENCODE_INSTRUCTION(Encode_Instruction)
{
   machine_code Result = {0};

   cut Instruction_Operand = Cut_Whitespace(Instruction);
   string Mnemonic_String = Instruction_Operand.Before;

   lookup_result Mnemonic = Lookup(Encoding_Map, Mnemonic_String);
   if(Mnemonic.Found)
   {
      string Operand_String = Trim_Left(Instruction_Operand.After);
      opcode_data *Addressing_Modes = Encoding_Table[Mnemonic.Value];
      parsed_operand Operand = Parse_Operand(Context, Operand_String, Addressing_Modes, Context->Constants);
      if(Operand.Data.Unresolved_Label.Length)
      {
         Request_Patch(&Context->Arena, &Result, Operand.Data.Unresolved_Label, 1, Operand.Data.Length);
      }

      opcode_data Opcode_Data = Addressing_Modes[Operand.Addressing_Mode];

      Result.Length = Opcode_Data.Encoding_Length;
      Result.Bytes[0] = Opcode_Data.Opcode;
      if(Result.Length > 1) Result.Bytes[1] = (u8)(Operand.Data.Value << 0);
      if(Result.Length > 2) Result.Bytes[2] = (u8)(Operand.Data.Value << 8);
   }
   else
   {
      Report_Error(Context, "Did not recognize mnemonic \"%.*s\".", SF(Mnemonic_String));
   }

   return(Result);
}
