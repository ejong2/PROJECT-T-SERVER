# **TCP Database Connection Project**

이 프로젝트는 개인적인 학습과 연구를 위해 제작된 TCP 및 DB 연결 프로그램입니다. 프로젝트의 주요 목표는 TCP 프로토콜을 이용해 데이터베이스와 통신하는 애플리케이션을 구현하는 것이며, 그 과정에서 네트워크 프로그래밍에 대한 깊이 있는 이해를 도모하는 것입니다.

현재는 간단한 사용자 인증 시스템(회원 가입 및 로그인)을 통해 서버와 클라이언트 간의 통신을 구현하였지만, 앞으로는 언리얼 엔진의 FSocket을 이용한 TCP 통신을 구현하는 프로젝트로 발전시킬 예정입니다.

## **주요 기능**

1. TCP 프로토콜을 이용한 클라이언트-서버 통신: 사용자 인증 정보를 클라이언트에서 서버로 전송하고, 서버는 이를 처리하여 응답 메시지를 반환합니다.
2. 사용자 인증: 사용자의 회원 가입 및 로그인 요청을 처리하는 메커니즘을 구현하였습니다. 이를 통해 기본적인 TCP 통신의 동작 방식을 이해할 수 있습니다.
3. 클라이언트-서버 연결 관리: 클라이언트의 연결 요청을 서버가 수락하고, 서버와 클라이언트의 연결 상태를 관리합니다.

## **구조 및 작동 원리**

프로젝트는 기본적으로 'GameServer.cpp'와 'DummyClient.cpp' 두 가지 주요 컴포넌트로 구성되어 있습니다. 'GameServer.cpp'는 서버 로직을, 'DummyClient.cpp'는 클라이언트 로직을 담당합니다.

- 클라이언트는 사용자의 인증 정보를 입력 받아 서버로 전송하고, 서버의 응답을 처리합니다.
- 서버는 클라이언트로부터의 요청을 받아 처리하고, 결과를 클라이언트에게 응답합니다.

자세한 동작 원리 및 코드 설명은 각 파일 내에 있는 주석을 참고해 주세요.

## **향후 계획**

앞으로는 언리얼 엔진의 FSocket을 활용하여 게임 클라이언트와의 TCP 통신을 구현할 계획입니다. 이를 통해 게임 서버와 클라이언트 간의 실시간 통신 및 데이터 동기화 등, 게임 네트워킹에 필요한 다양한 기능을 학습하고 구현해볼 예정입니다.
