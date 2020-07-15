**MADE BY 20171662 NAYEON LEE**

1. 프로젝트 목표

이 프로그램은 SIC/XE machine의 assembly program source 파일을 입력 받아서 object 파일을 생성하고, 어셈블리 과정 중 생성된 symbol table과 결과물인 object 파일을 볼 수 있는 기능을 제공한다.


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


** 모든 명령어는 소문자로만 입력 가능하며,
   예외로 'opcode mnemonic'명령어에서 mnemonic 부분은 대문자를 사용한다.

** assemble은 오직 .asm 파일만 가능하다.
