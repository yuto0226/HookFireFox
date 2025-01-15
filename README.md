# HookFireFox

可以自我啟動、注入 firefox，並 hook `nss3.dll` 的 `PR_Write`。

## 開發環境設定

安裝 vcpkg，用來安裝 minhook。

```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg; .\bootstrap-vcpkg.bat
```

與 Visual Studio MSBuild 整合

```shell
.\vcpkg.exe integrate install
```

> 可以把 vcpkg 設定到環境變數

安裝 minhook

```
./vcpkg.exe install minhook:x64-windows-static
```
