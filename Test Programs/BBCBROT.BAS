02 CD = 0
05 LCD = 0
10 X1=59
11 Y1=21
20 I1=-1.0
21 I2=1.0
22 R1=-2.0
23 R2=1.0
25 ACIA = &A001 :    REM *** INTERFACE ADDRES FOR WRITES ***
26 DATA 27,91,50,74 :  REM *** '<esc>[2J' / Clears the Screen
27 FOR A = 1 TO 4:READ SENDBYTE:?ACIA =SENDBYTE:NEXT A
30 S1=(R2-R1)/X1
31 S2=(I2-I1)/Y1
40 FOR Y=0  TO Y1
50 I3=I1+S2*Y
60 FOR X=0 TO X1
70 R3=R1+S1*X
71 Z1=R3
72 Z2=I3
80 FOR N=0 TO 29
90 A=Z1*Z1
91 B=Z2*Z2
100 IF A+B>4.0 GOTO 130
110 Z2=2*Z1*Z2+I3
111 Z1=A-B+R3
120 NEXT N
130 CD = 62-N : GOSUB 2000 : REM *** SEND CHAR TO TERMINAL, CALC VALUE ***
140 NEXT X
150 ?ACIA = 10 : ?ACIA = 13 :REM *** STARTS THE NEW LINE ***
160 NEXT Y
161 ?ACIA = 27 : ?ACIA = 91 : ?ACIA = 51 : ?ACIA = 55 : ?ACIA = 109
170 END
2000 IF CD = LCD THEN GOTO 2010 : REM ** COLOR HAS NOT CHANGED ***
2001 ?ACIA = 27 : ?ACIA = 91 : REM *** '<esc>[' folowing selects color ***
2002 IF CD =  32               THEN ?ACIA = 51 : ?ACIA = 55
2003 IF CD >= 33 AND CD <  42  THEN ?ACIA = 51 : ?ACIA = 49
2004 IF CD >= 42 AND CD <  51  THEN ?ACIA = 51 : ?ACIA = 50
2005 IF CD >= 51 AND CD <  60  THEN ?ACIA = 51 : ?ACIA = 51
2006 IF CD = 60 THEN ?ACIA = 51 : ?ACIA = 52
2007 IF CD >= 61 OR CD = 62 THEN ?ACIA = 51 : ?ACIA = 53
2009 ?ACIA = 109 : REM *** 'm' (finishes control code for color)
2010 ?ACIA = (CD) : REM *** Sends the actual character to the screen ***
2011 LCD = CD
2020 RETURN
