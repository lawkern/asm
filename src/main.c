/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

#include <stdint.h>
typedef uint8_t u8;

#include <stddef.h>
typedef ptrdiff_t index;

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define index YOU_CANT_HAVE_INDEX
#include <string.h>
#undef index

#define Array_Count(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef struct {
   u8 *Base;
   index Size;
   index Used;
} arena;

static void Report_Error(char *Message, ...)
{
   fprintf(stderr, "ERROR: ");

   va_list Arguments;
   va_start(Arguments, Message);
   vfprintf(stderr, Message, Arguments);
   va_end(Arguments);

   fprintf(stderr, "\n");
}

#define Allocate(Arena, type, Count)                        \
   (type *)Allocate_Size((Arena), sizeof(type) * (Count))

static void *Allocate_Size(arena *Arena, index Size)
{
   void *Result = 0;
   if(Size <= (Arena->Size - Arena->Used))
   {
      Result = Arena->Base + Arena->Used;
      Arena->Used += Size;
   }
   else
   {
      Report_Error("Arena ran out of memory, failed to allocate %zu bytes.", Size);
   }

   return(Result);
}

typedef struct {
   u8 *Data;
   index Length;
} string;

#define S(String) (string){(String), sizeof(String)-1}

static string Span(u8 *Begin, u8 *End)
{
   string Result = {0};
   Result.Data = Begin;
   if(Begin)
   {
      Result.Length = End - Begin;
   }

   return(Result);
}

static bool Equals(string A, string B)
{
   bool Result = (A.Length == B.Length) && (!A.Length || !memcmp(A.Data, B.Data, A.Length));
   return Result;
}

static string Trim_Left(string String)
{
   while(String.Length && *String.Data <= ' ')
   {
      String.Data++;
      String.Length--;
   }

   return(String);
}

static string Trim_Right(string String)
{
   while(String.Length && String.Data[String.Length - 1] <= ' ')
   {
      String.Length--;
   }

   return(String);
}

typedef struct {
   string Before;
   string After;
   bool Found;
} cut;

static cut Cut(string String, u8 Separator)
{
   cut Result = {0};

   if(String.Length > 0)
   {
      u8 *Begin = String.Data;
      u8 *End = Begin + String.Length;

      u8 *Cut_Position = Begin;
      while(Cut_Position < End && *Cut_Position != Separator)
      {
         Cut_Position++;
      }

      Result.Found = (Cut_Position < End);
      Result.Before = Span(Begin, Cut_Position);
      Result.After = Span(Cut_Position + Result.Found, End);
   }

   return(Result);
}

static string Read_Entire_File(arena *Arena, char *Path)
{
   string Result = {0};

   FILE *File = fopen(Path, "rb");
   if(File)
   {
      Result.Data = Arena->Base + Arena->Used;

      index Available_Space = Arena->Size - Arena->Used;
      Result.Length = fread(Result.Data, 1, Available_Space, File);
      Allocate_Size(Arena, Result.Length);

      if(Result.Length == Available_Space)
      {
         Report_Error("File exhausted arena memory, likely truncating \"%s\".", Path);
      }
   }
   else
   {
      Report_Error("Failed to open input file \"%s\".", Path);
   }

   return(Result);
}

typedef enum {
   ASM_LINE_INSTRUCTION,
   ASM_LINE_LABEL,
   ASM_LINE_LABEL_INSTRUCTION,
   ASM_LINE_DIRECTIVE,
} asm_line_kind;

typedef struct {
   asm_line_kind Kind;
   union
   {
      struct
      {
         string Label;
         string Instruction;
      };
      string Directive;
   };
   int Number;
} asm_line;

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
         string Text = Trim_Left(Trim_Right(Line_With_Comments.Before));
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

         string Text = Trim_Left(Trim_Right(Comment.Before));
         if(Text.Length > 0)
         {
            // NOTE: This line contained actual code, so store it for parsing.
            if(Text.Data[0] == '#')
            {
               Line.Kind = ASM_LINE_DIRECTIVE;
               Line.Directive = Text;
            }
            else
            {
               cut Label = Cut(Text, ':');
               if(Label.Found)
               {
                  Line.Label = Label.Before;
                  Line.Instruction = Trim_Left(Trim_Right(Label.After));
                  Line.Kind = (Line.Instruction.Length == 0) ? ASM_LINE_LABEL : ASM_LINE_LABEL_INSTRUCTION;
               }
               else
               {
                  Line.Kind = ASM_LINE_INSTRUCTION;
                  Line.Instruction = Label.Before;
               }
            }

            Lines[Line_Index++] = Line;
         }
      }

      for(int Line_Index = 0; Line_Index < Line_Count; ++Line_Index)
      {
         asm_line Line = Lines[Line_Index];
         printf("%03d:  ", Line.Number);

         switch(Line.Kind)
         {
            case ASM_LINE_INSTRUCTION:
            case ASM_LINE_LABEL_INSTRUCTION: {
               Generate_Instruction(Line);
               printf(" (INSTRUCTION)");
            } break;

            case ASM_LINE_LABEL: {
               printf("%.*s", Line.Label.Length, Line.Label.Data);
               printf(" (LABEL)");
            } break;

            case ASM_LINE_DIRECTIVE: {
               printf("%.*s", Line.Directive.Length, Line.Directive.Data);
               printf(" (DIRECTIVE)");
            } break;
         }

         printf("\n");
      }
   }

   return(0);
}
