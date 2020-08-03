# POSIX-Message-Queue-Example
### POSIX-Message-Queue-Example

다음은 POSIX Message Queue에서 발생할 위험 요소를 소개한다.
#### Message Queue의 이름
Message Queue의 이름을 프로세스 PID 또는 PID와 관련된 값으로 지을 경우, 프로세스를 생성할 때마다 Message Queue가 생성되어, 
`Too many files open` 에러가 발생할 수 있다.

***Many_mq*** 예제의 경우,
  프로세스를 실행할 때마다 thread를 통해 200개의 Message Queue가 생성되며, 각 서버와 클라이언트 Thread는 0부터 99까지의 값을 공유한다.

이때, 서버에서 Message Queue를 생성할 때 서버의 PID와 Thread 번호를 조합하여 생성한다. 생성한 Message Queue 이름은 input.db 파일을 통해 공유했다.

***Share_text*** 예제의 경우,
 Thread를 사용하지 않고 서버의 PID로 Message Queue를 생성하며, 서버와 클라이언트 프로세스를 실행할 때마다 서버의 PID에 따라 Message Queue가 추가된다. 서버와 클라이언트는 클라이언트가 입력한 문자를 공유한다. 마찬가지로, 생성한 Message Queue 이름은 input.db 파일을 통해 공유했다.

#### `Too many files open`  해결방법

- `ulimit -q` command를 통해 Message Queue에 사용될 용량을 조절할 수 있다.

> `ulimit -q unlimited`를 통해 무제한으로 설정할 수 있다.
- `/dev/mqueue` 디렉토리 내 Message Queue를 삭제한다.
> 아래 command를 통해 /dev/mqueue 내 Message Queue를 마운트할 수 있다.
> `mkdir /dev/mqueue`
> `mount -t mqueue none /dev/mqueue`
>
> /dev/mqueue 내 Message Queue![](https://lh4.googleusercontent.com/g42YkTkZZxMy5KLmebxZov4bX6kYzhdw65KMTKStHnCl1jSiDEqLuedDFzQsPINauqFp6gBLK8-couyDHUIffwGSQNhhz-KGZWbx06ISP2y8sHb6z0tpBQCTnQ8-K3xmYoLubdug)

- `mq_unlink((const  char *)mq_name)` 코드를 추가하여 서버가 종료할 때 메세지 큐를 삭제한다.

 
