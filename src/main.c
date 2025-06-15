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

#define Allocate(Arena, type, Count) (type *)Allocate_Size((Arena), sizeof(type) * (Count))

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
   string Head;
   string Tail;
   bool Ok;
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

      Result.Ok = (Cut_Position < End);
      Result.Head = Span(Begin, Cut_Position);
      Result.Tail = Span(Cut_Position + Result.Ok, End);
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

typedef struct line_block {
   string Lines[1024];
   int Count;

   struct line_block *Next;
} line_block;

static line_block *Lex(arena *Arena, string Source)
{
   // NOTE: For now, the lexed "tokens" are just the individual non-empty lines
   // in the input file.

   line_block *Result = 0;
   line_block *Block = 0;

   cut Input_Cut = {0};
   Input_Cut.Tail = Source;

   while(Input_Cut.Tail.Length)
   {
      Input_Cut = Cut(Input_Cut.Tail, '\n');
      if(Input_Cut.Ok)
      {
         string Line = Input_Cut.Head;
         Line = Trim_Left(Line);
         Line = Trim_Right(Line);

         if(Line.Length > 0)
         {
            if(!Block)
            {
               Block = Allocate(Arena, line_block, 1);
               Block->Count = 0;
               Block->Next = 0;

               if(!Result)
               {
                  Result = Block;
               }
            }
            else if(Block->Count >= Array_Count(Block->Lines))
            {
               Block->Next = Allocate(Arena, line_block, 1);
               Block = Block->Next;
               Block->Count = 0;
               Block->Next = 0;
            }

            Block->Lines[Block->Count++] = Line;
         }
      }
   }

   return(Result);
}

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
      for(line_block *Block = Lex(&Arena, Source); Block; Block = Block->Next)
      {
         for(int Line_Index = 0; Line_Index < Block->Count; ++Line_Index)
         {
            string Line = Block->Lines[Line_Index];
            printf("%03d: %.*s\n", Line_Index, Line.Length, Line.Data);
         }
      }
   }

   return(0);
}
