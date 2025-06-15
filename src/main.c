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
   ASM_LINE_DIRECTIVE,
} asm_line_kind;

typedef struct {
   asm_line_kind Kind;
   string Text;

   string Mnemonic;
   string Label;
   string Directive;
   string Arguments;

   int Number;
} asm_line;

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
         Line.Text = Trim_Left(Trim_Right(Comment.Before));
         Line.Number = Line_Number++;
         if(Line.Text.Length > 0)
         {
            // NOTE: This line contained actual code, so store it for parsing.
            if(Line.Text.Data[0] == '#')
            {
               Line.Kind = ASM_LINE_DIRECTIVE;
               cut Directive_With_Argument = Cut(Line.Text, ' ');
               Line.Directive = Directive_With_Argument.Before;
               if(Directive_With_Argument.Found)
               {
                  Line.Arguments = Directive_With_Argument.After;
               }
            }
            else
            {
               Line.Kind = ASM_LINE_INSTRUCTION;
               Line.Mnemonic = Line.Text;

               cut Line_With_Label = Cut(Line.Text, ':');
               if(Line_With_Label.Found)
               {
                  Line.Label = Line_With_Label.Before;
                  Line.Mnemonic = Trim_Left(Line_With_Label.After);
               }

               if(Line.Mnemonic.Length)
               {
                  cut Arguments = Cut(Line.Mnemonic, ' ');
                  if(Arguments.Found)
                  {
                     Line.Arguments = Arguments.After;
                  }
               }
               else
               {
                  // NOTE: ASM_LINE_LABEL refers to lines with only a label, no
                  // instruction. Lines such as `.loop: b .loop` are considered
                  // to be ASM_LINE_INSTRUCTION's instead.
                  Line.Kind = ASM_LINE_LABEL;
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
            case ASM_LINE_INSTRUCTION: {
               printf("%.*s", Line.Mnemonic.Length, Line.Mnemonic.Data);
            } break;

            case ASM_LINE_LABEL: {
               printf("%.*s", Line.Label.Length, Line.Label.Data);
            } break;

            case ASM_LINE_DIRECTIVE: {
               printf("%.*s", Line.Directive.Length, Line.Directive.Data);
            } break;
         }

         if(Line.Arguments.Length)
         {
            printf(" %.*s", Line.Arguments.Length, Line.Arguments.Data);
         }
         printf("\n");
      }
   }

   return(0);
}
