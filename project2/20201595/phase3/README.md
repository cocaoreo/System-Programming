[system programming lecture]

-project 2 phase3

csapp.{c,h}
        CS:APP3e functions

myshell.c
        작성한 shellcode 파일

Makefile
        make 명령어를 통해 실행파일 생성
        make clean 명령어를 통해 실행 파일 제거

파일 실행
        ./myshell을 입력해 실행

myshell 명령어

        cd: 디렉토리 이동
        ls: 현재 디렉토리에 있는 파일, 디렉토리, 실행 파일 출력
        mkdir directoryname: directoryname을 가지는 디렉토리 생성
        rmdir directoryname: directoryname을 가지는 디렉토리 삭제
        touch, cat, echo: 파일을 생성, 읽기, 출력을 담당하는 명령어
        exit: shell 종료
        quit: shell 종료
        grep filename: 파이프라인을 통해 들어온 입력 중 filename에 해당하는 것만 출력
        less: 파일/명령의 출력 내용을 한 페이지 씩 표시
        sort: 결과를 정렬
        jobs: 현재 background에 존재하는 프로세스와 그 상태를 표시
        fg %n: jobs 리스트에 있는 index가 n인 background 작업 foreground로 변경
        bg %n: jobs 리스트에 있는 index가 n인 stop 상태의 background 작업 running으로 변경
        kill %n: jobs 리스트에 있는 index가 n인 프로세스 강제 종료
        
        -pipe
        '|'을 입력해 사용 가능
        
        <사용 예시>
        ls | grep "abc"
        ls | grep "abc" | sort

        -background 실행
        명령의 맨 마지막에 '&'를 붙여서 실행 가능
        ctrl+c를 통해 작업 강제 종료
        ctrl+z를 통해 작업 stop으로 변경 후 background 리스트에 저장


        <사용 예시>
        ./a.out & 
        (이 때 ./a.out은 jobs 리스트에 1번 인덱스로 저장됨.)
        
        fg %1을 통해 ./a.out을 foreground 작업으로 변경
        ctrl+z를 통해 stop 변경 후 background에 저장

        bg %1을 통해 stop 상태의 ./a.out background에서 running으로 변경
