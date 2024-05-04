Chinese Version is below.

中文版在英文版之后。

# Introduction to TobenotLLMGameplay Plugin Features

TobenotLLMGameplay: Tobenot's LLM Game Plugin —— "Langchain" for Unreal Engine C++

This is a dedicated LLM game plugin for Unreal Engine C++. Perhaps with this plugin, one could quickly establish the relevant logic for large model games.

## Supported Unreal Engine Version

- Unreal Engine 5.3

## Installation Guide

1. Download the plugin repository.
2. Create a new folder named `Plugins` in your Unreal Engine project's root directory.
3. Extract the downloaded repository contents into the `Plugins` folder.
4. Compile the project using C++.
5. There are some settings available in the project settings, where some blueprint classes that override in the project can be set up initially with the base class.

## Overview of Features

### Character System
- **Shout Component**: Allows characters to interact naturally with nearby Agents, offering functionalities like providing dialogue options for players and gathering names and identities of surrounding Agents.
- **Dialogue Component**: Facilitates multi-person conversations, manages dialogue instances, and features long-term memory (automatic compression method).
- **Chat Component**: Offers one-on-one dialogues along with the preservation of historic messages.
- **Target Component**: Detects and selects dialogue targets.
- **FunctionInvoke Component**: A lightweight component for function invocation, facilitating operations without strictly following OpenAI's FunctionCall process.
- **Interactive Objects**: Designs basic information and logic for interactive items.
- **Chat UI**: Ideally, UI should be done in blueprints, but here are dialog boxes used for Dialogue and Chat.
- **Agent Interface**: Provides character identity names, positioning information, portrait display, and a simplified inventory.
- **Agent Component**: Allows characters to decide communication moments based on timers.
- **General Narrator (NarrativeAgent)**: Allows simplified settings and reading of preset JSON data, facilitating conversations and interactions.

### Scene System
- **Site Module**: Uses spherical models to represent game areas.

### Event System
- **Dynamic Event Generation**: Supports the dynamic creation of events including preset event import, loading, triggering, and the distribution and tracking process of various agents' desires within events.
- **Event Triggering**: Implements proximity-based triggering mechanisms and plot-driven narrative triggering mechanisms.
- **Interactive Item Generation**: Generates interactive items following event triggering.

#### Network Narrative System
- A system that records events to trigger new ones.

### Peripheral Systems
- **Identity Tables, GUID Archiving System**: Generates a unique GUID for each identity Actor, supporting conversation history save and restoration for chat components.
- **Live-Image Module**: AI-generated images.
- **Network Module**: Provides alternative API URLs and ports.
- **Language Module**: Configures game language options.

### API System
**GPT Plugin**: Enhances [OpenAI-Api-Unreal](https://github.com/tobenot/OpenAI-Api-Unreal)
  - Natively supports C++.
  - Now allows mid-request HTTP cancellation.
  - Adds word embedding interfaces.

## Design Concepts
### Network Narrative System Design

The core of implementing a meshed narrative system is creating rich storylines through recording and triggering events. This system's design primarily includes event recording and the mechanism for triggering new events based on those records.

#### Event Recording

To accurately record the dynamics within the game, the system is capable of:

- **Event Monitoring**:
  - Monitoring and recording various in-game events, such as dialogues, player interactions, and changes in game mechanics.
  - For game mechanics changes, an interface is provided for event reporting.

- **Event Analysis**:
  - Processes event results using a transformer large language model, breaking them down into several tags that encapsulate the event's meaning.
  - Adheres to the "actor-then-outcome" tag recording principle.

- **Tag Recording Principle**:
  - Single record principle: Each record expresses only one event entity.
  - Sequential coherence: Event outcomes are coherently linked through tag groups.

- **Data Embedding**:
  - Word embedding processing for each tag, facilitating further analysis and matching.

#### Event Triggering

New events are triggered based on recorded events and tags, including:

- **Trigger Detection Interface**:
  - Provides an external interface for the system to detect if event triggering conditions are met.

- **Tag Matching**:
  - Handles word embedding processing for tag groups against the prerequisites of the event to be triggered.
  - Searches for matching tag groups within event records.
  - Matching criteria: A tag group meets conditions only if all tags in a group appear in the correct order within a record and the similarity between tags exceeds 65%.

- **Logic Fulfillment**:
  - Determines whether tag groups fully satisfy conditions based on Boolean logic.
  - Prerequisite verification: An event's prerequisite conditions are considered fulfilled only if all corresponding Boolean logic conditions are met.
 

# TobenotLLMGameplay 插件功能介绍

TobenotLLMGameplay: Tobenot's LLM Game Plugin —— "Langchain" for Unreal Engine C++

虚幻引擎中的"Langchain"！这是一个专用于Unreal Engine C++的LLM游戏插件。也许用这个插件能快速地建立起大模型游戏的相关逻辑~

## 支持的虚幻引擎版本

- Unreal Engine 5.3

## 安装指南

1. 下载插件仓库。
2. 在你的Unreal Engine项目根目录下创建一个名为`Plugins`的新文件夹。
3. 将下载的仓库内容解压到`Plugins`文件夹中。
4. 通过C++进行项目编译。
5. 有一些设置可以在项目设置里看到，需要设置一些在项目中覆写的蓝图类，一开始可以直接写基类

## 功能概览

### 人物系统
- **Shout喊话组件**：允许角色在附近的Agent间自然地交互。在里面示例地做了一些小功能，比如支持为玩家提供可选的对话选项，收集周围Agent的姓名和身份。
  （用喊话组件说话会发给附近的Agent，以此来实现更自然的交互，比起它来说，一对一和多对一组件可以用来做那种通讯软件和小空间私聊，也可以用作文字交互小游戏，像做饭游戏、说服游戏）。
- **Dialogue组件**：实现多人会话，管理对话实例，拥有长期记忆（自动压缩方式）的功能。
- **Chat组件**：提供一对一对话及历史信息的保存。
- **目标组件**：检测和选择对话目标。
- **FunctionInvoke组件**：轻量级功能调用组件，实现函数调用操作，不严格走OpenAI的FunctionCall流程。
- **交互物**：设计了基本的互动物品信息和逻辑。
- **Chat UI**：说实话UI应该在蓝图里做，不过这里写了Dialogue和Chat用的对话框。
- **Agent接口**：提供角色身份名字、身份定位信息、肖像显示和简易物品栏。
- **Agent组件**：允许角色根据定时器自行决定交流时机。
- **通用叙述者NarrativeAgent**：可以非常简化的设置和读取预设JSON数据、进行对话和交互。
  
### 场景系统
- **位点模块**：使用球型模型表示游戏区域。

### 事件系统
- **事件动态生成**：支持动态创建事件。预设事件的导入、加载、触发，以及事件中各个Agent欲望的分发和追踪过程。
- **事件触发**：目前实现了基于接近的触发机制和基于剧情的网状叙事触发机制。
- **交互物生成**：在事件触发后可以生成可互动物品。
  
#### 网状叙事系统
- 记录已经发生的事件来触发新事件的系统

### 外围系统
- **身份表、GUID存档系统**：对每个身份的Actor生成唯一GUID，支持聊天组件的对话历史保存和恢复。
- **生图模块**：AI生成图片。
- **网络模块**：提供替换的API网址与端口。
- **语言模块**：配置游戏语言选项。

### API系统
**GPT插件**：改进了[OpenAI-Api-Unreal](https://github.com/tobenot/OpenAI-Api-Unreal)
  - 使其原生支持C++。
  - 现可中途取消HTTP请求。
  - 添加了词嵌接口

## 设计稿
### 网状叙事系统设计

实现网状叙事系统的核心是通过记录和触发事件来创建丰富的故事线。本系统设计主要包含事件的记录和基于此记录触发新事件的机制。

#### 事件记录

为了精确记录游戏中的动态，系统具备以下能力：

- **事件监听**：
  - 监听和记录游戏内发生的各种事件，如对话、玩家互动和游戏机制变化。
  - 对于游戏机制变化，提供接口以供事件上报。

- **事件分析**：
  - 利用transformer大语言模型处理事件结果，拆分成能够表达事件含义的多个标签组。
  - 遵循"先行动者后结果"的标签记录原则。

- **标签记录原则**：
  - 单一记录原则：每条记录仅表达一个事件实体。
  - 结果连贯性：通过标签组串接，使得事件结果意义上连贯。

- **数据嵌入**：
  - 对每个标签进行词嵌入处理，以便进一步的分析和匹配。

#### 事件触发

依据记录的事件和标签对新事件进行触发，触发机制包括：

- **触发检测接口**：
  - 对外提供接口，以供系统检测事件触发条件是否满足。

- **标签匹配**：
  - 针对待触发事件的前置条件，进行标签组的词嵌入处理。
  - 在事件记录中寻找符合条件的标签组。
  - 条件匹配准则：一个标签组满足条件，当且仅当该组中所有标签在一条记录中以正确顺序出现，标签和标签之间词嵌相似度大于65%。

- **逻辑满足**：
  - 根据布尔逻辑判断标签组是否全都满足条件。
  - 前置条件验证：只有当相应的布尔逻辑条件全部满足时，才认为事件触发的前置条件已经成立。

