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
   u8 *Data;
   index Length;
} string;

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

static u8 *Read_Entire_File(arena *Arena, char *Path)
{
   u8 *Result = 0;

   FILE *File = fopen(Path, "rb");
   if(File)
   {
      if(fseek(File, 0, SEEK_END) == 0)
      {
         size_t Size = ftell(File);
         if(fseek(File, 0, SEEK_SET) == 0)
         {
            Result = Allocate(Arena, u8, Size + 1); // Extra byte for null terminator.
            if(Result)
            {
               if(fread(Result, 1, Size, File) == Size)
               {
                  Result[Size] = 0; // Null-terminated for your convenience.
               }
               else
               {
                  Report_Error("Failed to read input file \"%s\".", Path);
                  Result = 0;
               }
            }
            else
            {
               Report_Error("Failed to allocate memory for input file \"%s\".", Path);
            }
         }
         else
         {
            Report_Error("Seek to beginning of file failed for \"%s\".", Path);
         }
      }
      else
      {
         Report_Error("Seek to end of file failed for \"%s\".", Path);
      }
   }
   else
   {
      Report_Error("Failed to open input file \"%s\".", Path);
   }

   // TODO: Not enough error states.

   return(Result);
}

static bool Is_Whitespace(u8 Character)
{
   bool Result = (Character == ' ' ||
                  Character == '\t' ||
                  Character == '\v' ||
                  Character == '\f' ||
                  Character == '\n' ||
                  Character == '\r');
   return(Result);
}

typedef struct line_block {
   string Lines[1024];
   int Count;

   struct line_block *Next;
} line_block;

static line_block *Lex(arena *Arena, u8 *Input_Memory)
{
   // NOTE: For now, the lexed "tokens" are just the individual non-empty lines
   // in the input file.

   line_block *Result = Allocate(Arena, line_block, 1);
   Result->Count = 0;
   Result->Next = 0;

   line_block *Block = Result;

   while(Input_Memory && *Input_Memory)
   {
      // NOTE: Remove preceeding whitespace (including empty lines).
      while(*Input_Memory && Is_Whitespace(*Input_Memory))
      {
         Input_Memory++;
      }

      if(*Input_Memory)
      {
         if(Block->Count >= Array_Count(Block->Lines))
         {
            Block->Next = Allocate(Arena, line_block, 1);

            Block = Block->Next;
            Block->Count = 0;
            Block->Next = 0;
         }

         string *Line = Block->Lines + Block->Count++;
         Line->Data = Input_Memory;

         while(*Input_Memory && *Input_Memory != '\n')
         {
            Input_Memory++;
         }
         Line->Length = Input_Memory - Line->Data;

         if(*Input_Memory == '\n')
         {
            Input_Memory++;
         }
         else
         {
            assert(*Input_Memory == 0);
         }
      }
   }

   return(Result);
}

int main(int Argument_Count, char **Arguments)
{
   arena Arena = {0};
   Arena.Size = 1024 * 1024;
   Arena.Base = malloc(Arena.Size);

   // NOTE: For now, assume that all provided arguments are input files.
   for(int Input_Index = 1; Input_Index < Argument_Count; ++Input_Index)
   {
      char *Path = Arguments[Input_Index];
      printf("Input_File_%02d: %s\n", Input_Index, Path);

      u8 *Input_Memory = Read_Entire_File(&Arena, Path);
      if(Input_Memory)
      {
         for(line_block *Block = Lex(&Arena, Input_Memory); Block; Block = Block->Next)
         {
            for(int Line_Index = 0; Line_Index < Block->Count; ++Line_Index)
            {
               string Line = Block->Lines[Line_Index];
               printf("%03d: %.*s\n", Line_Index, Line.Length, Line.Data);
            }
         }
      }
   }

   return(0);
}
