This is the newest untested PCB Layout with the following changes: 

    - Power and ground planes were brought to the bottom and top of the boards 
      this actually had the oposite effect of creating more nose due to top side 
      trace density. these issues were mostly resolved in 4.1 changeing passive 
      values and the board is more or less working but I wanted to clean up the 
      design a bit so that issue was addressed in rev 4.2

    - Reduction of transfer vias for the top and bottom planes might reduce some
      edge case production issues.  (will know when I get the boards back)

    - The tie up pin for the active high ram chip signal requires manual soldering, 
      the process requires a small ammount of skill (not difficult but requires
      bridging two pads on the board.)  This is resolved with a trace jumper in 4.2.

      The down side to this is that it may generate an engeneering question, as the
      net will appear shorted to +5V, and will generate a DRC fail.  A note in the 
      design order should resolve this, but I will report back if it does not.
      
    - added ground pads for scoping some high speed signals which are closer to the 
      signals on the board.

    - added more top side trace pads for the banking and ram chip so that it's 
      easier to experiment with making a ram expansion. on the SMD board, this 
      will have a deticated header but we needed the space for other things since 
      this is a through hole board. 

    - moved the mounting hole nearest to the sd interface back to allow the 
      use of different mounting hardware. Expansions will take this hole placement
      in mind. I did move it back only a mil or two so if you end up with a 4.1
      board, a spacer stand off will fit just fine, the expansion will just 
      only be tied to the 3 pegs which didn't change in that case ( probibly 
      not a huge issue.) In this design you need to trim the corner hole on the
      sd card interface exactly like the SD interface on the V20-MBC, 4.2 resolves
      this.  

These are currently out for production, we should know in a week or so if they are 
verified.  I have no reason to asume that they will not work but I have not done a 
test run.  please order at your own risk.
