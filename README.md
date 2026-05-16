```markdown
# [2023066980, 김영진] HW4 - 무선 네트워크

## 개요
본 프로젝트는 매체 접근 제어(MAC) 프로토콜, 특히 **ALOHA** 계열(Pure ALOHA, Slotted ALOHA) 및 **CSMA** 계열(Non-persistent, 1-persistent, p-persistent)의 성능을 분석하고 비교하기 위해 C언어로 구현된 이산 시간(discrete-time) 시뮬레이터입니다.

시뮬레이션은 광범위하게 제공되는 트래픽 부하($G$)에 대한 시스템 처리량($S$)을 평가합니다. 시뮬레이션 결과는 데이터 파일로 내보내지며, 이후 Python 스크립트를 통해 실험 데이터와 이론적 성능 곡선을 함께 그래프로 시각화하여 비교 분석합니다.

---

## 주요 특징
* **통합 시뮬레이터:** 단일 핵심 C 프로그램(`channel_sim.c`)이 ALOHA 및 CSMA 프로토콜 제품군을 모두 처리합니다.
* **이산 시간 해상도:** 틱(tic) 기반의 타이밍 모델을 통해 슬롯 경계를 정밀하게 시뮬레이션하며, Pure ALOHA의 연속 시간 취약 기간(vulnerable period)을 근사합니다.
* **포괄적인 CSMA 변체 지원:** 매개변수 설정을 통해 Non-persistent, 1-persistent, p-persistent (0.1 및 0.5) CSMA 모드를 모두 지원합니다.
* **이론적 검증:** Python 시각화 도구가 시뮬레이션 결과 위에 분석적 이론 방정식을 자동으로 겹쳐서 그려줍니다.
* **자동화된 워크플로우:** `Makefile`을 통해 컴파일, 실행, 데이터 라우팅, 그래프 작성을 단일 명령어로 원활하게 제어합니다.

---

## 폴더 트리

```text
.
├── cashe
│   └── channel_sim                  <--- 컴파일된 바이너리 실행 파일
├── results
│   ├── channel_sim_results.dat      <--- 그래프 생성에 사용되는 시뮬레이션 결과 데이터
│   └── protocol_throughput_plot.png <--- 최종 비교 처리량 그래프
├── Makefile
├── README.md
└── src
    ├── channel_sim.c                <--- 메인 C 시뮬레이터 소스 코드
    └── plot.py                      <--- Python 시각화 및 이론 매핑 스크립트
4 directories, 5 files

```

## 시스템 파라미터 (매크로 정의)

`channel_sim.c` 내부에서 매크로 및 상수를 통해 설정된 주요 환경 변수는 다음과 같습니다.

| 파라미터 | 기본값 | 설명 |
| --- | --- | --- |
| `NUM_STATIONS` | 300 | 공유 채널을 두고 경쟁하는 총 네트워크 스테이션(노드) 수. |
| `SLOT_SIZE_MS` | 10 | 밀리초(ms) 단위의 단일 시간 슬롯 지속 시간. |
| `SIMULATION_SLOTS` | 10000 | 통계적 안정성을 달성하기 위해 시뮬레이션되는 총 슬롯 수. |
| `NUM_G_VALUES` | 17 | 평가된 총 트래픽 부하($G$) 데이터 포인트 수. |
| `G_VALUES` | 0.2 ~ 9.0 | 실행 중 테스트된 특정 트래픽 부하($G$) 배열. |

## 코드 구조 및 함수

### 1. 주요 구성 요소

* `src/channel_sim.c`: 패킷 도착 생성, 채널 상태 확인, 충돌 해결, 지속성(persistence) 로직을 처리하고 원시 결과를 기록합니다.
* `src/plot.py`: `.dat` 파일을 pandas DataFrame으로 읽어 들이고, 이론적 한계를 계산하며, 시뮬레이션 마커를 매핑하고, matplotlib을 사용하여 최종 시각적 분석 결과를 출력합니다.

### 2. 세부 함수 명세

| 함수명 | 분류 | 핵심 역할 |
| --- | --- | --- |
| `malloc_channel` | ALOHA | 채널 틱(tic)의 타임라인을 나타내는 동적 배열을 할당하고 `FREE`로 초기화합니다. |
| `free_channel` | ALOHA | 채널 틱 배열을 위해 동적으로 할당된 메모리를 안전하게 해제합니다. |
| `rand_gen` | 공통 | 확률 $p$에 기반하여 패킷 도착 여부를 결정하기 위한 의사 난수(부동 소수점)를 생성합니다. |
| `sim_1tic` | ALOHA | 모든 스테이션의 도착을 샘플링하고 채널 상태가 `FREE`, `FULL`, 또는 `FAIL`이 되는지 평가하여 단일 틱을 시뮬레이션합니다. |
| `success_rate` | ALOHA | Slotted 및 Pure ALOHA 시뮬레이션을 위한 메인 루프를 구동하며, 활성 전송 윈도우를 추적하여 처리량을 계산합니다. |
| `simulate_csma` | CSMA | 반송파 감지(Carrier Sensing) 메커니즘을 구현하고, 타임라인에서 Non-persistent, 1-persistent, p-persistent 상태 루프를 처리합니다. |
| `save_log_file` | 공통 | 최종 수치 데이터를 구조화하여 `results/channel_sim_results.dat`로 내보냅니다. |

## 프로토콜 동작 및 알고리즘

### 📡 ALOHA 프로토콜

ALOHA 시뮬레이션은 구조적 제약을 모방하기 위해 `success_rate()` 내부에서 시간의 입도(`tic_per_slot`)를 변경합니다.

* **Slotted ALOHA (tic_per_slot = 1)**
* 시간이 슬롯 경계에 엄격하게 정렬됩니다. 스테이션은 정확한 틱 마크에서만 전송을 시작할 수 있습니다.
* 한 슬롯에서 정확히 1개의 스테이션만 전송하면 `FULL`(성공)로 기록됩니다. 2개 이상의 스테이션이 전송하면 즉시 충돌(`FAIL`)이 발생합니다.


* **Pure ALOHA (tic_per_slot = 100)**
* 스테이션은 패킷이 도착하는 즉시 완전히 비동기적으로 전송을 시작합니다.
* 취약 기간($2 \times \text{패킷 지속 시간}$)을 캡처하기 위해 패킷 지속 시간은 100 틱에 걸쳐 있습니다. 활성 전송이 진행되는 동안 다른 전송이 시작되면 현재 전송과 겹치는 패킷이 모두 취소되어 전송 중 충돌을 시뮬레이션합니다.



### 🔍 CSMA (Carrier Sense Multiple Access) 프로토콜

`simulate_csma()` 함수는 반송파 감지 단말을 시뮬레이션합니다. 각 틱은 과거 이력 윈도우(`tx_starts`)를 확인하여 매체가 현재 사용 중(busy)인지 추론합니다.

* **Non-Persistent CSMA**
* 채널이 유휴 상태(`!busy`)로 감지되면 스테이션은 즉시 전송합니다.
* 채널이 사용 중으로 감지되면 스테이션은 즉시 시도를 포기하고 백오프(back off)합니다. 채널이 해제될 때 즉시 전송하기 위해 대기열에 추가되지 않습니다.


* **1-Persistent CSMA**
* 채널이 유휴 상태이면 스테이션은 즉시 전송합니다.
* 채널이 사용 중이면 스테이션은 매체를 지속적으로 수신 대기하며 `deferred_queue`에 트래픽을 대기시킵니다.
* 채널이 유휴 상태로 전환되는 순간, 대기 중인 모든 스테이션이 1.0의 확률로 전송을 시도하므로, 부하가 높은 상황에서는 사용 중 기간 직후에 본질적으로 높은 충돌 급증이 발생합니다.


* **p-Persistent CSMA (0.1 및 0.5)**
* 채널이 사용 중이면 스테이션은 이를 추적하고 `deferred_queue`에 누적됩니다.
* 채널이 유휴 상태가 되면, 누적된 도착 및 새로운 도착은 구성된 분수 확률 $p_{\text{persist}}$ (예: 0.1 또는 0.5)로 전송을 시도합니다.
* 확률적 체크를 통과하지 못한 노드들은 다음 틱으로 시도를 미루어, 사용 중 상태 이후의 충돌 급증을 완화합니다.



## 빌드 및 실행

프로젝트 컴파일 및 수명 주기는 `Makefile`을 통해 관리됩니다.

Linux: make
Windows
```text
mkdir cashe, results
gcc src/channel_sim.c -o cashe/channel_sim.exe -lm -DNUM_STATIONS=300 -DSLOT_SIZE_MS=10 -DSIMULATION_SLOTS=10000 -DNUM_G_VALUES=17
.\cashe\channel_sim.exe
python src/plot.py
move channel_sim_results.dat results/
  move protocol_throughput_plot.png results/
```

```



```
  이 명령은 C 소스 코드를 `cashe/channel_sim`으로 컴파일하고, 바이너리를 실행하여 원시 데이터를 출력하고, `plot.py`를 실행하여 결과를 처리한 다음, 모든 것을 `results/` 폴더에 정리합니다.

* **빌드 환경 정리:**
  ```bash
  make clean
  

```

컴파일된 바이너리 아티팩트, 임시 데이터 로그 및 생성된 그림을 제거합니다.

## 결과 및 분석

### 처리량 비교 그래프

시뮬레이션 데이터에서 생성된 그래프는 구현된 프로토콜의 성능을 이론적 한계와 비교합니다.

