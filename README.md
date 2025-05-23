# 🎮 땅따먹기 콘솔 게임
> Paper.io를 모티브로 하여 제작한 콘솔 기반 2인용 땅따먹기 게임 (C 언어)

## 📌 주요 기능
- 2인용 실시간 대전: 방향키 (Player1), WASD (Player2)
- 점령한 영역 수에 따라 승패 판정
- 3종 아이템: 속도 증가 (♥), 속도 감소 (↓), 양측 멈춤 (※)
- 배경음 및 효과음(스페이스바, 엔터키 입력 시) 지원
- 플레이어별 색상 커스터마이징 가능

## 🧩 규칙
- 제한 시간 60초 안에 더 많은 영역을 차지한 플레이어가 승리
- 상대방의 영역과 중복하여 이동 불가
- 차지한 영역의 수에 따라 점수가 계산

## 🎨 스크린샷
1. **초기 화면:** 게임 시작 / 게임 설명 / 게임 종료 중 선택
![image](https://github.com/user-attachments/assets/a93abbc8-2ee6-4095-a6c2-74b86fdeff85)

2. **게임 설명:** 방향키 및 아이템 안내, 게임 규칙 제공
![image](https://github.com/user-attachments/assets/c990d1d1-2fc4-4332-9d76-9397af522210)

3. **게임 시작:** Player1과 Player2 각각 원하는 색상 설정
![image](https://github.com/user-attachments/assets/37c680aa-a757-4a90-9b8f-9ef56d75019a)
![image](https://github.com/user-attachments/assets/47054049-2034-4a00-934c-386054aa13e3)

4. **게임 진행 화면:** 실시간 점령 대결
![image](https://github.com/user-attachments/assets/a2fe931e-c06a-4909-b9dc-d707021ced2f)

5. **승리 화면:** 승리한 플레이어의 색상으로 승리 메시지 출력
![image](https://github.com/user-attachments/assets/b9209f61-555a-4097-aeca-3b3c262b3a0e)

6. **게임 종료 후:** 5초 뒤 초기 화면으로 자동 전환
![image](https://github.com/user-attachments/assets/661f413d-461c-4a50-8f89-6747c10e41d9)


## 📁 파일 구조
- 'main.c': 전체 게임 로직
- 'Opening.wav', 'space.wav', 'Sound.wav': 게임에 사용되는 사운드

## 🧠 배운 점
- 콘솔 그래픽을 활용한 좌표 기반 UI 구성
- 멀티플레이어 이동 및 점령 처리 로직
- 아이템 등장과 효과 적용을 통한 게임 밸런싱 경험

## ✅ 향후 개선점
- 싱글 플레이어 모드 추가
- 점령 영역 시각화 및 채색 방식 개선
