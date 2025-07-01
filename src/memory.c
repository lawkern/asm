/* (c) copyright 2025 Lawrence D. Kern /////////////////////////////////////// */

#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#include <stddef.h>
typedef ptrdiff_t index;

#define index YOU_CANT_HAVE_INDEX
#include <string.h>
#undef index

typedef struct {
   u8 *Base;
   index Size;
   index Used;
} arena;

#define Array_Count(Array) (index)(sizeof(Array) / sizeof((Array)[0]))

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

#define S(Literal) (string){(u8 *)(Literal), sizeof(Literal)-1}

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

static index C_String_Length(char *C_String)
{
   index Result = 0;
   while(*C_String++)
   {
      Result++;
   }

   return(Result);
}

static string From_C_String(char *C_String)
{
   u8 *Begin = (u8 *)C_String;
   u8 *End   = (u8 *)C_String + C_String_Length(C_String);

   string Result = Span(Begin, End);
   return(Result);
}

static char *To_C_String(arena *Arena, string String)
{
   char *Result = Allocate(Arena, char, String.Length + 1);
   memcpy(Result, String.Data, String.Length);
   Result[String.Length] = 0;

   return(Result);
}

static bool Equals(string A, string B)
{
   bool Result = (A.Length == B.Length) && (!A.Length || !memcmp(A.Data, B.Data, A.Length));
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

static string Trim(string String)
{
   return Trim_Right(Trim_Left(String));
}

static bool Has_Prefix(string String, string Prefix)
{
   bool Result = (String.Length >= Prefix.Length &&
                  Equals(Prefix, Span(String.Data, String.Data + Prefix.Length)));
   return(Result);
}

static bool Has_Suffix(string String, string Suffix)
{
   bool Result = (String.Length >= Suffix.Length &&
                  Equals(Suffix, Span(String.Data + (String.Length - Suffix.Length), String.Data + String.Length)));
   return(Result);
}

static string Remove_Prefix(string String, string Prefix)
{
   // NOTE: This assumes the existence of the prefix has already been checked.
   string Result = Span(String.Data + Prefix.Length, String.Data + String.Length);
   return(Result);
}

static string Remove_Suffix(string String, string Suffix)
{
   // NOTE: This assumes the existence of the suffix has already been checked.
   string Result = Span(String.Data, String.Data + (String.Length - Suffix.Length));
   return(Result);
}

static bool Has_Prefix_Then_Remove(string *String, string Prefix)
{
   bool Result = Has_Prefix(*String, Prefix);
   if(Result)
   {
      *String = Remove_Prefix(*String, Prefix);
   }
   return(Result);
}

static bool Has_Suffix_Then_Remove(string *String, string Suffix)
{
   bool Result = Has_Suffix(*String, Suffix);
   if(Result)
   {
      *String = Remove_Suffix(*String, Suffix);
   }
   return(Result);
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

static cut Cut_Whitespace(string String)
{
   cut Result = {0};

   if(String.Length > 0)
   {
      u8 *Begin = String.Data;
      u8 *End = Begin + String.Length;

      u8 *Cut_Position = Begin;
      while(Cut_Position < End && *Cut_Position > ' ')
      {
         Cut_Position++;
      }

      Result.Found = (Cut_Position < End);
      Result.Before = Span(Begin, Cut_Position);
      Result.After = Span(Cut_Position + Result.Found, End);
   }

   return(Result);
}

typedef struct {
   s64 Value;
   bool Ok;
} parsed_integer;

static u8 Digit_Values[256] =
{
   ['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4,
   ['5'] = 5, ['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9,

   ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,
   ['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15,
};

static parsed_integer Parse_Integer(string Number)
{
   parsed_integer Result = {0};
   Result.Ok = true;

   s64 Sign = (Has_Prefix_Then_Remove(&Number, S("-"))) ? -1 : 1;

   int Radix = 10;
   if     (Has_Prefix_Then_Remove(&Number, S("0b"))) Radix = 2;
   else if(Has_Prefix_Then_Remove(&Number, S("0o"))) Radix = 8;
   else if(Has_Prefix_Then_Remove(&Number, S("0d"))) Radix = 10;
   else if(Has_Prefix_Then_Remove(&Number, S("0x"))) Radix = 16;

   u64 Value = 0;
   for(int Digit_Index = 0; Digit_Index < Number.Length; ++Digit_Index)
   {
      u8 Digit = Digit_Values[Number.Data[Digit_Index]];
      if(Digit > (Radix - 1))
      {
         Result.Ok = false;
         break;
      }

      Value = (Value * Radix) + Digit;
   }
   Result.Value = Sign * Value;

   return(Result);
}

typedef struct map map;
struct map
{
   map *Children[4];
   string Key;
   u64 Value;
};

static u64 Hash64(string String)
{
   u64 Result = 0x100;
   for(index Index = 0; Index < String.Length; ++Index)
   {
      Result ^= String.Data[Index] & 0xFF;
      Result *= 1111111111111111111;
   }

   return(Result);
}

typedef struct {
   u64 Value;
   bool Found;
} lookup_result;

static lookup_result Lookup(map *Map, string Key)
{
   lookup_result Result = {0};

   for(u64 Hash = Hash64(Key); Map; Hash <<= 2)
   {
      if(Equals(Key, Map->Key))
      {
         Result.Value = Map->Value;
         Result.Found = true;
         break;
      }
      Map = Map->Children[Hash >> 62];
   }

   return(Result);
}

static void Insert(arena *Arena, map **Map, string Key, u64 Value)
{
   bool Found = false;
   for(u64 Hash = Hash64(Key); *Map; Hash <<= 2)
   {
      if(Equals(Key, (*Map)->Key))
      {
         (*Map)->Value = Value;
         Found = true;
         break;
      }
      Map = &(*Map)->Children[Hash >> 62];
   }

   if(!Found)
   {
      *Map = Allocate(Arena, map, 1);
      (*Map)->Key = Key;
      (*Map)->Value = Value;
   }
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
      Report_Error("Failed to open file \"%s\".", Path);
   }

   return(Result);
}

static bool Write_Entire_File(u8 *Memory, index Size, char *Path)
{
   bool Result = false;

   FILE *File = fopen(Path, "wb");
   if(File)
   {
      if(fwrite(Memory, 1, Size, File) == (size_t)Size)
      {
         // Success
      }
      else
      {
         Report_Error("Failed to write to file \"%s\".", Path);
      }
   }
   else
   {
      Report_Error("Failed to create file \"%s\".", Path);
   }

   return(Result);
}
