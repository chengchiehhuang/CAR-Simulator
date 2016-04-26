# A palindrome is a string that is exactly the same backwards and forward (e.g. anna).
# Whitespace is significant in this version, so "ahahaha" is a palindrome while "aha haha" is not.

.text
main:
#load matrix size
	la $1, matrix_size	 # 40*4 x 40*4 matrix
	ld $14, ($1)
#=================================PRINT MATRIX==================================
	la $4, input_msg
	li $2, 4
	syscall
	li $15, 0     # i
	li $16, 0     # j
	la $13, input1	
read_loop_i:
	bge $15, $14, read_break_loop_i
read_loop_j:
	bge $16, $14, read_break_loop_j
	li $20, 0     # sum	

# output: i * 40 + j => $15 * $14 + $16
	mult $21, $15, $14
	add $21, $21, $16
	multi $21, $21, 4   #DWORD
	add $21, $13, $21	#output 
	ld $20, ($21)
	
	addi $4, $20, 0
	li $2, 1
	syscall	
	
	li $4, 32
	li $2, 2
	syscall
	
	addi $16, $16, 1
	b read_loop_j
read_break_loop_j:
	li   $16, 0	
	
	# new line
	li $4, 10
	li $2, 2
	syscall
	li $4, 13
	syscall	
	
	addi $15, $15, 1
	b read_loop_i
read_break_loop_i:
#=================================PRINT MATRIX==================================
	la $4, input_msg
	li $19, 50
	sb 13($4), $19
	li $2, 4
	syscall
	
	li $15, 0     # i
	li $16, 0     # j
	la $13, input2	
read2_loop_i:
	bge $15, $14, read2_break_loop_i
read2_loop_j:
	bge $16, $14, read2_break_loop_j
	li $20, 0     # sum	

# output: i * 40 + j => $15 * $14 + $16
	mult $21, $15, $14
	add $21, $21, $16
	multi $21, $21, 4   #DWORD
	add $21, $13, $21	#output 
	ld $20, ($21)
	
	addi $4, $20, 0
	li $2, 1
	syscall	
	
	li $4, 32
	li $2, 2
	syscall
	
	addi $16, $16, 1
	b read2_loop_j
read2_break_loop_j:
	li   $16, 0	
	
	# new line
	li $4, 10
	li $2, 2
	syscall
	li $4, 13
	syscall	
	
	addi $15, $15, 1
	b read2_loop_i
read2_break_loop_i:
#=================================Compute MATRIX==================================
	
	la $4, compute_msg
	li $2, 4
	syscall
	
	la $11, input1	# A = S
	la $12, input2	# we need to move B to the end
	la $13, output
	
	li $15, 0     # i
	li $16, 0     # j
	li $17, 0     # k


loop_i:
	bge $15, $14, break_loop_i
loop_j:
	bge $16, $14, break_loop_j
	li $20, 0     # sum	
loop_k:
	bge $17, $14, break_loop_k
	li $21, 0     # tmp_addr	
	li $22, 0     # value for tmp_addr 1
	li $23, 0     # value for tmp_addr 2	
#input 1  i * 40 + k => $15 * $14 + $17
	mult $21, $15, $14
	add $21, $21, $17
	multi $21, $21, 4   #DWORD
	add $21, $11, $21	#input 1
	ld $22, ($21)
#input 2  k * 40 + j => $17 * $14 + $16
	mult $21, $17, $14
	add $21, $21, $16
	multi $21, $21, 4	# DWORD	
	add $21, $12, $21	# input2
	ld $23, ($21)
	
	mult $22, $22, $23	# V1 * V2
	add $20, $20, $22   # sum += V1*V2
	
#	li $18, 0     # global index
#	add $18, $18, $15
#	multi $18, $18, 40   # $9=temp
#	add $18, $18, $16
#	multi $18, $18, 40   # $9=temp
#	add $18, $18, $17
#	addi $4, $18, 0
#	li $2, 1
#	syscall
	
	addi $17, $17, 1
	b loop_k
break_loop_k:

	li   $17, 0	
# output: i * 40 + j => $15 * $14 + $16
	mult $21, $15, $14
	add $21, $21, $16
	multi $21, $21, 4   #DWORD
	add $21, $13, $21	#output
	sd ($21), $20
	
	addi $16, $16, 1
	b loop_j
break_loop_j:
	li   $16, 0
	addi $15, $15, 1
	b loop_i
break_loop_i:
	la $4, done_msg
	li $2, 4
	syscall
	
#=================================PRINT MATRIX==================================
	la $4, output_msg
	li $2, 4
	syscall
	
	li $15, 0     # i
	li $16, 0     # j
	la $13, output	
read3_loop_i:
	bge $15, $14, read3_break_loop_i
read3_loop_j:
	bge $16, $14, read3_break_loop_j
	li $20, 0     # sum	

# output: i * 40 + j => $15 * $14 + $16
	mult $21, $15, $14
	add $21, $21, $16
	multi $21, $21, 4   #DWORD
	add $21, $13, $21	#output 
	ld $20, ($21)
	
	addi $4, $20, 0
	li $2, 1
	syscall	
	
	li $4, 32
	li $2, 2
	syscall
	
	addi $16, $16, 1
	b read3_loop_j
read3_break_loop_j:
	li   $16, 0	
	
	# new line
	li $4, 10
	li $2, 2
	syscall
	li $4, 13
	syscall	
	
	addi $15, $15, 1
	b read3_loop_i
read3_break_loop_i:

	
exit:
	li $2, 10 # exit
	syscall

.data
matrix_size:	.word 10
input1:     .spacedw 100 2
input2:     .spacedw 100 3
output:     .spacedw 100 0
input_msg:     .asciiz "Input Matrix 1 : \n"
compute_msg:   .asciiz "Compute ... "
done_msg:      .asciiz "Done!\n"
output_msg:    .asciiz "Output Matrix : \n"
