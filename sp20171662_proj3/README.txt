**MADE BY 20171662 NAYEON LEE**

1. 프로젝트 목표

이 프로그램은 오브젝트 파일을 link시켜 메모리에 올리는 일을 수행한다. 또한 메모리에 올라간 프로그램을 실행하는 기능 또한 포함한다.


2. 프로그램 실행 방법

해당 폴더의 압축을 풀고 디렉토리에 들어간 후 make를 입력한다.
프로그램 실행은
# ./20171662.out
을 입력하면 된다.


3. 사용 가능한 명령어

h[elp]
d[ir]
q[uit]
hi[story]
du[mp] [start,end]
e[dit] address, value
f[ill] start, end value
reset
opcode mnemonic
opcodelist
assemle filename
type filename
symbol
bp
bp [address]
bp clear
run
loader [filename.obj] [filename2.obj] [...]
progaddr [address]

** 모든 명령어는 소문자로만 입력 가능하며,
   예외로 'opcode mnemonic'명령어에서 mnemonic 부분은 대문자를 사용한다.

** assemble은 오직 .asm 파일만 가능하다.

** loader 명령어는 오직 .obj 파일만 가능하다.

**run 명령어는 copy.obj 파일에 한하여 사용 가능하다.
