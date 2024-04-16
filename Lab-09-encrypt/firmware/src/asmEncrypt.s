/*** asmEncrypt.s   ***/

#include <xc.h>

// Declare the following to be in data memory 
.data  

/* create a string */
.global nameStr
.type nameStr,%gnu_unique_object
    
/*** STUDENTS: Change the next line to your name!  **/
nameStr: .asciz "Bijan Moradi"  
 
/* initialize a global variable that C can access to print the nameStr */
.global nameStrPtr
.type nameStrPtr,%gnu_unique_object
nameStrPtr: .word nameStr   /* Assign the mem loc of nameStr to nameStrPtr */


.type cipherText,%gnu_unique_object

.align
// space allocated for cipherText: 200 bytes, prefilled with 0x2A */
cipherText: .space 200,0x2A  
 
.global cipherTextPtr
.type cipherTextPtr,%gnu_unique_object
cipherTextPtr: .word cipherText

// Tell the assembler that what follows is in instruction memory    
.text
.align

// Tell the assembler to allow both 16b and 32b extended Thumb instructions
.syntax unified

    
/********************************************************************
function name: asmEncrypt
function description:
     pointerToCipherText = asmEncrypt ( ptrToInputText , key )
     
where:
     input:
     ptrToInputText: location of first character in null-terminated
                     input string. Per calling convention, passed in via r0.
     key:            shift value (K). Range 0-25. Passed in via r1.
     
     output:
     pointerToCipherText: mem location (address) of first character of
                          encrypted text. Returned in r0
     
     function description: asmEncrypt reads each character of an input
                           string, uses a shifted alphabet to encrypt it,
                           and stores the new character value in memory
                           location beginning at "cipherText". After copying
                           a character to cipherText, a pointer is incremented 
                           so that the next letter is stored in the bext byte.
                           Only encrypt characters in the range [a-zA-Z].
                           Any other characters should just be copied as-is
                           without modifications
                           Stop processing the input string when a NULL (0)
                           byte is reached. Make sure to add the NULL at the
                           end of the cipherText string.
     
     notes:
        The return value will always be the mem location defined by
        the label "cipherText".
     
     
********************************************************************/    
.global asmEncrypt
.type asmEncrypt,%function
asmEncrypt:   

  
    push {r4-r11,LR}
    
/* YOUR asmEncrypt CODE BELOW THIS LINE! VVVVVVVVVVVVVVVVVVVVV  */
    /*NOTE, there was an issue with some of the comments above turning everything orange in the IDE, so I removed them*/
    LDR r8,= cipherText /*load in the pointer to the destinatioin ciphered text*/
    cipherLoop:
	LDRB r7,[r0], 1 /*load the first byte from string pointer in r0, post increment by 1*/
	CMP r7,0 /*check if we have our null terminator*/
	BEQ done /*null terminator case, we are done*/
	CMP r7,'Z' 
	BGT checkLower /*the ascii value is greater than Z, so we need to check if its within lowercase a-z*/
	CMP r7, 'A' /*check its range, is it between A-Z?*/
	BLT dontChange /*not in the cipher range, dont modify*/
	ADD r7,r7,r1 /*add our shift*/
	CMP r7, 'Z' /*check if we need to wrap around to A*/
	BLE store /*it is still within range A-Z, so do not wrap around*/
	SUB r7,r7,26 /*we passed Z, subtract 26 so we wrap around as per ceaser cipher in slides*/
	
	checkLower: /*lets check if we have a lowercase letter*/
	    CMP r7, 'a' /*check its range, is it between a-z?*/
	    BLT dontChange
	    ADD r7,r7,r1 /*add our shift*/
	    CMP r7, 'z' /*check if we need to wrap around to a*/
	    BLE store
	    SUB r7,r7,26 /*we passed z, subtract 26 so we wrap around as per ceaser cipher in slides*/
	
	store:
	    STRB r7,[r8],1 /*store our modified byte then post increment*/
	    B cipherLoop /*loop into next char, continue until null terminator*/
	
	dontChange:
	    STRB r7,[r8],1 /*char is not in cypher range. dont modify*/
	    B cipherLoop
    done:
	MOV r5,0 /*set r5 to the null byte*/
	STRB r5,[r8] /*store our null byte*/
	LDR r0,= cipherText /*pass our memory location in cypher text into r0 as defined in function desc*?
    
    /* YOUR asmEncrypt CODE ABOVE THIS LINE! ^^^^^^^^^^^^^^^^^^^^^  */

 
	
    pop {r4-r11,LR}

    mov pc, lr	 /* asmEncrypt return to caller */
   

/**********************************************************************/   
.end  /* The assembler will not process anything after this directive!!! */
           




