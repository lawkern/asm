/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct assembler_context;
static void Report_Error(struct assembler_context *Context, char *Message, ...);

#include "memory.c"

#include "architecture.h"
#if ARCH_6502
#   include "architecture_6502.c"
#elif ARCH_ARMV4
#   include "architecture_armv4.c"
#elif ARCH_MIPS
#   include "architecture_mips.c"
#elif ARCH_ARMV8
#   include "architecture_armv8.c"
#else
#   error Unhandled architecture.
#endif

static void Report_Error(assembler_context *Context, char *Message, ...)
{
   if(Context)
   {
      fprintf(stderr, "%.*s:%d: error: ", SF(Context->Input_File_Path), Context->Current_Line_Number);
   }
   else
   {
      fprintf(stderr, "ERROR: ");
   }

   va_list Arguments;
   va_start(Arguments, Message);
   vfprintf(stderr, Message, Arguments);
   va_end(Arguments);

   fprintf(stderr, "\n");
}

static void Encode_Literal_Bytes(assembler_context *Context, source_code_line *Line, int Bytes_Per_Literal)
{
   // NOTE: Produce the literal byte values supplied by the #*bytes assembler
   // directives.
   if(Line->Instruction.Length)
   {
      Report_Error(Context, "Don't use an embedding directive on the same line as an instruction.");
   }
   else
   {
      index Literal_Count = 0;
      cut Literals = {0};

      // Count literals.
      Literals.After = Line->Directive;
      while(Literals.After.Length)
      {
         Literals = Cut_Whitespace(Literals.After);
         Literal_Count++;
      }

      u8 *Destination = Line->Machine_Code.Bytes;
      Line->Machine_Code.Length = Literal_Count * Bytes_Per_Literal;
      if(Line->Machine_Code.Length > Array_Count(Line->Machine_Code.Bytes))
      {
         Destination = Allocate(&Context->Arena, u8, Line->Machine_Code.Length);
         Line->Machine_Code.Bytes_Pointer = Destination;
      }

      // Populate Literals.
      index Byte_Count = 0;
      Literals.After = Line->Directive;
      while(Literals.After.Length)
      {
         Literals = Cut_Whitespace(Literals.After);

         s64 Value = 0;
         bool Ok = false;

         string Literal = Literals.Before;
         if(Literal.Length)
         {
            if(Literal.Data[0] >= '0' && Literal.Data[0] <= '9')
            {
               parsed_integer Parsed_Literal = Parse_Integer(Literal);
               Ok = Parsed_Literal.Ok;

               if(Ok)
               {
                  Value = (s64)Parsed_Literal.Value;
               }
               else
               {
                  Report_Error(Context, "Could not parse \"%.*s\" as an integer literal.", SF(Literal));
               }
            }
            else
            {
               lookup_result Constant = Lookup(Context->Constants, Literal);
               Ok = Constant.Found;

               if(Ok)
               {
                  Value = (s64)Constant.Value;
               }
               else
               {
                  Request_Patch(&Context->Arena, &Line->Machine_Code, Literal, Line->Machine_Code.Address, Line->Machine_Code.Length);
               }
            }

            if(Ok)
            {
               // TODO: Handle endianess.
               for(int Byte_Index = 0; Byte_Index < Bytes_Per_Literal; ++Byte_Index)
               {
                  Destination[Byte_Count++] = (u8)(Value >> (Byte_Index * 8));
               }
            }
         }
      }
   }
}

typedef enum {
   STRINGKIND_STRING,
   STRINGKIND_CSTRING,
} string_kind;

static void Encode_Literal_String(assembler_context *Context, source_code_line *Line, string_kind Kind)
{
   machine_code *Result = &Line->Machine_Code;

   if(Line->Instruction.Length)
   {
      Report_Error(Context, "Don't use an embedding directive on the same line as an instruction.");
   }
   else
   {
      if(Has_Prefix_Then_Remove(&Line->Directive, S("\"")) &&
         Has_Suffix_Then_Remove(&Line->Directive, S("\"")))
      {
         bool Null_Terminate = (Kind == STRINGKIND_CSTRING);
         Result->Length = Line->Directive.Length + Null_Terminate;

         u8 *Destination = Result->Bytes;
         if(Result->Length > Array_Count(Result->Bytes))
         {
            Destination = Allocate(&Context->Arena, u8, Result->Length);
            Result->Bytes_Pointer = Destination;
         }

         for(int Byte_Index = 0; Byte_Index < (Result->Length - Null_Terminate); ++Byte_Index)
         {
            Destination[Byte_Index] = Line->Directive.Data[Byte_Index];
         }
         if(Null_Terminate)
         {
            Destination[Result->Length] = 0;
         }
      }
      else
      {
         Report_Error(Context, "Use double quotes for string literals.");
      }
   }
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

static void Tokenize_Source_Lines(source_code_line *Result, string Source_Code)
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
      Line.Line_Number = Source_Line_Number++;

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

static void Parse_Source_Line(assembler_context *Context, source_code_line *Line)
{
   Line->Machine_Code.Address = Context->Current_Address;
   Context->Current_Line_Number = Line->Line_Number;

   if(Line->Directive.Length)
   {
      if(Has_Prefix_Then_Remove(&Line->Directive, S("file ")))
      {
         Context->Output_File_Name = Trim_Left(Line->Directive);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("location ")))
      {
         parsed_integer Parsed_Address = Parse_Integer(Trim(Line->Directive));
         if(Parsed_Address.Ok)
         {
            Context->Current_Address = Parsed_Address.Value;
         }
         else
         {
            Report_Error(Context, "Failed to parse #location value: \"%.*s\".", SF(Line->Directive));
         }
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("bytes ")))
      {
         Encode_Literal_Bytes(Context, Line, 1);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("2bytes ")))
      {
         Encode_Literal_Bytes(Context, Line, 2);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("4bytes ")))
      {
         Encode_Literal_Bytes(Context, Line, 4);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("8bytes ")))
      {
         Encode_Literal_Bytes(Context, Line, 8);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("string ")))
      {
         Encode_Literal_String(Context, Line, STRINGKIND_STRING);
      }
      else if(Has_Prefix_Then_Remove(&Line->Directive, S("cstring ")))
      {
         Encode_Literal_String(Context, Line, STRINGKIND_CSTRING);
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
               Insert(&Context->Arena, &Context->Constants, Name, Parsed_Value.Value);
            }
            else
            {
               Report_Error(Context, "Invalid integer value: %.*s", SF(Value));
            }
         }
      }
   }

   if(Line->Label.Length)
   {
      Insert(&Context->Arena, &Context->Constants, Line->Label, Line->Machine_Code.Address);
   }

   if(Line->Instruction.Length)
   {
      Line->Machine_Code = Encode_Instruction(Context, Line->Instruction);
      Line->Machine_Code.Address = Context->Current_Address;
   }

   Context->Current_Address += Line->Machine_Code.Length;
}

static void Encode_Source_Line(assembler_context *Context, u8 *Result, source_code_line *Line)
{
   Context->Current_Address = Line->Machine_Code.Address;
   Context->Current_Line_Number = Line->Line_Number;

   u8 *Source = (Line->Machine_Code.Length > Array_Count(Line->Machine_Code.Bytes))
      ? Line->Machine_Code.Bytes_Pointer
      : Line->Machine_Code.Bytes;

   Apply_Patches(Context, &Line->Machine_Code);
   for(int Byte_Index = 0; Byte_Index < Line->Machine_Code.Length; ++Byte_Index)
   {
      Result[Context->Current_Address++] = Source[Byte_Index];
   }

}

static string Name_Output_File(assembler_context *Context)
{
   string Result = Context->Output_File_Name;

   if(Context->Output_File_Name.Length == 0)
   {
      // NOTE: Remove preceding path from input file name.
      cut Nodes = {0};
      Nodes.After = Context->Input_File_Path;
      while(Nodes.After.Length)
      {
         Nodes = Cut(Nodes.After, '/');
      }
      Result = Nodes.Before;

      // NOTE: Remove file extension in case where input file name was used.
      Has_Suffix_Then_Remove(&Result, S(".asm"));
   }

   return(Result);
}

int main(int Argument_Count, char **Arguments)
{
   assembler_context Context = {0};
   Context.Arena.Size = 256 * 1024 * 1024;
   Context.Arena.Base = malloc(Context.Arena.Size);

   arena *Arena = &Context.Arena;

   Initialize_Architecture(&Context);

   // NOTE: For now, assume that all provided arguments are input files.
   for(int Argument_Index = 1; Argument_Index < Argument_Count; ++Argument_Index)
   {
      char *Path = Arguments[Argument_Index];
      string Source_Code = Read_Entire_File(Arena, Path);

      if(Source_Code.Length)
      {
         Context.Input_File_Path = From_C_String(Path);

         // First pass to determine the number of lines to allocate. This will
         // include any non-empty line of source code.
         int Line_Count = Count_Lines_Of_Code(Source_Code);

         // Second pass to identify directives, labels and instructions for each
         // allocated line of assembly code.
         source_code_line *Lines = Allocate(Arena, source_code_line, Line_Count);
         Tokenize_Source_Lines(Lines, Source_Code);

         // Third pass to generate machine code based on identified assembly
         // instructions. The address associated with each label is stored.
         for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
         {
            Parse_Source_Line(&Context, Lines + Line_Index);
         }

         // Fourth pass to populate output buffer with machine code and patch
         // addresses into any instructions that reference labels.
         u8 *Output = Allocate(Arena, u8, Context.Current_Address);
         for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
         {
            Encode_Source_Line(&Context, Output, Lines + Line_Index);
         }

         // TODO: Converting back and forth to null-terminated strings is silly,
         // but the file read and write functions work more naturally with them
         // when using the CRT. So maybe stop using CRT functions.
         string Output_Name = Name_Output_File(&Context);
         if(!Write_Entire_File(Output, Context.Current_Address, To_C_String(Arena, Output_Name)))
         {
            Report_Error(0, "Failed to write to output file \"%.*s\".", SF(Output_Name));
         }
      }

      // Reset assembler state for the next input file.
      Reset_Arena(Arena);
      Context.Current_Address = 0;
      Context.Constants = 0;
   }

   return(0);
}
