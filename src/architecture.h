/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

// NOTE: This header file specifies the API all supported instruction sets must
// implement.

typedef struct machine_code_patch machine_code_patch;
struct machine_code_patch
{
   string Label;
   index Offset;
   index Length;
   machine_code_patch *Next;
};

typedef struct {
   index Address;
   index Length;
   union
   {
      u8 Bytes[16];
      u8 *Bytes_Pointer;
   };
   machine_code_patch *Patches;
} machine_code;

typedef struct {
   string Label;
   string Instruction;
   string Directive;
   int Line_Number;

   machine_code Machine_Code;
} source_code_line;

typedef struct assembler_context assembler_context;
struct assembler_context
{
   arena Arena;

   string Input_File_Path;
   string Output_File_Name;
   map *Constants;

   index Current_Address;
   int Current_Line_Number;
};

static void Request_Patch(arena *Arena, machine_code *Machine_Code,
                          string Label, index Offset, index Length)
{
   machine_code_patch *Patch = Allocate(Arena, machine_code_patch, 1);
   Patch->Label = Label;
   Patch->Offset = Offset;
   Patch->Length = Length;
   Patch->Next = Machine_Code->Patches;

   Machine_Code->Patches = Patch;
}

static void Apply_Patches(assembler_context *Context, machine_code *Machine_Code)
{
   for(machine_code_patch *Patch = Machine_Code->Patches; Patch; Patch = Patch->Next)
   {
      lookup_result Constant = Lookup(Context->Constants, Patch->Label);
      if(Constant.Found)
      {
         assert(Patch->Length <= Machine_Code->Length);

         u8 *Destination = (Machine_Code->Length > Array_Count(Machine_Code->Bytes))
            ? Machine_Code->Bytes_Pointer
            : Machine_Code->Bytes;

         // TODO: Endianess.
         for(index Byte_Index = 0; Byte_Index < Patch->Length; ++Byte_Index)
         {
            index Address = Patch->Offset + Byte_Index;
            u8 Value = (u8)(Constant.Value >> (Byte_Index * 8));

            printf("%zu <- %x\n", Address, Value);

            Destination[Address] = Value;
         }
      }
      else
      {
         Report_Error(Context, "Failed to resolve \"%.*s\".", SF(Patch->Label));
      }
   }
}

#define INITIALIZE_ARCHITECTURE(Name) void Name(assembler_context *Context)
static INITIALIZE_ARCHITECTURE(Initialize_Architecture);

#define ENCODE_INSTRUCTION(Name) machine_code Name(assembler_context *Context, string Instruction)
static ENCODE_INSTRUCTION(Encode_Instruction);
