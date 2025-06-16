/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

static void Generate_Instruction(asm_line Line)
{
   cut Operands = Cut(Line.Instruction, ' ');
   string Mnemonic = Operands.Before;

   if(Operands.Found)
   {
      if(Equals(Mnemonic, S("sta")))
      {
      }
      else if(Equals(Mnemonic, S("lda")))
      {
      }
   }
   else
   {
      // Must be a single-byte instruction.
      u8 Instruction_Byte = 0x00;

      // TODO: Collapse this down into a hash table or something.
      if(Equals(Line.Instruction, S("brk"))) Instruction_Byte = 0x00;
      else if(Equals(Line.Instruction, S("rti"))) Instruction_Byte = 0x40;
      else if(Equals(Line.Instruction, S("rts"))) Instruction_Byte = 0x60;

      else if(Equals(Line.Instruction, S("php"))) Instruction_Byte = 0x08;
      else if(Equals(Line.Instruction, S("plp"))) Instruction_Byte = 0x28;
      else if(Equals(Line.Instruction, S("pha"))) Instruction_Byte = 0x48;
      else if(Equals(Line.Instruction, S("pla"))) Instruction_Byte = 0x68;
      else if(Equals(Line.Instruction, S("dey"))) Instruction_Byte = 0x88;
      else if(Equals(Line.Instruction, S("tay"))) Instruction_Byte = 0xA8;
      else if(Equals(Line.Instruction, S("iny"))) Instruction_Byte = 0xC8;
      else if(Equals(Line.Instruction, S("inx"))) Instruction_Byte = 0xE8;

      else if(Equals(Line.Instruction, S("clc"))) Instruction_Byte = 0x18;
      else if(Equals(Line.Instruction, S("sec"))) Instruction_Byte = 0x38;
      else if(Equals(Line.Instruction, S("cli"))) Instruction_Byte = 0x58;
      else if(Equals(Line.Instruction, S("sei"))) Instruction_Byte = 0x78;
      else if(Equals(Line.Instruction, S("tya"))) Instruction_Byte = 0x98;
      else if(Equals(Line.Instruction, S("clv"))) Instruction_Byte = 0xB8;
      else if(Equals(Line.Instruction, S("cld"))) Instruction_Byte = 0xD8;
      else if(Equals(Line.Instruction, S("sed"))) Instruction_Byte = 0xF8;

      else if(Equals(Line.Instruction, S("txa"))) Instruction_Byte = 0x8A;
      else if(Equals(Line.Instruction, S("txs"))) Instruction_Byte = 0x9A;
      else if(Equals(Line.Instruction, S("tax"))) Instruction_Byte = 0xAA;
      else if(Equals(Line.Instruction, S("tsx"))) Instruction_Byte = 0xBA;
      else if(Equals(Line.Instruction, S("dex"))) Instruction_Byte = 0xCA;
      else if(Equals(Line.Instruction, S("nop"))) Instruction_Byte = 0xEA;

      else if(Equals(Line.Instruction, S("bpl"))) Instruction_Byte = 0x10;
      else if(Equals(Line.Instruction, S("bmi"))) Instruction_Byte = 0x30;
      else if(Equals(Line.Instruction, S("bvc"))) Instruction_Byte = 0x50;
      else if(Equals(Line.Instruction, S("bvs"))) Instruction_Byte = 0x70;
      else if(Equals(Line.Instruction, S("bcc"))) Instruction_Byte = 0x90;
      else if(Equals(Line.Instruction, S("bcs"))) Instruction_Byte = 0xB0;
      else if(Equals(Line.Instruction, S("bne"))) Instruction_Byte = 0xD0;
      else if(Equals(Line.Instruction, S("beq"))) Instruction_Byte = 0xF0;

      printf("%02x ", Instruction_Byte);
   }

   printf(" %.*s ", Mnemonic.Length, Mnemonic.Data);
}
