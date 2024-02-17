.section        __TEXT,__text,regular,pure_instructions
.globl  _log_2
.p2align        4, 0x90

_log_2:                            
        
        pushq   %rbp            // leave this alone
        movq    %rsp, %rbp      // leave this alone

        // Check if n is zero
        testq   %rdi, %rdi      // check if n = 0
        jnz     PROCEED         // if not, jump to PROCEED
        movq    $-1, %rax       // otherwise, put -1 in %rax
        jmp     DONE            // and jump to DONE

PROCEED:
        xorq    %rax, %rax      // initialize the result (%rax) to 0

LOOP_TOP:
        shrq    %rdi            // shift n right by 1 to divide by 2
        testq   %rdi, %rdi      // compare n to 0
        jz      DONE            // if n = 0, we're done, jump to DONE
        incq    %rax            // otherwise, increment the result
        jmp     LOOP_TOP        // and jump to the top of the loop

DONE:
        popq    %rbp            // leave this alone
        retq                    // leave this alone
