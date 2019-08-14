# Introduction

The goal of our assignment was to create a shell using various techniques we
learned in class through C code implementation. We started with implementing
input and input correction. Then, we focused on executing the different commands
along with forking. We then tried adding input and output redirection, which led
us to implementing piping. 

# Implemetation
We checked input by referencing it with a static array of known commands in 
the header file, as well as checking the length and
characters in the command. Once we got the entire string, we then utilized a
struct to save the relevant information for each command. Each struct contained
the command buffer, input/output/pipeline flags, and filenames if applicable.
The way we parsed this information into the struct was by looping through
individual characters and then saving or parsing based on if the character was a
letter or a special such as a space. We utilized the arragement of the words in
command lines to determine where we would find the relevant information. For
piping, we decided to make a new struct for each pipe since the commands would
be independent of each other with the only connection being the input or output
direction. For piping and input/output redirection, we stored flags in each 
struct so that we know if the command required additional functionality, such as
storing the filename associated with the input and output. When the commands are
entered, the first commands that are checked are the built in commands since
they don't need to be forked. If the command is not in that list, our program
then proceeds to fork with the child running the new process. We made the child
check for input/output flags and implement the redirection since the child will
die anyways so changing I/O will not matter after it dies. We also added in a
wait flag in the data structure to detect a wait command which will trigger a
wait in the main function if the flag is triggered. 

# Testing
To test our output, we started by running different commands that we found on 
the project document and seeing if our output would match the the professor's
executable. When we would come across an error, we would stop and run the
command line again to see if the error was repeatable. To debug the error, we
mainly used GDB to step through the program and narrow down to where the error
was occurring. From there, we would analyze the faulty code and see how to fix
it.

# Organization
The way we divided up our program was through the phases that were specified in
the project document. We started sequentially starting with the first phase and
continued onto the next phase when finished the previous phase. We added
functions whenenever we wanted to break up a process into a simpler task, such
as getting input or parsing the command line.

# Problems and Improvements 
One of the biggest problems we had was with receiving input and parsing it.
Originally, we tried using the strtok() function in conjunction with the
fgets() function to get input and parse it by spaces, but we realized this would
not work for every input. We then tried creating our own parsing function, but
we encountered many small problems with it since there were many variations of
input that we had to account for such as spaces before and after input/output
redirection. We also had to figure out how to detect all of the different
symbols correclty. In the end, we solved all of these problems by looking at how
the input was being read and adjusting the buffer to account for any changes. We
also added in flags to our data structure to account for the unique symbols.

An improvement we can have with our implementation is that we can change our
data structure so that it organizes all of the command line information better,
making it easier to implement in our design. 
