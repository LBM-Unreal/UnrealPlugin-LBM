# Project Documents

[Milestone 1 Slides](https://docs.google.com/presentation/d/1kCbG1b0JJE_pyrgcBuqKFJYQE60t1BLb7Cjyb2nZFx8/edit?usp=sharing)

# Additional References

## LBM Introduction

https://www.youtube.com/watch?v=jfk4feD7rFQ

https://www.youtube.com/watch?v=8qorVDJS1QA

https://ppluscht.github.io/LBM/

https://pastewka.github.io/Accelerators/lecture/boundary-conditions.html

# Project Setup

1. Install your Unreal Engine, either epic launcher version or source code version. I will use epic launcher version. For the source code version, please follow the documentation on epic website. 

2. If you install epic launcher version, I highly recommend you to install the debug symbol files as well since it will provide more information for debugging.

![](./Misc/1.png)

3. When you finish install unreal engine, right click file LBM.uproject, then click `Generate Visual Studio project files`. Then use LBM.sln vs solution file to compile and debug the UE project.

4. Before you run the project, check the file `./Config/DefaultEngine.ini` in the project folder. You should change the setting `renderdoc.BinaryPath` to your install location of renderdoc. **Note: Don't use backslash`\` in path, please use slash`/`** If you compile your renderdoc from source, you should set the directory to the folder contains renderdoc.dll

5. Also, check your `Engine/UE5/Config/ConsoleVariables.ini` in vs solution explorer. You may want to use `r.ShaderDevelopmentMode=1` and `r.DumpShaderDebugInfo=1`.

6. For addtional info for debugging shaders, you may find addtional generated files in `Saved\ShaderDebugInfo\PCD3D_SM6\Global` in your project folder as you finish compile your shaders. `Global` folder contains global shaders and compute shaders, rest of the folders are material shaders. 

# Running the Project

1. The way I recommend to debug project is to run through `Local Windows Debugger` big button so that you can capture logs and add breakpoints. 

2. Once you start your UE editor, you can use hot reload button to recompile code changes by hitting the button. You can also set your hotkey in Editor Preference.

![](./Misc/3.png)

3. To test the compute shader code, open `TestBP` blueprint in content drawer, and switch to event graph. If you want to debug the shader using renderdoc, you can put your functions between `Start Renderdoc` and `End Renderdoc`. After executing `End Renderdoc`, `Captures.rdc` will appear at the project folder for you to inspect. Do not use the renderdoc blueprint functions in `Event Tick` since it will continously capture the resource at every frame. 

![](./Misc/4.png)

4. Go to viewport and drag TestBP from content drawer to scene. And hit start. After a few seconds for renderdoc finish its work, you will see a texture on the plane. This demonstrates how to use unreal engine to write a compute shader with texture input and output. 

![](./Misc/5.png)

5. For the global shader creation, please see my blog [Link](https://sirenri2001.github.io/2025-02-05-unreal-shader-tutorial-(Chinese)/). Also check Reference folder for additional papers for LBM implementation.

# Q&A 

1. Q:Why my debug key input does not work?

A: After dropping your blueprint actor to scene, select that then see the property window, select Misc tab, replace `Disable Input` to `Player 0`
