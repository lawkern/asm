/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void Report_Error(char *Message, ...)
{
   fprintf(stderr, "ERROR: ");

   va_list Arguments;
   va_start(Arguments, Message);
   vfprintf(stderr, Message, Arguments);
   va_end(Arguments);

   fprintf(stderr, "\n");
}

#include "memory.c"

typedef struct {
   string Label;
   index Address;
} label_address;

static int Label_Address_Count;
static label_address Label_Addresses[512];

typedef struct {
   int Length;
   u8 Bytes[16];

   string Label_Operand;
} machine_instruction;

#define GENERATE_MACHINE_INSTRUCTION(Name) machine_instruction Name(string Instruction)
#define PATCH_LABEL_ADDRESS(Name) void Name(machine_instruction *Result, index Instruction_Address, index Label_Address)

#if ARCH_6502
#   include "architecture_6502.c"
#elif ARCH_ARM4VT
#   include "architecture_armv4t.c"
#else
#   error Unhandled architecture.
#endif

typedef struct {
   char *Source_Code_Path;
   int Source_Code_Line_Number;

   string Label;
   string Instruction;
   string Directive;

   index Machine_Address;
   machine_instruction Machine_Instruction;
} source_code_line;

static void Print_Instruction(source_code_line *Line)
{
   printf("%s:%-4d | ", Line->Source_Code_Path, Line->Source_Code_Line_Number);

   if(Line->Machine_Instruction.Length)
   {
      printf("0x%04x: ", (u32)Line->Machine_Address);
      for(int Index = 0; Index < Line->Machine_Instruction.Length; ++Index)
      {
         printf("%02x ", Line->Machine_Instruction.Bytes[Index]);
      }
      for(int Index = 4; Index >= (Line->Machine_Instruction.Length); --Index)
      {
         printf("   ");
      }
   }
   else
   {
      printf("                       ");
   }
   printf(" | ");

   printf("%-20.*s | ", (int)Line->Instruction.Length, Line->Instruction.Data);

   if(Line->Label.Length)
   {
      printf("Label('%.*s') ", (int)Line->Label.Length, Line->Label.Data);
   }
   if(Line->Directive.Length)
   {
      printf("Directive('%.*s') ", (int)Line->Directive.Length, Line->Directive.Data);
   }

   printf("\n");
}

int main(int Argument_Count, char **Arguments)
{
   arena Arena = {0};
   Arena.Size = 1024 * 1024 * 1024;
   Arena.Base = malloc(Arena.Size);

   // NOTE: For now, assume that all provided arguments are input files.
   for(int Input_Index = 1; Input_Index < Argument_Count; ++Input_Index)
   {
      char *Source_Code_Path = Arguments[Input_Index];
      string Source_Code = Read_Entire_File(&Arena, Source_Code_Path);

      // First pass to determine the number of lines to allocate. This will
      // include any non-empty line of source code.

      int Line_Count = 0;
      cut Source_Cut = {0};
      Source_Cut.After = Source_Code;
      while(Source_Cut.After.Length)
      {
         Source_Cut = Cut(Source_Cut.After, '\n');
         cut Comment = Cut(Source_Cut.Before, '\\');
         Line_Count += (Trim(Comment.Before).Length > 0);
      }

      // Second pass to identify directives, labels and instructions for each
      // allocated line of assembly code.

      source_code_line *Lines = Allocate(&Arena, source_code_line, Line_Count);
      int Current_Line_Index = 0;  // Index of allocated source line to populate.
      int Source_Line_Number = 1;

      Source_Cut.After = Source_Code;
      while(Source_Cut.After.Length)
      {
         Source_Cut = Cut(Source_Cut.After, '\n');
         cut Comment = Cut(Source_Cut.Before, '\\');
         string Text = Trim(Comment.Before);

         source_code_line Line = {0};
         Line.Source_Code_Path = Source_Code_Path;
         Line.Source_Code_Line_Number = Source_Line_Number++;

         if(Text.Length > 0)
         {
            if(Has_Prefix_Then_Remove(&Text, S("#")))
            {
               // NOTE: If a line begins with '#', it's a directive that extends
               // to the end of the line, e.g. "#section text".
               Line.Directive = Trim_Left(Text);
            }
            else
            {
               // NOTE: Labels are required to end with a colon.
               cut Label = Cut(Text, ':');
               if(Label.Found)
               {
                  Line.Label = Label.Before;
                  Text = Trim_Left(Label.After);
               }

               cut Directive = Cut(Text, '#');
               Line.Instruction = Trim(Directive.Before);
               if(Directive.Found)
               {
                  Line.Directive = Trim(Directive.After);
               }
            }

            Lines[Current_Line_Index++] = Line;
         }
      }

      // Third pass to generate machine code based on identified assembly
      // instructions. The address associated with each label is stored.

      index Machine_Address = 0;
      for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
      {
         source_code_line *Line = Lines + Line_Index;
         Line->Machine_Address = Machine_Address;

         if(Line->Label.Length)
         {
            label_address Label_Address = {0};
            Label_Address.Label = Line->Label;
            Label_Address.Address = Line->Machine_Address;
            Label_Addresses[Label_Address_Count++] = Label_Address;
         }

         if(Line->Instruction.Length)
         {
            Line->Machine_Instruction = Generate_Machine_Instruction(Line->Instruction);
            Line->Machine_Address = Machine_Address;

            Machine_Address += Line->Machine_Instruction.Length;
         }
      }

      // Fourth pass to patch addresses into any instructions that reference
      // labels.
      for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
      {
         source_code_line *Line = Lines + Line_Index;
         if(Line->Machine_Instruction.Label_Operand.Length)
         {
            // TODO: Smarter lookup.
            index Label_Address = 0xFF;
            for(int Label_Index = 0; Label_Index < Label_Address_Count; ++Label_Index)
            {
               label_address *A = Label_Addresses + Label_Index;
               if(Equals(A->Label, Line->Machine_Instruction.Label_Operand))
               {
                  Label_Address = A->Address;
               }
            }

            Patch_Label_Address(&Line->Machine_Instruction, Line->Machine_Address, Label_Address);
         }
      }

      for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
      {
         Print_Instruction(Lines + Line_Index);
      }
   }

   return(0);
}
