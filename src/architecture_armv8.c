static GENERATE_MACHINE_INSTRUCTION(Generate_Machine_Instruction)
{
   (void)Instruction;

   machine_code Result = {0};
   return(Result);
}

static PATCH_LABEL_ADDRESS(Patch_Label_Address)
{
   (void)Result;
   (void)Instruction_Address;
   (void)Label_Address;
}
