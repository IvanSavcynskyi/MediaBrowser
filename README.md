# MediaBrowser (C++)

MediaBrowser — це веб-додаток для перегляду медіафайлів (відео, зображення) з локальної папки.  
Проєкт реалізований на **C++** з використанням **Drogon Framework** та **CMake**.

## Вимоги

Перед запуском переконайтесь, що встановлено:

- C++17 або новіший
- CMake
- vcpkg
- Drogon
- SQLite3

## Встановлення залежностей

Встановіть залежності через vcpkg:

```bash
vcpkg install
```
У файлі config.json вкажіть шлях до папки з медіа:
```
{
  "media": {
    "root": "E:/path/to/media"
  }
}
```
## Збірка проєкту

```
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="path_to_vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

## Тести

Мінімальні unit/smoke тести запускаються через CTest:

```
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="path_to_vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## CI/CD

GitHub Actions workflow знаходиться у `.github/workflows/ci.yml`.
Він запускається на `push`, `pull_request` і вручну через `workflow_dispatch`.

Pipeline виконує:

- встановлення C++/Node середовища на `windows-latest`;
- встановлення залежностей C++ через `vcpkg`;
- конфігурацію і Release-збірку backend;
- запуск CTest;
- `npm ci` і `npm run build` для frontend;
- завантаження артефакту `MediaBrowser-windows-release`.

## Запуск

Спочатку запустіть бекенд:

```
./build/Release/MediaBrowser
```

Під час старту бекенд сканує `media.root`, створює SQLite базу
`media_browser.sqlite3` у цій папці та синхронізує таблицю `media_files`
з актуальними відео, аудіо й зображеннями.

Потім у окремому терміналі запустіть фронтенд:

```bash
cd frontend
npm install
npm run dev
```

Після запуску відкрийте у браузері:
```
http://localhost:5173
```
