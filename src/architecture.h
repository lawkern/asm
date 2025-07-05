/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

// NOTE: This header file specifies the API all supported instruction sets must
// implement.

typedef struct {
   index Address;
   index Length;
   union
   {
      u8 Bytes[16];
      u8 *Bytes_Pointer;
   };
   string Label_Operand;
} machine_code;

typedef struct {
   string Label;
   string Instruction;
   string Directive;
   int Line_Number;

   machine_code Machine_Code;
} source_code_line;

#define INITIALIZE_ARCHITECTURE(Name) void Name(assembler_context *Context)
static INITIALIZE_ARCHITECTURE(Initialize_Architecture);

#define ENCODE_INSTRUCTION(Name) machine_code Name(assembler_context *Context, string Instruction)
static ENCODE_INSTRUCTION(Encode_Instruction);

#define PATCH_INSTRUCTION(Name) void Name(machine_code *Result, index Instruction_Address, index Label_Address)
static PATCH_INSTRUCTION(Patch_Instruction);
