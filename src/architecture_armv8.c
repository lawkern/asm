/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

static INITIALIZE_ARCHITECTURE(Initialize_Architecture)
{
   (void)Context;
}

static ENCODE_INSTRUCTION(Encode_Instruction)
{
   (void)Context;
   (void)Instruction;

   machine_code Result = {0};
   return(Result);
}

static PATCH_INSTRUCTION(Patch_Instruction)
{
   (void)Result;
   (void)Instruction_Address;
   (void)Label_Address;
}
