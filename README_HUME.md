4) Compile-time addition (convenient for testing, still local-only):

    Add the following to `GResident.pro` (local edit only) — qmake will set a C++ macro:

    ```make
    DEFINES += HUME_API_KEY=\"<your_api_key_here>\"
    ```

    - This causes the application to pick up the API key at build-time and is useful
       for quick system tests. Because it's local to your machine project file edits,
       ensure you do not push a `.pro.user` file or commit these defines to the repo.
    - The code will pick up `HUME_API_KEY` automatically in `VoiceToText` constructor
       if defined.

5) From the running app — Set (and save) key via the UI

   - A new menu appears on the main window labelled `Hume`. Use `Hume -> Set Hume API Key...` to enter your key.
   - The key is saved to `.hume_api_key` in the app directory (or `~/.hume_api_key` fallback) and will be used by the runtime.
   - This is a convenient and secure way to set keys without editing project files or environment variables.
How to securely configure your Hume API key for local development

Do NOT commit your API key to git. Use one of the following secure options:

1) Environment variable (recommended):

   PowerShell (Windows):
   ```powershell
   setx HUME_API_KEY "<your_api_key_here>"
   # Restart your terminal / IDE after setx
   ```

   Bash (Unix/macOS):
   ```bash
   export HUME_API_KEY="<your_api_key_here>"
   ```

2) Local untracked file (convenience, also local-only):
   - Copy `hume_api_key.example` to `.hume_api_key` or `hume_api_key` in the application folder
   - Put your key on the first line; the code reads `~/.hume_api_key` and `appdir/.hume_api_key`.

   Alternative: local header (compile-time) for dev/testing
   -------------------------------------------------------

   If you prefer to add the key directly to compiled code while keeping it out of
   version control, create a local header named `local_hume_key.h` based on the
   `local_hume_key.h.example` file. Add `local_hume_key.h` to your `.gitignore` (the
   project already ignores that file), then put your key in the header:

   ```
   #define HUME_API_KEY "sk-live-xxxxxxxxxxxxxxxxxxxxxxxxxx"
   ```

   Once that header is present, you can either:
   - include it at the top of `voicetotext.cpp` (add `#include "local_hume_key.h"`),
      or
   - add `DEFINES += HUME_API_KEY=\"sk-live-xxxxxxxxxxxxxxxxxxxxxxxxxx\"` into
      your `GResident.pro` file (not recommended for committed repositories).

   Reminder: do NOT commit `local_hume_key.h` or a `.pro` change that contains a
   secret to your repo — keep it local. Using `.hume_api_key` or environment
   variables is safer and works without code changes.

   Example `.hume_api_key`:
   ```text
   nCkDcVxCcO78RGwrW2UvZxHJaA0PAxAh7t0tv0kW6TikIudO
   ```

3) Direct embedding (not recommended):
   - If you absolutely must, add the key at runtime instead of committing it. For example:
   ```cpp
   m_voiceToText->setApiKey("<your_api_key_here>");
   ```
   - DO NOT commit this change. Use it only for quick testing then revert.

Why you shouldn’t commit the key
--------------------------------
- Committing keys leaks access—if the repository is pushed to public or shared remote, the key will be exposed.
- Use environment variables or local files that are ignored by git to minimize accidental leaks.

Troubleshooting
---------------
- If the app doesn't detect your key, check `qDebug()` output or call `resolveHumeApiKey()` / `resolveHumeApiKeyFromFile()` from code to inspect where it was loaded from.
