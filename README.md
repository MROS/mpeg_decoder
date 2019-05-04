# mpeg-1 影像解碼器 (mpeg-1 video decoder)

## 安裝依賴

本程式依賴於 [SFML](https://www.sfml-dev.org) 函式庫，請依照官網指示安裝，或是搜尋你所使用的發行版的套件庫。

在 ubuntu ，你可以執行

``` sh
sudo apt install libsfml-dev
```

進行安裝

## 編譯

先進入本專案所在目錄

``` sh
git clone https://github.com/MROS/mpeg_decoder
cd mpeg_decoder
```

使用 cmake 編譯
``` sh
mkdir build
cd build
cmake ..
make
```

## 執行（尚不可用）

照上節編譯完成後，會得到一執行檔 `mpeg-decoder`

```
./mpeg_decoder <mpeg file>
```