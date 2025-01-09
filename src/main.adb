--------------------------------------------------------------------------------
-- (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////////
--------------------------------------------------------------------------------

with Ada.Text_IO;      use Ada.Text_IO;
with Ada.Command_Line; use Ada.Command_Line;
with Interfaces;       use Interfaces;

with Ada.Sequential_IO;

procedure Main is
   package Instruction_IO is new Ada.Sequential_IO (Unsigned_8);
   use Instruction_IO;

   File : Instruction_IO.File_Type;
   Byte : Unsigned_8;
begin
   Put_Line ("ASM SIMULATOR V0.0");

   if Argument_Count = 1 then
      Open (File, In_File, Argument (1));

      while not End_Of_File (File) loop
         Read (File, Byte);
         Put_Line (Byte'Image);
      end loop;
   end if;
end Main;
