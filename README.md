# mpeg-1 影像解碼器 (mpeg-1 video decoder)

## 安裝依賴

本程式依賴於 [SFML](https://www.sfml-dev.org) 函式庫，請依照官網指示安裝，或是搜尋你所使用的發行版的套件庫。

在 ubuntu ，你可以執行

``` sh
sudo apt install libsfml-dev
```

在 arch 系的發行版，可用

``` sh
sudo pacman -S sfml
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

## 執行

照上節編譯完成後，會得到一執行檔 `mpeg-decoder`

```
./mpeg_decoder <mpeg file>
```

## 說明

### 遺漏

本專案沒有完全實踐 mpeg-1 的影片標準：

- 運動補償的半格精度沒做
- 有些該用 // 的地方直接用 / （取整方式不同）
- B 幀的 skipped macroblock 沒看懂，但因爲已經可播放，就不再研究
- 各種 extension-data ，以及自定義的量化矩陣，都沒有支援。目前會拋出異常，但有預留讀取的程式碼。
- 一些沒啥用的參數沒做處理，如 vbv_buffer, bit_rate 也固定在 24
- SFML 庫的 Window::setSize 方法，會切割到影片，因此寫死成 320 X 240。大概可以調參解決，但一時找不到。
- ......

### 設計

採生產者消費者模型，分成兩條執行緒（見 main.cpp），解碼的執行緒一直生產圖片進佇列，播放影片的執行緒則從佇列拿影片播放。

應該是挺好擴展的，要實作暫停、倒退等等功能，在佇列上動手腳即可。

#### 主要檔案介紹

- decoder.cpp: 解碼流程都寫在這，有點亂，沒有好好整理
- bit_reader.cpp: 從資料流讀取不定比特的資訊、解碼霍夫曼
- util.cpp: 功能性函數，包含離散餘弦變換
- image_queue.cpp: 生產者消費者模型用到的佇列
- play_video.cpp: 播放器，使用 SFML 庫

- check.js: 檢查標準的霍夫曼編碼表是否滿足[範式](https://github.com/MROS/jpeg_tutorial/blob/master/doc/%E8%B7%9F%E6%88%91%E5%AF%ABjpeg%E8%A7%A3%E7%A2%BC%E5%99%A8%EF%BC%88%E9%99%84%E9%8C%84%E4%BA%8C%EF%BC%89%E5%84%AA%E5%8C%96%E6%8A%80%E5%B7%A7.md#%E7%AF%84%E5%BC%8F%E9%9C%8D%E5%A4%AB%E6%9B%BC%E8%A1%A8%E5%84%AA%E5%8C%96)，結論爲真。

### 幫助

有興趣的可以去看源碼，一些地方 uncomment 掉可以打印錯誤訊息。

## 易錯提醒

- YCbCr 的解碼公式與 JPEG 中的不同，請看 http://softpixel.com/~cwright/programming/colorspace/yuv/
- run level 的霍夫曼解碼要特判多處，請詳細閱讀標準
- recon 時，I 跟 P, B 幀算法不同，注意那個 Sign
- P 幀也可以用 P 幀做補償
- P 幀跟 B 幀 motion vector 重置的時間點不完全一致

## 其他心得

整個解碼過程的耦合度相當高，原本做了好幾個 struct 想把各個變數區隔開，但反而導致存取變數要寫很長，影響可讀性。不如全部塞在一個 Class 就好。
