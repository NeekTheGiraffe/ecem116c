addi x1, x0, -345
addi x2, x0, 240
sw x1, 4(x2)
addi x2, x0, 800
lw x10, -556(x2)
# expect: (-345,0)
# The python module has a bug for this test
# Please convert line 10 of the output from 36 -> 34
