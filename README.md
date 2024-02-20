# TobenotLLMGameplay: Tobenot's LLM Game Plugin — "Langchain" for Unreal Engine C++

This plugin is a compilation of general logic frequently employed by Tobenot for his LLM gaming projects. It meticulously shapes three primary structures: character, scene, and event—encapsulating the essential components of game development.

## Key Features

### Character System:
- **Chat Component**: Enables dialogue interactions and records chat history.
- **Target Component**: Detects and designates entities for conversation.
- **FunctionInvoke Component**: A streamlined function call mechanism that initiates calls without return values.
- **Interactive Objects**: Elementary data and mechanisms for object interaction.
- **Chat UI**: Facilitates a selection of interactions and dialogues; minimal UI handling recommended as UI elements are outside the realm of C++.

### Scene System:
- **Waypoint Module**: Denotes an area using a spherical representation.

### Event System:
- **Event Dynamic Creation**: Allows for on-the-fly event creation.
- **Event Activation**: Presently limited to proximity-based activation using waypoints.
- **Interactive Object Production**: Spawns interactive objects upon event initiation.

### Ancillary Systems:
- **Identity Register & GUID Archive**: Assigns unique GUIDs to each character actor; currently limited to recording and retrieving chat logs.
- **Image Production Module**: AI-assisted image creation.
- **Networking Module**: Supplies customizable API endpoints and ports.
- **Language Module**: Configurations for in-game language preferences.

### API System:
- **GPT Plugin**: An adaptation of [OpenAI-Api-Unreal](https://github.com/tobenot/OpenAI-Api-Unreal) tailored for native C++ support after considerable enhancements. Submission planned for a later date.

## Installation Procedure
1. Acquire the plugin repository download.
2. In your Unreal project's root directory, parallel to the `Content` folder, establish a new folder titled `Plugins`.
3. Decompress the downloaded repository and relocate its contents to the `Plugins` folder.
4. Proceed to compile and integrate into your C++ workflow.

## Compatibility
- Unreal Engine version 5.3


# 简体中文：
# TobenotLLMGameplay: Tobenot's LLM Game Plugin —— "Langchain" for Unreal Engine C++

这是一个由tobenot在其LLM游戏中使用的通用逻辑的集合插件。
本插件精心设计了三大系统：人物、场景、事件，涵盖了游戏开发中的核心要素。

## 目前的主要功能

### 人物系统：
- **Chat组件**：对话和保存历史的能力
- **目标组件**：检测和选择对话目标的能力
- **FunctionInvoke组件**：只调用不返回的轻量级FunctionCall
- **交互物**：基础的交互物信息与交互逻辑
- **Chat UI**：做了一些选择目标和聊天的，发现还是别管太多UI好，UI还是非C++吧

### 场景系统：
- **位点模块**：用球体表示一个区域
  
### 事件系统：
- **事件动态生成**：支持事件动态生成
- **事件触发**：目前只接位点做了接近触发
- **交互物生成**：触发事件后生成交互物

### 外围系统：
- **身份表、GUID存档系统**：对每个身份的Actor生成唯一GUID，目前只支持对话历史保存恢复
- **生图模块**：AI生成图片
- **网络模块**：提供替换的api网址与端口
- **语言模块**：游戏语言设置

### API系统：
- **GPT插件**：Fork了[OpenAI-Api-Unreal](https://github.com/tobenot/OpenAI-Api-Unreal)，做了大量修改，使其原生支持C++。日后有空提交一下。

## 安装指南
1. 下载插件仓库
2. 在您的虚幻项目根目录下创建一个名为`Plugins`的新文件夹（与`Content`文件夹同级）
3. 将下载的仓库内容解压并放入`Plugins`文件夹中
4. 通过C++进行编译使用。

## 开发的虚幻引擎版本
- 虚幻引擎 5.3
