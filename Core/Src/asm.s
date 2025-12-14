.text 

.global rgb565_to_lcd
.code 16
.syntax unified

/***************************************************/
.type rgb565_to_lcd, function
rgb565_to_lcd:
push {r2-r4}

/*r0=char*buff, r1=lens*/
/*r3=data, r2=tmp data*/

ldr r4, =0x60020000
and r1, r1, #0xfffffffe;

compute_color:
/*color=(bf[i]<<8)|bf[i+1];*/
ldrh r3, [r0], #2
rev16 r3, r3

/*bl lcd_w16*/
strh r3, [r4]

/*len--*/
subs r1, r1, #2
cmp r1, #0
bne.n compute_color

pop {r2-r4}
bx lr

/***************************************************/
