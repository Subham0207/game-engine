# How should object which need saving work

1. We have filesmap which is of type { guid_string: filelocation_string }[].
2. We have EngineState which can store ref array of different objectTypes. Like vector<PlayerController> and vector<Statemachine>.