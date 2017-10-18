# Strange_game
prepareGame.c 
This program creates directory with 7 files with rooms description. Names for the rooms are chosen randomly from 
10 pre-defined options. Each room has 3 to 6 connections to other rooms.
This program adapts pseudocode from "2.2 Program Outline"

theGame.c
this program finds the newest sub-directory created by prepareGame.c, reads contet of those files, and start a game where user 
needs to find a way from start room to end room. The program will let you know how many steps it took to find the end room. 
And if user prints "time" during the game, s/he'll get the current time 
