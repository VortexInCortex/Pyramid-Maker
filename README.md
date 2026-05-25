# Description

Simple CMD program which displays a pyramid in ASCII, and spins it around! Should work for Linux, though untested.

# Running the Porgram

To use the program, run the .exe in a terminal window, and adjust your terminal window setting to show 41 rows and 156 columns, or manually resize the terminal
window until it looks right.

# Technical Challenges

This little program leverages a big buffer of a few KB to render the whole frame at once. Also, instead of clearing the screen with system("cls"), it resets the
cursor's position to the start of the frame. These two performance optimizations
turned the program from a sluggish and choppy animation to one that displays properly!<br>
<br>
To also note, this program had to apply basic rasterization techniques to transform shapes we can easily draw with mathematical equations to an image on a
screen.