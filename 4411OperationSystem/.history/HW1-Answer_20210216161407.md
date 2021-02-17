## HW1 Answer
Zhiyuan Lu, M91140491

### 1
This is due to that the current state on kernel level should be saved and avoids that the process causes unsafe modification of the kernel’s return address.

### 2
(1). This instruction is used when system return control back to user program. User program continues its execution from the last interrupt point. One example is back to former process after context switch. 
(2). Exception will happen.

### 3
Processes will be created continuously. The number of processes increases exponentially. When it reaches the resource bottleneck, like memory, the running speed will be slowed down.

### 4
1+2+4+8+16=31

### 5
Program1 output:
```
"6"
"6"

```
Program2 output:
```
"6"

```

### 6
| Task | FIFO Complete| FIFO Response | RR Complete | RR Response | SJF Complete | SJF Response |
| :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| 0 | 85 | 0 | 220 | 0 | 85 | 0 |
| 1 | 115| 75 | 80 | 0 | 135 | 95 |
| 2 | 150| 100 | 125 | 5 | 170 | 120 |
| 3 | 170| 70 | 145 | 20 | 105 | 5 |
| 4 | 220| 85 | 215 | 25 | 220 | 85 |
|AVG| | 66| | 10 | | 61|

### 7
| Task | 1) | 2) | 3) | 4) | 5) |
| :-: | :-: | :-: | :-: | :-: | :-:|
| A | 100 | 140 | 100 | 140 | 104 |
| B | 200 | 121 | 200 | 120 | 194 |
| C | 300 | 122 | 300 | 121 | 196 |

### 8
This process run more one scheduling quantum. Consequently, this process is advanced, and others are slowed down. This case could happen, if the system allows prioritized process and this process gets the priority.

### 9
(a) (b) are  T/(T+S)
(c) (d) (e) are Q/(Q+S). Specifically, (d) is 1/2, (e) is 0.