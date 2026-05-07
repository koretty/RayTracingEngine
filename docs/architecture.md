# システムアーキテクチャ 

このドキュメントでは、本プロジェクトの全体構造、使用技術、およびディレクトリ構成について記述します。

## 1. システム全体図
システムの全体像と、主要ファイルの役割を以下に示します。

### 1.1 main.cppの役割
`main.cpp`は、事前定義しておいた`config`を読み込み、`renderer`に橋渡しし描画を行う役割を担っています。なお、`renderer`で計算されたピクセル情報を`SDL3(外部ライブラリ)`に渡すことで、最終的な描画出力を行っています。以下の図は、`main.cpp`がどのように他のコンポーネントと連携しているかを示しています。

```mermaid
graph TD
    %% 外部のヘッダーファイルやモジュール
    Scene["scene_config.hpp"]
    Camera["camera_config.hpp"]
    Renderer["renderer.hpp"]
    SDL3["SDL3(外部ライブラリ)"]

    subgraph MainContainer [" "]
        direction TB

        Title{"main.cpp"}
        style Title fill:none,stroke:none,font-size:18px,font-weight:bold
        
        Step1["1. 情報の読み込みと初期化"]
        Step2["2. 描画処理"]
        Step3["3. 描画出力"]
    
        Title ~~~ Step1
        Step1 --> Step2 --> Step3
    end

    Scene -->|シーン情報| Step1
    Camera -->|カメラ情報| Step1
    Renderer -->|描画関数| Step2
    SDL3-->Step3 

```

### 1.2 renderer.cppの役割
`renderer.cpp`は、シーン情報とカメラ情報に基づいてレンダリング処理を実行し、ピクセル情報を生成する役割を担っています。以下の図は、`renderer.cpp`がどのように他のコンポーネントと連携しているかを示しています。

```mermaid
graph TD
    Scene["scene.hpp"]
    Camera["camera.hpp"]

    subgraph MainContainer [" "]
        direction TB

        Title{"renderer.cpp"}
        style Title fill:none,stroke:none,font-size:18px,font-weight:bold
        
        Step1["1. 全ピクセルに対してループを回す"]
        Step2["2. 1つのピクセルに対して、複数のレイを生成する"]
        Step3["3. 1つのレイに対して一番近い交差点を求める"]
        Step4{"4. レイが物体と交差するか？"}
        Step5{"5. 交差した物体のマテリアル情報が正しいものか？"}
        Step6["6. ピクセルの色を決定する"]
        Background["背景色を返す"]
        Normal["法線ベクトルを返す"]

        Title ~~~ Step1
        Step1 --> Step2 --> Step3 --> Step4
        
        Step4 -- Yes --> Step5
        Step4 -- No --> Background

        Step5 -- Yes --> Step6 
        Step5 -- No --> Normal
    end

    Scene -->|シーン情報| Step1
    Camera -->|カメラ情報| Step1
    

```


### 1.3 scene.cppの役割
`scene.cpp`は、シーンの情報を管理し、レンダリング処理に必要なデータを提供する役割を担っています。以下の図は、`scene.cpp`がどのように他のコンポーネントと連携しているかを示しています。

```mermaid
graph TD
    
    subgraph MainContainer [" "]
        direction TB

        Title{"scene.cpp / find_closest_hit"}
        style Title fill:none,stroke:none,font-size:18px,font-weight:bold

        Step1["1. bvhツリーを探索する"]
        Step2{"2. bvhノードがあるか？"}
        Step3["3. hit関数で内部のオブジェクトと交差するか判定"]
        Step1 --> Step2 
        Step2 -- Yes --> Step3
        Step2 -- No --> NoHit["falseを返す"]
    end



    

```


## 2. 技術スタック 


## 3. ディレクトリ構成
プロジェクトの主要なディレクトリ構造とその役割です。
```text

src/
├── main/
│   ├── main.cpp                # エントリーポイント
│   └── config/
│       ├── scene_config.hpp    # シーン設定
│       └── camera_config.hpp   # カメラ設定
├── renderer/
│   ├── renderer.cpp            # レンダリング処理
│   └── renderer.hpp            # レンダラーのインターフェース情報
├── scene/
│   ├── scene.cpp               # シーンの実装
│   ├── scene.hpp               # シーンのインターフェース定義
```

