5 POKE &H8102,&HFF
10 I=RND(255)
20 FOR J=0 TO 255 : NEXT J
25 POKE &H8100,I
40 GOTO 10
50 END
