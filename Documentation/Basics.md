# How should object which need saving work

1. We have filesmap which is of type { guid_string: filelocation_string }[].
2. We have EngineState which can store ref array of different objectTypes. Like vector<PlayerController> and vector<Statemachine>.
3. when displaying list of statemachines say in characterUI, we use the filesmap and get the filenames that end with `.statemachine`.
4. Possessing a character: uses the playerController and camera attached to the possesses character.

# How do you handle blendspace creation via editor UI.
1. use the `Engineregistry.filesmap` to display the project files in the content browser.
2. if a blendspace file is clicked. Then open that blendspace in the blendspaceUI.
3. Add `create a blendspace` button. Which should open the blendspaceUI and let you create a blendspace and save it.
4. The animation names can be retrived from `Engineregistry.filesmap`. So we can drag and drop blendpoints.
5. Blendpoints contains info of the animation that was intended for drag and drop.

# Files
You create statemachine and then each character will a separate instance of the statemachine.
You create blendspace and then it also needs to be instancetiated to be used.

# Tests
1. Create a new project. Load the project hit play and you should be able to move the character. jump, and roll.
2. Load another character. Hit play. The other character's playercontroller is not active and doesnot have a statemachine hooked. So it should just fall to ground in T-pose.