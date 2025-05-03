# プロジェクト概要

E-maは、寺院の本堂を舞台に展開される、インタラクティブなデジタル絵馬体験を提供するシステムです。

来場者が描いた「願い」や「イラスト」が、その場で幻想的な和風アートへと変換され、プロジェクションマッピングで空間いっぱいに投影されます。花々にふれあうセンサー演出や、和風音楽の自動生成を通して、願いが花になり土にかえり、また新たに咲くという「くりかえしの世界」を体験できる、新感覚のアートプロジェクトです。
本プロジェクトでは、Unity / Ruby on Rails / Cloud Run / capacitive sensor などを用いたシステム開発と、企業・大学との連携による空間演出を行っています。

# リポジトリ概要

このリポジトリには、展示会場に設置された金属製の花オブジェクトの制御プログラムが含まれています。

使用しているマイコンは ESP32-WROOM-32E です。

花に触れたことを静電容量式タッチセンサーで検知し、その情報をシリアル通信を通じてPC側のUnityアプリケーションへ送信します。Unityはそのタッチ情報をもとに、各花に対応するLEDの光量を計算し、再びシリアル通信を使ってESP32側へ返します。

# システム構成
- 各花６つには1つのESP32（子機）が接続されており、タッチセンサとLEDを制御します。（花一つにつき一つのタッチセンサ，ＬＥＤを接続）
- 子機のESP32は直列接続されており、各子機が自分のタッチ情報と前段の情報をまとめて次の子機へ転送します。
- 最終的に、親機ESP32がすべてのタッチ情報を受け取り、PC（Unity）に送信します。
- Unityが計算したLEDデータは、親機から各子機に向けて直接先のタッチ情報のポートとは別ポートのシリアル通信で分配されます（親機がハブの役割を担います）。

# ファイル構成

parent：親機用プログラム。タッチ情報の収集およびLEDデータの送信を行います。

child：子機用プログラム。タッチセンサの読み取りとデータの受け渡しを行います。

child_temp：子機プログラムのテンプレートです。複数の子機で共通のコードを使用するためのベースとなります。

# 環境

- Arduino IDEを用いて各ESP32にプログラムを書き込んでください。
- Unity側のシリアル通信処理は以下のリポジトリのSerialCommunicationを参照してください：https://github.com/momoyama-tech/e-ma-unity

# 詳細な技術仕様・設計

本プロジェクトのアーキテクチャや使用技術の詳細については、以下のNotionページにまとめています：

[📘 E-ma プロジェクト概要 - Notion](https://pinnate-smash-23b.notion.site/E-ma-19efe465481f804e9a28cbca9379be98?pvs=4)

# Project Overview

E-ma is an interactive digital ema (votive tablet) experience set in the main hall of a temple.
Visitors’ hand-drawn “wishes” and “illustrations” are transformed in real-time into atmospheric Japanese-style digital art and projected across the space using projection mapping.

Through interactions with capacitive touch sensors embedded in flower objects, as well as the automatic generation of Japanese-style background music, participants experience a symbolic cycle of life: wishes bloom as flowers, return to the soil, and bloom again—an artistic representation of rebirth and continuity.

The project integrates technologies including Unity, Ruby on Rails, Cloud Run, and capacitive sensing, in collaboration with university laboratories and corporate partners to deliver both system development and spatial design.

# Repository Overview

This repository contains the control programs for the metallic flower objects installed in the exhibition space.
Each flower is equipped with an ESP32-WROOM-32E microcontroller.

When a visitor touches a flower, the touch is detected via capacitive touch sensors. This input is sent via serial communication to a Unity application running on a PC.
Unity calculates the corresponding LED brightness for each flower and sends this data back to the ESP32 devices via serial communication.

# System Architecture

- Each set of 6 flowers is connected to one ESP32 "child" unit, which handles both touch detection and LED control. (One touch sensor and one LED per flower.)
- Child ESP32s are connected in serial daisy-chain fashion. Each child collects its own touch data and forwards it along with upstream data to the next child.
- A parent ESP32 receives the complete touch dataset from all children and sends it to the Unity application on the PC.
- Unity calculates LED brightness for all flowers and sends the data from the parent to all child units via a separate serial port (the parent acts as a communication hub).

# File Structure

parent: Program for the parent unit. It aggregates touch data and distributes LED brightness values.

child: Program for child units. Each reads capacitive input and forwards data along the chain.

child_temp: A template for child programs. Used to configure multiple identical children easily.

# Environment
- Use Arduino IDE to flash the programs to each ESP32.
- Unity serial communication scripts are available here:https://github.com/momoyama-tech/e-ma-unity

# Technical Specifications & Documentation

Detailed architecture, technical components, and overall project structure are documented here:

[📘 E-ma Project Overview (Notion)](https://pinnate-smash-23b.notion.site/E-ma-19efe465481f804e9a28cbca9379be98?pvs=4)
