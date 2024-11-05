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


You will need to specify 4 layers, and give them layers in the following order (if ordering from PCB Way)
  TopLayer
  InnerLayer1
  InnerLayer2
  BottomLayer

This revision Is shown to work with the ahc chipset, there are two bom files with either Mouser 
or Digikey part numbers. you only need one.  Each BOM has a section at the bottom containing 
parts you have to go and find 

URL's subject to change, inaccuracy and fluxuating prices please do your own research before buying: 

  63C09P listings that have yealded sucsessfully optained parts:

      https://www.ebay.com/itm/392620303721  
        - This is a listing for ct (10) cpu's for around 1 dollar a piece.  
        they are pulls.  So far none of these have been fake. ( but you have to order 10)

      https://www.utsource.net/itm/p/1375441.html
        - this is a utsource listing, here you have to buy 2 - they are aproximately 2 
        with shpping cost.  Have not tried to order these yet but Have had amaizingly good luck 
        with UTSource pulls.  They have gardrails in place to asure you are getting what you are 
        paying if its listed as utsource used its verified pulls that being said I have never ordered
        this specific part. 

        This is one of the major reasons to group buy or kit buy.  I am sorry for this, its not impossible to 
        get one of these but it might be tricky to track down count (1) from a verified source.  Additionally
        since the parts markets are kind of tricky, even if one seller gave you good parts in the past 
        its not always the case ordering a second time.

  HW-125 (sd card module)
      here are a few listings for the sd card reader module (HW-125 on the BOM)
      https://www.ebay.com/itm/191840080086
      https://www.amazon.com/WWZMDiB-Adater-Module-Support-Arduino/dp/B0B779R5TZ
      https://www.aliexpress.us/item/3256806721966749.html

  FTDI adapter: 
      This is the serial to TTL adapter which has a USB - Mini connection on it.
      pin out on the boards serial connection matches this specific adapter
      https://www.aliexpress.us/item/3256806833296462.html
      https://www.ebay.com/itm/313612877526
      https://www.amazon.com/HiLetgo-FT232RL-Converter-Adapter-Breakout/dp/B00IJXZQ7C/

  USBTinyISP:
      This is only needed if you have to program the bootloader.
      Kitted boards come with the bootloader pre-loaded so no need to buy this unless you
      brick your controller (which won't typically happen programming from serial)
      https://www.amazon.com/Geekstory-USBtinyISP-downloader-microcontroller-Programming/dp/B0C1V1HLPK/
      https://www.ebay.com/itm/226245863722
      https://www.aliexpress.us/item/3256806685302032.html

An updated build guide is here : https://hackaday.io/project/193108/instructions?page=1
an older one can also be found in the programmers refrence in the documents folder of this repo

Untested 6809E Adapter:

There exists an interposer for connecting a 63C09E into a 63C09 socket, which builds the clocks on the interposer.  
These are prohibitively expensive, but probibly will work.

I simply can't justify the price at this time even to test one.

I have considered creating such a cost reduced interposer to work with both the 40pin dip and the plcc cpu as well.
I think it may be very inexpensive leveraging asembly, and panelization. 

if anybody is interested the interposer can be bought here: 
https://www.ebay.com/itm/185241563587

