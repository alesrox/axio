# 🛠️ Code Generation Specification (Mapeo AST -> ISA)

Este documento define la estrategia de traducción que empleará el compilador para transformar las estructuras sintácticas del lenguaje (AST) en instrucciones de 32 bits para la VM basada en registros.

---

## 1. Gestión de Registros y Tabla de Símbolos

Dado que la VM utiliza un modelo de **Registros Virtuales** y **NaN-Boxing (64 bits)**:

* **Asignación Local:** El compilador mantiene una **Tabla de Símbolos** activa por cada ámbito (*scope*).
* **Indexación:** Cada variable declarada (`VarDecl`) recibe un índice de registro incremental dentro del marco actual (`R(0)`, `R(1)`, `R(2)`, ...).
* **Liberación / Reutilización:** Los registros temporales utilizados para evaluar expresiones intermedias se liberan inmediatamente después de su uso.

---

## 2. Declaración y Asignación de Variables

### Sintaxis
```text
int a = 5;
int b = 10;
int c = a + b;
```

### Mapeo de Registros
* a → R(0)
* b → R(1)
* c → R(2)

### Bytecode Generado
```text
LOADK    R(0), ConstIndex(5)   ; Carga la constante 5 en R(0)
LOADK    R(1), ConstIndex(10)  ; Carga la constante 10 en R(1)
ADD_INT  R(2), R(0), R(1)      ; R(2) = R(0) + R(1)
```

---

## 3. Estructuras de Control: Sentencia if / else

Para las sentencias condicionales se utiliza la técnica de Backpatching (relleno diferido del desplazamiento del salto).

### Sintaxis
```text
if a > b {
    c = 1;
} else {
    c = 2;
}
```

### Esquema de Bytecode y Saltos
```text
[0x00] GT_INT        R(temp), R(a), R(b)  ; R(temp) = (a > b)
[0x01] JUMP_IF_FALSE R(temp), +3          ; Si es falso, salta al bloque ELSE
; --- Bloque THEN ---
[0x02] LOADK         R(c), ConstIndex(1)
[0x03] JUMP          +2                   ; Salto incondicional al final del IF
; --- Bloque ELSE ---
[0x04] LOADK         R(c), ConstIndex(2)
```

---

## 4. Bucles

### 4.1. Bucle While

El bucle requiere la evaluación de la condición al inicio y un salto relativo negativo al final del bloque para repetir la ejecución.

#### Sintaxis
```text
while a < 10 {
    a = a + 1;
}
```

#### Esquema de Bytecode y Saltos
```text
; --- INICIO BUCLE (0x00) ---
[0x00] LT_INT        R(temp), R(a), ConstIndex(10)
[0x01] JUMP_IF_FALSE R(temp), +3    ; Si a >= 10, sale del bucle
; --- CUERPO ---
[0x02] ADD_INT       R(a), R(a), ConstIndex(1)
[0x03] JUMP          -3            ; Regresa a INICIO BUCLE (0x00)
```

### 4.2 Bucles for

El bucle `for item in iterable` se traduce internamente en una secuencia de evaluación de iterador, verificación de fin de secuencia (`JUMP_IF_FALSE`) y salto de retorno negativo.

#### Sintaxis
```text
for i in range(0, 10) {
    print(i);
}
```

#### Esquema de Bytecode y Saltos
```text
; --- PREPARACIÓN DEL ITERADOR ---
[0x00] CALL_RANGE   R(iter), ConstIndex(0), ConstIndex(10) ; Crea iterador range(0, 10)

; --- INICIO BUCLE (0x01) ---
[0x01] ITER_NEXT    R(i), R(iter)        ; Obtiene el siguiente elemento; si no hay más, R(temp_bool) = False
[0x02] JUMP_IF_FALSE R(temp_bool), +3             ; Si la secuencia terminó salta fuera (+3)

; --- CUERPO ---
[0x03] CALL_PRINT   R(i)                 ; print(i)
[0x04] JUMP          -4                  ; Regresa a INICIO BUCLE (0x01)
```

---

## 5. Llamadas a Funciones (Sliding Register Frame)

Las llamadas a función se benefician del paso de argumentos sin copia (zero-copy) mediante la reorientación del marco de registros.

### Sintaxis
```text
func add(int x, int y) -> int {
    return x + y;
}

int res = add(a, b);
```

### Esquema de Bytecode
```text
; Preparar argumentos en registros contiguos a partir del registro base del nuevo frame
[0x10] MOVE R(2), R(a)        ; Copia arg 0 -> R(2)
[0x11] MOVE R(3), R(b)        ; Copia arg 1 -> R(3)

; CALL R(A), B, C
; R(A) = Dirección/Registro de la función
; B    = Registro base de la ventana de la función (R(2))
; C    = Cantidad de argumentos (2)
[0x12] CALL R(fn_add), R(2), 2

; El resultado retornado por RET se deposita en el primer registro del marco llamado (R(2))
[0x13] MOVE R(res), R(2)
```
