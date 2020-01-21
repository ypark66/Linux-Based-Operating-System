USER CODE RUNNING ORIGINATES IN ece391shell.c!!!!!!!!!!!!!

TODO:

Fix Terminal_Read such that it waits on a newline being pressed before returning.
Right now, I just make the terminal driver call it when ENTER is pressed,
but the user code will call it immediately and expect it to wait until the buffer
is valid.

Current bug is that on returning from sys_write, a PAGE FAULT occurs. The linkage
is probably not the issue since sys_execute returns.

Make the file function signatures match the function pointer signatures in the FOT.
Check Terminal.c's read/write functions to see the proper way to cast after doing this.

Need to verify: 
sys_write: Works for terminal -- check on RTC, files
sys_read: Works for files, terminal -- check on RTC.
sys_open:
sys_close:
sys_halt:

If child programs cause exceptions, make sure that sys_execute is generating the right
process number! After shell starts, the process number should be 1.


-------------------------------------------------
UPDATED (4/8)

Right now, the execute branch has halt pushed. 
When you try running it, you get a PAGE FAULT with no "starting 391 shell" line, 
which is supposed to be the first line that should be printed out. 

So mbranch is without halt function implemented, 
so you can test and debug with branch code first. 
When we fix terminal stuff, then we can work on halt.

Also, for rtc write function, I tried to correct the function signatures 
but I am not sure what the frequency is so could not really work on it.
*****RTC FUNCTIONS ARE ALREADY FINE, I FIXED ALL FUNCTIONS EXCEPT FILE SYSTEM

Will write test cases and correct the function signature stuff tomorrow. 
I don't think halt is working right tho. 
A few lines of code is added by Phillip in the execute function, 
and I think they are causing page fault.

------------------------------------------------------------------------------------------
UPDATED (4/27)

TODO:

Current bugs, as far as I know, are ALL in scheduling. The entirety of the scheduling functions
are in task_switch.c. The scheduler is performing the round-robin switching correctly (meaning
that it goes 0, 1, 2, 3, 0, 1, 2, 3...). It skips servicing a shell if that shell has a child 
process (which is the correct thing to do). The variable next_proc_num is all you need to check
if you think that the wrong process is being serviced next.

Bug I am working on fixing right now is that pingpong writes on whatever terminal is active,
even though the VGA appears to be remapped correctly. My guess is that I AM indeed remapping
VGA correctly, but that maybe the program data being pointed to is wrong.

Other than that, there is a lot of random crashing with various programs, and I think this is most
likely due to failure to deallocate certain variables or wrong book-keeping info. That is all I
know for now.



BUG:	fish shows up in other terminal when run in terminal 2 and 3

BUG:	fish crashes in terminal 1, and fish crashes on halt in terminal 2 and 3

BUG:	screen leaking

BUG:	after hello gets executed as a 5th program, if I swap terminals, hello halts even if I don't type response.

BUG:	after 5th program gets executed, the last free shell does not get executed

Every program halts normally except hello and fish.
Im assuming rest of the bugs are related to video mapping and video memory.

--------------------------------------------------------------------------------------------------------
UPDATED (4/29)

TODO:

syserr and fish does not work. check these
pingpong leakage error needs to be fixed.
We've fixed cursor and no such command crush error.
