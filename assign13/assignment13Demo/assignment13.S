.global divide
divide:
  ldi r18, 0 //designate r18 to count the #times divided
  jmp firstDivide
  divideLoop:
    sub r24, r22
    inc r18
  firstDivide:
    cp r24, r22
    brge divideLoop
  mov r24, r18
  ret

.global sumArray
sumArray:
  mov r26, r24 //low byte of address is in r24
  mov r27, r25 //high byte of address is in r25
  ldi r20, 0 //total sum register low byte
  ldi r21, 0 //total sum register high byte
  ldi r18, 0 //index counter register
  sumArrayLoop:
    ld r19, X + //load r19 with index value
    add r20, r19 //add to total sum
    adc r21, r1 //r1 is always 0 so youre just carrying from low byte
    subi r18, -1 //update index counter
    cp r18, r22
    brlo sumArrayLoop
  mov r24, r20
  mov r25, r21
  ret

.global avgArray
avgArray:
  call sumArray

  //THIS IS PART IS MY DIVISION USING SUBTRACTION
  ldi r18, 0
  ldi r19, 0
  ldi r20, 1
  jmp firstit
  upthere:
  sub r24, r22
  sbc r25, r1
  add r18, r20
  adc r19, r1
  firstit:
    sub r24, r22
    sbc r25, r1
      brcc skip
    jmp done
  skip:
    add r24, r22
    adc r25, r1
    jmp upthere
  done:
    mov r24, r18
    mov r25, r19
  
  //THIS PART IS DIVISION USING BITWISE OPERATIONS (SHIFTING)
  
//  ldi r18, 17 //load bit counter
//  sub r19, r19 //clear registers and carry flag
//  clr r20 //r19 and r20 represent my remainder
//  myloop:
//    rol r24
//    rol r25
//    dec r18
//      breq iamdone
//    rol r19
//    rol r20
//    sub r19, r22
//    sbc r20, r1
//      brcc skip //if carry is 0 cleared go to skip skip
//    add r19, r22
//    adc r20, r1
//    clc //clear carry
//    jmp myloop
//    skip:
//      sec //set carry flag to 1
//      jmp myloop
//  iamdone:
//    nop
  ret


.global dot
dot:
  mov r26, r24 //low byte of arry a address is in r26
  mov r27, r25 //high byte of array a address is in r27
  mov r28, r22 //Y is array b
  mov r29, r23 //high  Y is array b
  ldi r18, 0 //counter
  clr r24
  clr r25
  dotloop:
  ld r30, X+
  ld r31, Y+
  mul r30, r31
  add r24, r0
  adc r25, r1
  inc r18
  cp r18, r20
    brlo dotloop
  clr r1
  ret
  
