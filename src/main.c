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
   string Instruction;
   string Directive;

   int Number;
} asm_line;

typedef struct {
   u64 Address;
   int Length;
   u8 Bytes[16];
} machine_instruction;

#define GENERATE_MACHINE_INSTRUCTION(Name) machine_instruction Name(asm_line Line)

#if ARCH_6502
#   include "architecture_6502.c"
#elif ARCH_ARM4VT
#   include "architecture_armv4t.c"
#else
#   error Unhandled architecture.
#endif

int main(int Argument_Count, char **Arguments)
{
   arena Arena = {0};
   Arena.Size = 1024 * 1024 * 1024;
   Arena.Base = malloc(Arena.Size);

   // NOTE: For now, assume that all provided arguments are input files.
   for(int Input_Index = 1; Input_Index < Argument_Count; ++Input_Index)
   {
      char *Path = Arguments[Input_Index];
      printf("Input_File_%02d: %s\n", Input_Index, Path);

      string Source = Read_Entire_File(&Arena, Path);

      cut Source_Cut = {0};
      int Line_Count = 0; // Number of lines to allocate.

      Source_Cut.After = Source;
      while(Source_Cut.After.Length)
      {
         Source_Cut = Cut(Source_Cut.After, '\n');
         cut Line_With_Comments = Cut(Source_Cut.Before, '\\');
         string Text = Trim(Line_With_Comments.Before);
         Line_Count += (Text.Length > 0);
      }

      asm_line *Lines = Allocate(&Arena, asm_line, Line_Count);
      int Line_Index = 0;  // Index of allocated source line to populate.
      int Line_Number = 1; // Line number position in source file.

      Source_Cut.After = Source;
      while(Source_Cut.After.Length)
      {
         Source_Cut = Cut(Source_Cut.After, '\n');
         cut Comment = Cut(Source_Cut.Before, '\\');

         asm_line Line = {0};
         Line.Number = Line_Number++;

         string Text = Trim(Comment.Before);
         if(Text.Length > 0)
         {
            if(Text.Data[0] == '#')
            {
               // If a line begins with '#', it's a directive that extends to
               // the end of the line, e.g. "#section text".
               cut Directive = Cut(Text, '#');
               Line.Directive = Trim(Directive.After);
            }
            else
            {
               cut Label = Cut(Text, ':');
               if(Label.Found)
               {
                  Line.Label = Trim(Label.Before);
                  Text = Trim(Label.After);
               }

               cut Directive = Cut(Text, '#');
               Line.Instruction = Trim(Directive.Before);
               if(Directive.Found)
               {
                  Line.Directive = Trim(Directive.After);
               }
            }

            Lines[Line_Index++] = Line;
         }
      }

      for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
      {
         asm_line Line = Lines[Line_Index];
         printf("L%03d | ", Line.Number);

         if(Line.Instruction.Length)
         {
            machine_instruction Machine_Instruction = Generate_Machine_Instruction(Line);
            for(int Index = 0; Index < Machine_Instruction.Length; ++Index)
            {
               printf("%02x ", Machine_Instruction.Bytes[Index]);
            }
         }
         if(Line.Label.Length)
         {
            printf("(LABEL: '%.*s') ", Line.Label.Length, Line.Label.Data);
         }
         if(Line.Directive.Length)
         {
            printf("(DIRECTIVE: '%.*s') ", Line.Directive.Length, Line.Directive.Data);
         }

         printf("\n");
      }
   }

   return(0);
}
