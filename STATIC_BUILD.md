# Статическая сборка Incline3D GUI

Данный документ содержит инструкции по сборке приложения Incline3D GUI со статической линковкой Qt. Статическая сборка позволяет создать исполняемый файл, не требующий наличия установленных библиотек Qt на целевой машине пользователя.

## Содержание

1. [Обзор](#обзор)
2. [Требования](#требования)
3. [Сборка статического Qt](#сборка-статического-qt)
   - [Linux](#linux)
   - [Windows](#windows)
4. [Сборка приложения](#сборка-приложения)
5. [Особенности распространения](#особенности-распространения)
6. [Устранение неполадок](#устранение-неполадок)

---

## Обзор

Статическая сборка Qt означает, что все библиотеки Qt компилируются в один исполняемый файл, вместо динамической линковки с отдельными `.so`/`.dll` файлами.

**Преимущества:**
- Не требуется установка Qt на машине пользователя
- Упрощённое распространение (один исполняемый файл)
- Гарантированная совместимость версий

**Недостатки:**
- Больший размер исполняемого файла (100-200 МБ)
- Необходимость пересборки при обновлении Qt
- Лицензионные ограничения (требуется коммерческая лицензия Qt или GPL)
- Более сложный процесс сборки

---

## Требования

### Общие требования

- CMake 3.20 или новее
- C++20-совместимый компилятор:
  - GCC 11+ (Linux)
  - MSVC 2022 (Windows)
  - Clang 14+
- Около 20 ГБ свободного места для сборки Qt
- Около 2-4 часов на сборку Qt

### Linux

Необходимые системные пакеты (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    perl \
    python3 \
    git \
    pkg-config \
    libfontconfig1-dev \
    libfreetype-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxrender-dev \
    libxcb1-dev \
    libxcb-cursor-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-util-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libegl1-mesa-dev \
    libdrm-dev \
    libgbm-dev \
    libssl-dev \
    libicu-dev \
    libdbus-1-dev \
    libatspi2.0-dev \
    libcups2-dev
```

### Windows

- Visual Studio 2022 с компонентами C++
- Windows SDK 10.0.19041.0 или новее
- Perl (ActivePerl или Strawberry Perl)
- Python 3.x

---

## Сборка статического Qt

### Linux

#### Шаг 1: Скачивание исходного кода Qt

```bash
# Создание рабочего каталога
mkdir -p ~/qt-static-build
cd ~/qt-static-build

# Скачивание Qt 6.6.x (или актуальной версии)
git clone https://code.qt.io/qt/qt5.git qt6
cd qt6
git checkout v6.6.3
perl init-repository --module-subset=qtbase,qtsvg,qtimageformats,qttranslations
```

Альтернативно, можно скачать архив с исходниками с https://download.qt.io/official_releases/qt/

#### Шаг 2: Конфигурация

```bash
cd ~/qt-static-build
mkdir qt6-build && cd qt6-build

../qt6/configure \
    -prefix /opt/qt6-static \
    -release \
    -static \
    -opensource \
    -confirm-license \
    -nomake examples \
    -nomake tests \
    -skip qt3d \
    -skip qtactiveqt \
    -skip qtcharts \
    -skip qtconnectivity \
    -skip qtdatavis3d \
    -skip qtdoc \
    -skip qtgamepad \
    -skip qtlocation \
    -skip qtlottie \
    -skip qtmultimedia \
    -skip qtnetworkauth \
    -skip qtpurchasing \
    -skip qtquick3d \
    -skip qtquicktimeline \
    -skip qtremoteobjects \
    -skip qtscxml \
    -skip qtsensors \
    -skip qtserialbus \
    -skip qtserialport \
    -skip qtspeech \
    -skip qtvirtualkeyboard \
    -skip qtwayland \
    -skip qtwebchannel \
    -skip qtwebengine \
    -skip qtwebglplugin \
    -skip qtwebsockets \
    -skip qtwebview \
    -bundled-xcb-xinput \
    -xcb \
    -fontconfig \
    -system-freetype \
    -opengl desktop \
    -openssl-linked \
    -feature-concurrent \
    -feature-printsupport \
    -feature-opengl \
    -feature-widgets \
    -- -DCMAKE_BUILD_TYPE=Release
```

**Важные флаги:**
- `-static` — статическая сборка
- `-prefix /opt/qt6-static` — каталог установки
- `-opensource -confirm-license` — открытая лицензия
- `-nomake examples -nomake tests` — не собирать примеры и тесты
- `-skip` — пропуск ненужных модулей для уменьшения времени сборки

#### Шаг 3: Сборка и установка

```bash
# Сборка (займёт 2-4 часа)
cmake --build . --parallel $(nproc)

# Установка (требуются права root для /opt)
sudo cmake --install .
```

После установки статический Qt будет находиться в `/opt/qt6-static`.

### Windows

#### Шаг 1: Подготовка среды

Откройте «x64 Native Tools Command Prompt for VS 2022».

```batch
REM Создание рабочего каталога
mkdir C:\qt-static-build
cd C:\qt-static-build

REM Скачивание Qt (через git или скачайте архив вручную)
git clone https://code.qt.io/qt/qt5.git qt6
cd qt6
git checkout v6.6.3
perl init-repository --module-subset=qtbase,qtsvg,qtimageformats,qttranslations
```

#### Шаг 2: Конфигурация

```batch
cd C:\qt-static-build
mkdir qt6-build
cd qt6-build

..\qt6\configure ^
    -prefix C:/Qt/6.6.3-static ^
    -release ^
    -static ^
    -static-runtime ^
    -opensource ^
    -confirm-license ^
    -nomake examples ^
    -nomake tests ^
    -skip qt3d ^
    -skip qtactiveqt ^
    -skip qtcharts ^
    -skip qtconnectivity ^
    -skip qtdatavis3d ^
    -skip qtdoc ^
    -skip qtmultimedia ^
    -skip qtnetworkauth ^
    -skip qtquick3d ^
    -skip qtremoteobjects ^
    -skip qtsensors ^
    -skip qtserialbus ^
    -skip qtserialport ^
    -skip qtspeech ^
    -skip qtvirtualkeyboard ^
    -skip qtwebchannel ^
    -skip qtwebengine ^
    -skip qtwebsockets ^
    -skip qtwebview ^
    -opengl desktop ^
    -feature-concurrent ^
    -feature-printsupport ^
    -feature-opengl ^
    -feature-widgets ^
    -- -DCMAKE_BUILD_TYPE=Release
```

**Важно:** Флаг `-static-runtime` линкует статически CRT (C Runtime), что необходимо для полностью автономного исполняемого файла.

#### Шаг 3: Сборка и установка

```batch
REM Сборка
cmake --build . --parallel

REM Установка
cmake --install .
```

---

## Сборка приложения

### Указание пути к статическому Qt

При конфигурации CMake укажите путь к статическому Qt через переменную `CMAKE_PREFIX_PATH`:

#### Linux

```bash
cd /path/to/incline3d-gui

cmake -S . -B build-static \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/qt6-static \
    -DINCLINE3D_GUI_BUILD_TESTS=OFF

cmake --build build-static --parallel $(nproc)
```

#### Windows

```batch
cd C:\path\to\incline3d-gui

cmake -S . -B build-static ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.6.3-static ^
    -DINCLINE3D_GUI_BUILD_TESTS=OFF

cmake --build build-static --config Release
```

### Проверка типа сборки

CMake автоматически определяет тип Qt (статический или динамический) и выводит сообщение при конфигурации:

```
-- Обнаружена статическая сборка Qt
-- Настройка статической линковки Qt...
-- Статическая линковка Qt настроена
```

### Результат сборки

После успешной сборки исполняемый файл `incline3d_gui` (Linux) или `incline3d_gui.exe` (Windows) будет содержать все необходимые компоненты Qt и не потребует дополнительных DLL/SO файлов.

Проверка зависимостей:

```bash
# Linux
ldd build-static/incline3d_gui
# Должен показать только системные библиотеки (libc, libm, libGL, libX11 и т.д.)

# Windows (PowerShell)
dumpbin /dependents build-static\Release\incline3d_gui.exe
# Должен показать только системные DLL (kernel32.dll, user32.dll, opengl32.dll и т.д.)
```

---

## Особенности распространения

### Зависимости, остающиеся динамическими

Даже при статической сборке Qt некоторые системные библиотеки остаются динамическими:

**Linux:**
- libc, libm (glibc)
- libGL, libGLX (OpenGL)
- libX11, libxcb (X Window System)
- libfontconfig, libfreetype (шрифты)

**Windows:**
- kernel32.dll, user32.dll, gdi32.dll (системные)
- opengl32.dll (OpenGL)

Эти библиотеки присутствуют на всех целевых системах.

### Включение дополнительных файлов

При распространении приложения необходимо включить:

1. **CLI-утилита `inclproc`** — для расчётов инклинометрии
2. **Конфигурационные файлы** (при необходимости)

### Минимальный дистрибутив

```
incline3d/
├── incline3d_gui           # Основное приложение (Linux)
├── incline3d_gui.exe       # Основное приложение (Windows)
├── inclproc                # CLI-утилита (Linux)
├── inclproc.exe            # CLI-утилита (Windows)
└── README.txt              # Инструкции
```

### Размер исполняемого файла

Ориентировочный размер статически слинкованного исполняемого файла:
- Linux: 80-120 МБ
- Windows: 100-150 МБ

Для уменьшения размера можно использовать `strip` (Linux) или флаг `/DEBUG:NONE` (Windows):

```bash
# Linux
strip build-static/incline3d_gui

# Или при сборке
cmake -S . -B build-static \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    ...
```

---

## Устранение неполадок

### Ошибка: «cannot find -lQt6...»

Убедитесь, что путь к статическому Qt указан правильно через `CMAKE_PREFIX_PATH`.

### Ошибка: «undefined reference to `QXcbIntegrationPlugin`»

Плагины Qt не линкуются. Проверьте, что:
1. Qt собран со всеми необходимыми плагинами
2. В `main.cpp` присутствуют директивы `Q_IMPORT_PLUGIN`
3. CMake нашёл и подключил плагины (см. вывод конфигурации)

### Ошибка: отсутствуют системные библиотеки (Linux)

Установите недостающие пакеты разработки:

```bash
sudo apt install libxcb-*-dev libxkbcommon-x11-dev libfontconfig1-dev
```

### Приложение не запускается: «could not find or load the Qt platform plugin»

Для статической сборки эта ошибка не должна возникать, если плагины правильно импортированы. Проверьте:

1. Макрос `INCLINE3D_STATIC_QT` определён при компиляции
2. `Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)` (Linux) или `Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)` (Windows) присутствует в `main.cpp`

### Проблемы со шрифтами (Linux)

При статической сборке fontconfig должен найти системные шрифты. Если шрифты не отображаются:

```bash
# Проверка fontconfig
fc-list | head

# Обновление кэша шрифтов
fc-cache -fv
```

### Проблемы с OpenGL

Убедитесь, что на системе установлены драйверы OpenGL:

```bash
# Linux
glxinfo | grep "OpenGL version"

# Windows: OpenGL драйвер поставляется с драйвером видеокарты
```

---

## Альтернативы статической сборке

Если статическая сборка Qt слишком сложна, рассмотрите альтернативы:

### 1. windeployqt / linuxdeployqt

Инструменты для автоматического копирования необходимых DLL/SO рядом с приложением:

```bash
# Windows
windeployqt --release build/incline3d_gui.exe

# Linux (через linuxdeploy)
./linuxdeploy-x86_64.AppImage --appdir AppDir --executable build/incline3d_gui --output appimage
```

### 2. AppImage (Linux)

Упаковка приложения со всеми зависимостями в один файл `.AppImage`.

### 3. Docker-контейнеры

Распространение приложения в виде Docker-образа с предустановленным Qt.

---

## Лицензионные соображения

При статической сборке Qt обратите внимание на лицензионные требования:

- **Qt Open Source (LGPL/GPL):** При статической линковке требуется выпуск приложения под GPL или предоставление исходного кода и возможности перелинковки
- **Qt Commercial:** Статическая линковка разрешена без ограничений

Подробнее: https://www.qt.io/licensing/

---

## Контакты

При возникновении вопросов по сборке обращайтесь:

- Email: yan@bubenok.com
- Telegram: @iBubenok
- GitHub: @iBubenok
