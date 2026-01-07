# TeaGo, The Language For Teachers And Students
### Offering simple syntax, simple sarcasm, and simple errors

## Why?
For a Computer Science final in highschool, and why not?

## Syntax
### Recursion
The syntax is quite simple.<br>
```do``` is the same as "repeat _ _ _ _, _ _ _ _ times" - recursion!<br>
A do range must have a number of times to do something. To repeat something 5 times, you can do<!--heh.. do C:-->:<br>
```do 5```
After that, you put any actions to do inside the do range.<br>
But that's not valid just yet!<br>
We need a stop!<br>
```do 5 stop```
this will do nothing, 5 times!<br>
Let's make it say "HAI!" 5 times. But, on a new line.<br>
```
do 5
  write "HAI!\n"
stop
```
The entire program can also be written on one line.<br>
The interpreter uses spaces and new lines as seperators.

### Making variables
Something very important to know, is this language can only have 36 variables at once.<br>
There is no type safety.<br>
All variables are always mutable.<br>

To make a variable, like ```var``` and set it to 32, the syntax is ```teach var 32```.

### Printing
To print, the syntax is simply ```write```.<br>
You can print integers, strings, or variables.<br>
There is no default endline.<br>
The only supported escape sequences, are \" and \n.

### Ending a program
You must put "end" at the end, or have some end sequence it comes across.
