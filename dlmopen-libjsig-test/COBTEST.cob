       IDENTIFICATION DIVISION.
       PROGRAM-ID.    COBTEST.
       AUTHOR.        OGKR.

       DATA DIVISION.
       LINKAGE SECTION.
           01 COBOL-THREAD-READY PIC X(1).
           01 COBOL-THREAD-ID PIC X(1).
           01 COBOL-THREAD-CAN-EXIT PIC X(1).
       
       PROCEDURE DIVISION USING COBOL-THREAD-ID 
                                COBOL-THREAD-READY
                                COBOL-THREAD-CAN-EXIT.
       S000-COBTEST SECTION.
       S000-00.
           DISPLAY "HELLO FROM THREAD " COBOL-THREAD-ID " (IN-COBOL)".
           MOVE "Y" TO COBOL-THREAD-READY.
       S000-10.
           IF COBOL-THREAD-CAN-EXIT = "Y"
               GO TO S000-99.
           CALL "C$SLEEP" using "1".
           GO TO S000-10.
       S000-99.
           STOP RUN RETURNING 0.
