/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

typedef enum {
   CONDITION_CODE_EQ    = 0x0, // Equal (Z set)
   CONDITION_CODE_NE    = 0x1, // Not equal (Z clear)
   CONDITION_CODE_CS_HS = 0x2, // Carry set/unsigned higher or same (C set)
   CONDITION_CODE_CC_LO = 0x3, // Carry clear/unsigned lower (C clear)
   CONDITION_CODE_MI    = 0x4, // Minus/negative (N set)
   CONDITION_CODE_PL    = 0x5, // Plus/positive or zero (N clear)
   CONDITION_CODE_VS    = 0x6, // Overflow (V set)
   CONDITION_CODE_VC    = 0x7, // No overflow (V clear)
   CONDITION_CODE_HI    = 0x8, // Unsigned higher (C set and Z clear)
   CONDITION_CODE_LS    = 0x9, // Unsigned lower or same (C clear or Z set)
   CONDITION_CODE_GE    = 0xA, // Signed greater than or equal (N == V)
   CONDITION_CODE_LT    = 0xB, // Signed less than (N != V)
   CONDITION_CODE_GT    = 0xC, // Signed greater than (Z == 0, N == V)
   CONDITION_CODE_LE    = 0xD, // Signed less than or equal (Z == 1 or Z != V)
   CONDITION_CODE_AL    = 0xE, // Always (unconditional)

   CONDITION_CODE_UNDEFINED = 0xF, // Unpredictable instruction for ARMv4
} condition_code;

typedef enum {
   // Data-processing Instructions
   OPCODE_AND = 0x0, // Logical AND
   OPCODE_EOR = 0x1, // Logical Exclusiv OR
   OPCODE_SUB = 0x2, // Subtract
   OPCODE_RSB = 0x3, // Reverse Subtract
   OPCODE_ADD = 0x4, // Add
   OPCODE_ADC = 0x5, // Add with Carry
   OPCODE_SBC = 0x6, // Subtract with Carry
   OPCODE_RSC = 0x7, // Reverse Subtract with Carry
   OPCODE_TST = 0x8, // Test
   OPCODE_TEQ = 0x9, // Test Equivalence
   OPCODE_CMP = 0xA, // Compare
   OPCODE_CMN = 0xB, // Compare Negated
   OPCODE_ORR = 0xC, // Logical (inclusive) OR
   OPCODE_MOV = 0xD, // Move
   OPCODE_BIC = 0xE, // Bit Clear
   OPCODE_MVN = 0xF, // Move Not
} opcode;

typedef struct {
   condition_code Condition;
   opcode Opcode;
} instruction;

static void Generate_Instruction(asm_line Line)
{
   cut Operands = Cut(Line.Instruction, ' ');
   if(Operands.Found)
   {
      string Mnemonic = Operands.Before;

      if(Equals(Mnemonic, S("mov")))
      {
      }
      else if(Equals(Mnemonic, S("add")))
      {
      }
   }
   else
   {
   }

   // if(Line.Label.Length)
   // {
   //    printf("%.*s ", Line.Label.Length, Line.Label.Data);
   // }
   // printf("%.*s", Line.Instruction.Length, Line.Instruction.Data);
}
