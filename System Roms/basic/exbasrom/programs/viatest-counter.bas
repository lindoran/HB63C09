5 POKE &H8102,&HFF
10 FOR N=0 TO 255
20 PRINT "N= ";N
25 POKE &H8100,N
30 NEXT
40 GOTO 10
