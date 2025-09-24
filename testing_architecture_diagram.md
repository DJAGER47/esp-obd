# Диаграмма архитектуры тестирования OBD2

## Общая архитектура тестовой системы

```mermaid
graph TB
    subgraph "Тестовая архитектура OBD2"
        subgraph "Критический приоритет"
            A[tests_obd_core_methods.cpp<br/>25-30 тестов]
            B[tests_obd_services.cpp<br/>15-20 тестов]
        end
        
        subgraph "Высокий приоритет"
            C[tests_obd_pid_group_1_20.cpp<br/>35-40 тестов]
            D[tests_obd_pid_group_21_40.cpp<br/>25-30 тестов]
            E[tests_obd_pid_group_41_60.cpp<br/>30-35 тестов]
            F[tests_obd_pid_group_61_80.cpp<br/>10-15 тестов]
            G[tests_obd_error_handling.cpp<br/>20-25 тестов]
        end
        
        subgraph "Средний приоритет"
            H[tests_obd_performance.cpp<br/>15-20 тестов]
        end
        
        subgraph "Существующие тесты"
            I[tests_obd_basic.cpp<br/>9 тестов]
            J[tests_obd_integration.cpp<br/>9 тестов]
            K[tests_obd_pids.cpp<br/>17 тестов]
        end
    end
    
    subgraph "Инфраструктура тестирования"
        L[MockIsoTp<br/>Мок ISO-TP интерфейса]
        M[Unity Framework<br/>Тестовый фреймворк]
        N[OBD2 Helper Functions<br/>Вспомогательные функции]
    end
    
    A --> L
    B --> L
    C --> L
    D --> L
    E --> L
    F --> L
    G --> L
    H --> L
    I --> L
    J --> L
    K --> L
    
    A --> M
    B --> M
    C --> M
    D --> M
    E --> M
    F --> M
    G --> M
    H --> M
    I --> M
    J --> M
    K --> M
    
    A --> N
    B --> N
    C --> N
    D --> N
    E --> N
    F --> N
    G --> N
    H --> N
```

## Структура покрытия по компонентам

```mermaid
graph LR
    subgraph "OBD2 Класс"
        subgraph "Основные методы"
            A1[findResponse]
            A2[conditionResponse]
            A3[selectCalculator]
            A4[queryPID]
            A5[get_response]
            A6[processPID]
        end
        
        subgraph "PID Методы"
            B1[Группа 1-20<br/>20 методов]
            B2[Группа 21-40<br/>15 методов]
            B3[Группа 41-60<br/>20 методов]
            B4[Группа 61-80<br/>4 метода]
        end
        
        subgraph "Сервисы"
            C1[SERVICE_01]
            C2[SERVICE_02]
            C3[SERVICE_03]
        end
        
        subgraph "Вспомогательные"
            D1[ctoi]
            D2[nextIndex]
            D3[removeChar]
            D4[timeout]
        end
    end
    
    subgraph "Покрытие тестами"
        E1[🔴 Критическое<br/>Не покрыто]
        E2[🟡 Частично<br/>Покрыто]
        E3[🟢 Полностью<br/>Покрыто]
    end
    
    A1 --> E1
    A2 --> E1
    A3 --> E1
    A4 --> E2
    A5 --> E2
    A6 --> E2
    
    B1 --> E1
    B2 --> E1
    B3 --> E1
    B4 --> E1
    
    C1 --> E2
    C2 --> E1
    C3 --> E1
    
    D1 --> E1
    D2 --> E1
    D3 --> E1
    D4 --> E1
```

## Поток выполнения тестов

```mermaid
sequenceDiagram
    participant T as Test Function
    participant M as MockIsoTp
    participant O as OBD2 Class
    participant U as Unity Framework
    
    T->>M: reset()
    T->>M: add_receive_message()
    T->>M: set_receive_result(true)
    
    T->>O: queryPID(service, pid)
    O->>M: send(message)
    M-->>O: return success
    
    T->>O: get_response()
    O->>M: receive(message)
    M-->>O: return mock_data
    
    T->>O: [specific_pid_method]()
    O->>O: findResponse()
    O->>O: selectCalculator()
    O->>O: conditionResponse()
    O-->>T: return calculated_value
    
    T->>U: TEST_ASSERT_*()
    U-->>T: validation_result
    
    T->>M: cleanup resources
```

## Матрица зависимостей тестов

```mermaid
graph TD
    subgraph "Базовые тесты"
        A[Constructor Tests]
        B[Mock Infrastructure Tests]
    end
    
    subgraph "Основные методы"
        C[queryPID Tests]
        D[get_response Tests]
        E[findResponse Tests]
        F[conditionResponse Tests]
        G[selectCalculator Tests]
    end
    
    subgraph "PID тесты"
        H[PID Group 1-20]
        I[PID Group 21-40]
        J[PID Group 41-60]
        K[PID Group 61-80]
    end
    
    subgraph "Интеграционные"
        L[Service Tests]
        M[Error Handling]
        N[Performance Tests]
    end
    
    A --> C
    B --> C
    C --> D
    D --> E
    E --> F
    F --> G
    
    G --> H
    G --> I
    G --> J
    G --> K
    
    H --> L
    I --> L
    J --> L
    K --> L
    
    L --> M
    M --> N