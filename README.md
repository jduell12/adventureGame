# adventureGame
The repository should be compiled and run in the order of buildrooms.c first followed by the adventure.c program. 

These programs run an adventure game where the user starts in one room and must enter which room to enter next. 
In order to win and exit the game the user must reach the end room. This game is played using prompts in the 
terminal. 

buildrooms.c: <p>A program that will create a new directory with a specified name followed by a random number from 0 to 100,000.</p>
              <p>The program will generate 7 room files from a possible list of 10 room names. </p>
              <p>Each room file will have the following elements: </p>
                <ul>
                <li>  A room name
                  <p>-each room will have a unique name and each name will only be used for one room file</p>
                  <p>-each name will have no more than 8 characters</p>
                </li>
                 <li> A room type
                   <p>-the possible room types are : START_ROOM, END_ROOM and MID_ROOM<p/>
                   <p>-only one room may be the START_ROOM which is different from the END_ROOM</p>
                   <p>-only one room may be the END_ROOM which is different from the START_ROOM</p>
                 </li>
  <li>Outbound connections from the room
    <p>-each room has at least 3 outbound connections to other rooms </p>
    <p>-each room has no more than 6 outbound connections to other rooms</p>
    <p>-outbound connections are assigned randomly</p>
    <p>-no room can be connected to itself </p>
    <p>-no room can have more than 1 connection to another room</p>
  </li>
                </ul>
             <p>When this program is run and compiled no output is written to the terminal. </p>
             
adventure.c: A program that represents an interface to the user. 
             It lists the current room the user is in, the possible connections and prompts for a new room name.
             If the user enters a name different from the possible connections list (even if capitalization is different but spelling is the same) the program prompts the user to re-enter a room name.
             The program keeps tracks of the number of steps the user takes as well as the path the user takes through the rooms.
             Once the user reaches the end room the program writes a congratulatory message to the terminal and tells the user how many steps they took and what their path through the rooms was.
             Once the game has exited the new directory will NOT be deleted by the program. 
