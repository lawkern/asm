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
   arena Arena;
   map *Constants;
} assembler_context;

static assembler_context Context;

typedef struct {
   int Length;
   union
   {
      u8 Bytes[16];
      u8 *Bytes_Pointer;
   };

   string Label_Operand;
} machine_code;

#define GENERATE_MACHINE_INSTRUCTION(Name) machine_code Name(string Instruction)
#define PATCH_LABEL_ADDRESS(Name) void Name(machine_code *Result, index Instruction_Address, index Label_Address)

#if ARCH_6502
#   include "architecture_6502.c"
#elif ARCH_ARMV4T
#   include "architecture_armv4t.c"
#elif ARCH_ARMV8
#   include "architecture_armv8.c"
#else
#   error Unhandled architecture.
#endif

typedef struct {
   string Source_Code_Path;
   int Source_Code_Line_Number;

   string Label;
   string Instruction;
   string Directive;

   index Machine_Address;
   machine_code Machine_Code;
} source_code_line;

static void Encode_Literal_Bytes(source_code_line *Line, int Bytes_Per_Literal)
{
   machine_code Result = {0};

   if(Line->Instruction.Length)
   {
      Report_Error("Don't use an embedding directive on the same line as an instruction.");
   }
   else
   {
      index Literal_Count = 0;
      cut Literals = {0};

      // Count literals.
      Literals.After = Line->Directive;
      while(Literals.After.Length)
      {
         Literals = Cut(Literals.After, ' ');
         Literal_Count++;
      }

      u8 *Destination = Result.Bytes;
      Result.Length = Literal_Count * Bytes_Per_Literal;
      if(Result.Length > Array_Count(Result.Bytes))
      {
         Destination = Allocate(&Context.Arena, u8, Result.Length);
         Result.Bytes_Pointer = Destination;
      }

      // Populate Literals.
      index Byte_Count = 0;
      Literals.After = Line->Directive;
      while(Literals.After.Length)
      {
         Literals = Cut_Whitespace(Literals.After);
         if(Literals.Before.Length)
         {
            parsed_integer Parsed_Literal = Parse_Integer(Literals.Before);
            if(Parsed_Literal.Ok)
            {
               s64 Value = (s64)Parsed_Literal.Value;

               // TODO: Handle endianess.
               for(int Byte_Index = 0; Byte_Index < Bytes_Per_Literal; ++Byte_Index)
               {
                  Destination[Byte_Count++] = (u8)(Value >> (Byte_Index * 8));
               }
            }
            else
            {
               Report_Error("Could not parse \"%.*s\" as an integer literal.", (int)Literals.Before.Length, Literals.Before.Data);
            }
         }
      }
   }

   Line->Machine_Code = Result;
}

typedef enum {
   STRINGKIND_STRING,
   STRINGKIND_CSTRING,
} string_kind;

static void Encode_Literal_String(source_code_line *Line, string_kind Kind)
{
   machine_code Result = {0};

   if(Line->Instruction.Length)
   {
      Report_Error("Don't use an embedding directive on the same line as an instruction.");
   }
   else
   {
      if(Has_Prefix_Then_Remove(&Line->Directive, S("\"")) &&
         Has_Suffix_Then_Remove(&Line->Directive, S("\"")))
      {
         bool Null_Terminate = (Kind == STRINGKIND_CSTRING);
         Result.Length = Line->Directive.Length + Null_Terminate;

         u8 *Destination = Result.Bytes;
         if(Result.Length > Array_Count(Result.Bytes))
         {
            Destination = Allocate(&Context.Arena, u8, Result.Length);
            Result.Bytes_Pointer = Destination;
         }

         for(int Byte_Index = 0; Byte_Index < (Result.Length - Null_Terminate); ++Byte_Index)
         {
            Destination[Byte_Index] = Line->Directive.Data[Byte_Index];
         }
         if(Null_Terminate)
         {
            Destination[Result.Length] = 0;
         }
      }
      else
      {
         Report_Error("Use double quotes for string literals.");
      }
   }

   Line->Machine_Code = Result;
}

static void Print_Instruction(source_code_line *Line)
{
   printf("%.*s:%-4d | ",
          (int)Line->Source_Code_Path.Length,
          Line->Source_Code_Path.Data,
          Line->Source_Code_Line_Number);

   if(Line->Machine_Code.Length)
   {
      printf("0x%04x: ", (u32)Line->Machine_Address);
      for(int Index = 0; Index < Line->Machine_Code.Length; ++Index)
      {
         printf("%02x ", Line->Machine_Code.Bytes[Index]);
      }
      for(int Index = 4; Index >= (Line->Machine_Code.Length); --Index)
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

static int Count_Lines_Of_Code(string Source_Code)
{
   int Result = 0;

   cut Lines = {0};
   Lines.After = Source_Code;

   while(Lines.After.Length)
   {
      Lines = Cut(Lines.After, '\n');
      cut Comment = Cut(Lines.Before, '\\');

      Result += (Trim(Comment.Before).Length > 0);
   }

   return(Result);
}

static void Tokenize_Source_Lines(source_code_line *Result, string Source_Code, string Source_Code_Path)
{
   int Source_Line_Number = 1;
   int Current_Line_Index = 0; // Index of allocated source line to populate.

   cut Remaining_Lines = {0};
   Remaining_Lines.After = Source_Code;

   Remaining_Lines.After = Source_Code;
   while(Remaining_Lines.After.Length)
   {
      Remaining_Lines = Cut(Remaining_Lines.After, '\n');
      cut Comment = Cut(Remaining_Lines.Before, '\\');
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

         Result[Current_Line_Index++] = Line;
      }
   }
}

typedef struct {
   index Size;
   string File_Name;
} parse_result;

static parse_result Parse_Source_Lines(source_code_line *Lines, int Line_Count)
{
   parse_result Result = {0};

   for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
   {
      source_code_line *Line = Lines + Line_Index;
      Line->Machine_Address = Result.Size;

      if(Line->Directive.Length)
      {
         if(Has_Prefix_Then_Remove(&Line->Directive, S("file ")))
         {
            Result.File_Name = Trim_Left(Line->Directive);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("location ")))
         {
            parsed_integer Parsed_Address = Parse_Integer(Trim(Line->Directive));
            if(Parsed_Address.Ok)
            {
               Result.Size = Parsed_Address.Value;
            }
            else
            {
               Report_Error("Failed to parse #location value: \"%.*s\".", Line->Directive);
            }
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("bytes ")))
         {
            Encode_Literal_Bytes(Line, 1);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("2bytes ")))
         {
            Encode_Literal_Bytes(Line, 2);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("4bytes ")))
         {
            Encode_Literal_Bytes(Line, 4);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("8bytes ")))
         {
            Encode_Literal_Bytes(Line, 8);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("string ")))
         {
            Encode_Literal_String(Line, STRINGKIND_STRING);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("cstring ")))
         {
            Encode_Literal_String(Line, STRINGKIND_CSTRING);
         }
         else if(Has_Prefix_Then_Remove(&Line->Directive, S("constant ")))
         {
            cut Constant_Parts = Cut_Whitespace(Trim_Left(Line->Directive));
            string Name = Constant_Parts.Before;
            string Value = Constant_Parts.After;
            if(Name.Length && Value.Length)
            {
               parsed_integer Parsed_Value = Parse_Integer(Value);
               if(Parsed_Value.Ok)
               {
                  Insert(&Context.Arena, &Context.Constants, Name, Parsed_Value.Value);
               }
               else
               {
                  Report_Error("Invalid integer value: %.*s", (int)Value.Length, Value.Data);
               }
            }
         }
      }

      if(Line->Label.Length)
      {
         Insert(&Context.Arena, &Context.Constants, Line->Label, Line->Machine_Address);
      }

      if(Line->Instruction.Length)
      {
         Line->Machine_Code = Generate_Machine_Instruction(Line->Instruction);
         Line->Machine_Address = Result.Size;
      }

      Result.Size += Line->Machine_Code.Length;
   }

   return(Result);
}

static void Write_Machine_Code(u8 *Result, source_code_line *Lines, int Line_Count)
{
   index Output_Machine_Address = 0;

   for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
   {
      source_code_line *Line = Lines + Line_Index;
      string Label = Line->Machine_Code.Label_Operand;

      if(Label.Length)
      {
         lookup_result Label_Address = Lookup(Context.Constants, Label);
         if(Label_Address.Found)
         {
            Patch_Label_Address(&Line->Machine_Code, Line->Machine_Address, Label_Address.Value);
         }
         else
         {
            Report_Error("Reference to missing label \"%.*s\".", (int)Label.Length, Label.Data);
         }
      }

      Output_Machine_Address = Line->Machine_Address;
      u8 *Source = (Line->Machine_Code.Length > Array_Count(Line->Machine_Code.Bytes))
         ? Line->Machine_Code.Bytes_Pointer
         : Line->Machine_Code.Bytes;

      for(int Byte_Index = 0; Byte_Index < Line->Machine_Code.Length; ++Byte_Index)
      {
         Result[Output_Machine_Address++] = Source[Byte_Index];
      }
   }
}

int main(int Argument_Count, char **Arguments)
{
   arena *Arena = &Context.Arena;
   Arena->Size = 1024 * 1024 * 1024;
   Arena->Base = malloc(Arena->Size);

   // NOTE: For now, assume that all provided arguments are input files.
   for(int Argument_Index = 1; Argument_Index < Argument_Count; ++Argument_Index)
   {
      char *Source_Code_Path_0 = Arguments[Argument_Index];
      string Source_Code = Read_Entire_File(Arena, Source_Code_Path_0);
      if(Source_Code.Length)
      {
         // First pass to determine the number of lines to allocate. This will
         // include any non-empty line of source code.
         string Source_Code_Path = From_C_String(Source_Code_Path_0);
         int Line_Count = Count_Lines_Of_Code(Source_Code);

         // Second pass to identify directives, labels and instructions for each
         // allocated line of assembly code.
         source_code_line *Lines = Allocate(Arena, source_code_line, Line_Count);
         Tokenize_Source_Lines(Lines, Source_Code, Source_Code_Path);

         // Third pass to generate machine code based on identified assembly
         // instructions. The address associated with each label is stored.
         parse_result Parse_Result = Parse_Source_Lines(Lines, Line_Count);

         // Fourth pass to populate output buffer with machine code and patch
         // addresses into any instructions that reference labels.
         u8 *Output = Allocate(Arena, u8, Parse_Result.Size);
         Write_Machine_Code(Output, Lines, Line_Count);

         string Output_File_Name = {0};
         if(Parse_Result.File_Name.Length)
         {
            Output_File_Name = Parse_Result.File_Name;
         }
         else
         {
            // Use the input file name for output if no #file directive.
            cut Path = {0};
            Path.After = Source_Code_Path;
            while(Path.After.Length)
            {
               Path = Cut(Path.After, '/');
            }
            Output_File_Name = Path.Before;
            Has_Suffix_Then_Remove(&Output_File_Name, S(".asm"));
         }

         // TODO: Converting back and forth to null-terminated strings is silly,
         // but the file read and write functions work more naturally with them
         // when using the CRT. So maybe stop using CRT functions.
         Write_Entire_File(Output, Parse_Result.Size, To_C_String(Arena, Output_File_Name));
      }
   }

   return(0);
}
