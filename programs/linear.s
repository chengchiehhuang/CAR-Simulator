# A palindrome is a string that is exactly the same backwards and forward (e.g. anna).
# Whitespace is significant in this version, so "ahahaha" is a palindrome while "aha haha" is not.

.text
main:
	li $21, 1
	li $22, 0
big_loop:	
	li $1, 2
	li $3, 0
	la $5, string1
	la $9,  string1	# A = S
	la $10, newline1
# compute length of the string (strlen)
length_loop:
#	addi $3, $3, 1
#	lb   $11, ($10)                 # load the byte at B into $11.
#	beqz $11, end_length_loop       # if $11 == 0, branch out of loop.
#	addi $10, $10, 1                # otherwise, increment B,
#	b length_loop                   # and loop

#end_length_loop:
	subi $1, $1, 1
	addi $5, $5, 6000               # move to next string
#	subi $10, $10, 2                # subtract 2 to move B back past the '\0' and '\n'.
	
	subi $10, $10, 32
	
	li $23, 0		# offset
	li $24, 32
	addi $25, $9, 0
	addi $26, $10, 0
test_loop2:
	bge $23, $24, end_test2
	addi $23, $23, 8	
	addi $9, $25, 0
	addi $10, $26, 0
test_loop:
	bge $9, $10, end_test
	add $10, $10, $23
	ld  $11, ($9)           
	add $10, $10, $23
	ld  $12, ($10) 
	
	bne $11, $12, end_test        
	addi $9,  $9,  32                # increment A,
	subi $10, $10, 32                # decrement B,
	b test_loop                     # and loop
end_test:
	b test_loop2
end_test2:

	beqz $1, end_of_big_loop
	addi $9, $5, 0
	addi $10, $5, 0
	li $3, 0 
	b length_loop
end_of_big_loop:
	bge $22, $21, exit2
	addi $22, $22, 1
	b big_loop
exit2:        
	li $2, 10 # exit
	syscall

.data
#is_palin_msg:     .asciiz "is a palindrome.\n"
#not_palin_msg:    .asciiz "is not a palindrome.\n"
#string_space:     .space 2048 # reserve space for computation
string1:     .spacedw 1500 1111638594
newline1:     .asciiz "\n"
#end_of_string1 .word 0
string2:     .spacedw 1200 1111638594
newline2:     .asciiz "\n"
#end_of_string2 .word 0