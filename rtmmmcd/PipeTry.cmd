/* REXX */
'@echo off'                       /* Don't echo commands */
say " REXX Module Command Pipe Tester V1.0                                     Ethos"
   call OpenPlayer
   if Result <> 0 then do
      say "Error" Error
      return
   end
   say "Command pipe opened to" Type || ". Command set Version" Version
   say "Type quit to exit"

   do while (1)
      Line = linein();
      if translate(Line) = "QUIT" then return
      if translate(Line) = "EXIT" then return

      parse var Line A B C

      if CallPlayer(A,B,C) <> 0 then do
         say "Error" Error
         iterate;
      end

   end
return

CallPlayer: procedure expose Result. Player Error
   parse arg Command, SCommand, Arg

   Error = ""
   Result.0 = 0

   /* Player not loaded */
   if (Player = "") then do
      Error = "Player not Inited"
      return 1
   end

   Say sending
   call lineout Player,Command SCommand Arg
   Say sent
   Line = linein(Player)
   Say received

   say Line

return 0

OpenPlayer:procedure expose Player Version Type Error
   Error = ""

   Result = Stream("\pipe\RTMMMCD","C","OPEN");
   if (Result <> "READY:") then do
      Error = "Unable to communicate with the  Player, is it running?"
      Player = ""
      return 1
   end
   Player = "\pipe\RTMMMCD"

return 0
