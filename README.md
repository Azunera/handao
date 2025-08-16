
# Handao / 汉道

![Example pf the project](/assets/images/example.png)
Hanzi Faxian is a project for an app capable of in-live detection and definition of chinese characters through pop ups (both simplified and traditional).

### Future Planned Features
- New detection workflow based on whole screen screenshots, bound boxs and hovering
- Customization GUI = Hotkey change, perfomance selection, old vs new detection model
- Improve processing perfomance
- Perhaps a Linux and MacOS version

## For Users 🙋
### Installation

For install use this link to get the .zip for the project: https://github.com/Azunera/handao/releases/download/v0.1/handao_v0.1.zip.
Extract it on any desired folder, then run the .exe to activate it, it will open a terminal then where you will get the information of current program state. 
Currently only a Windows version is available, if anyone needs a Linux or MacOS version im willing to make one.

### Usage
A few seconds after running the program when it finishes loading, you can activate it by doing left control + alt + g while having your mouse over a chinese text. I particularly designed to be used over desktop apps and videogames, since there are more efficient and fast alternatives in major browsers, however it can still be usable.

- **ctrl + alt + f or left click**: Take a screenshot at mouse current position (F for FIND)
- **ctrl + alt + g**: Deactivate the program, reducing a bit resource usage and enabling ctrl + alt + f for usage in other programs
- **ctrl + alt + y**: Exit the program. If this happens to fail for any reason, open task manager and finish the task, then report the bug here: ""

#### Why so annoying shortcuts?

I've been trying to find a comfortable shortcuts, but most are a used by a lot of apps, so chose these which for the several apps I tested, all of them lacked functionality. Still I have to add a way to custom it.

### Availability
Currently only a Windows version is available.

### Security
The app may be flagged of an antivirus, and this is due to I need to use some methods such as `GetAsyncKeyState` to check if the shortcuts been pressed and if any other action besides hovering was done to disable a current window. Due to stuff like this, I highly recommend you to strictly download this app from this repository and never from 3rd sources unless they have been approved by me here, to avoid tempered versions which may seem legitimate.

## For Developers 🧑‍💻

### 🔎 How it works ->
#### Initialization:
During initialization the program parses the dictionary from the same folder as the .exe. As well as initializes the rest of functions. Is important that other files also have to be present in the .exe folder. Such as a build\bin\Debug\tessdata\chi_sim.traineddata tessdata folder.

#### Input Reading Thread
A thread is initializaed that reds for the keys left control, left menu and g. G may be tapped while the other keys can stay pressed to activate, however it cannot be spammed as it is prevented by a flag. The variable that it modifies is an atomic flag given to it named "process"

#### Main processing
After the flag of process is given truth, the position of mouse is retrieved, and a screenshot utilizing throguh bitmaps (to be continued...). Then the bitmap is converted to a scaled gray-scale PIX from leptonica library (needed for Tesseract to analyse it) Then tesseract does a simple analysis giving the text. Then the information of mouse pos and text extracted is given to the SFML.

#### Pop Up Windows
PopUpWindow is the class that contains the SFML windows and what functions can be run with it, it has to called from main and loop the events there since it does not contain a standalone run function.
(to be continued....)
### 📜Requirements ->
- Built with cmake 4.0.3, minimum is at least 3.5
- Visual Studio 17 2022 as Constructor
- MSVC 19.44.0, other previous version may be compatible. Other compilers may fail as Tesseract uses MSVC
- Tesseract installed via vcpkg,
- Leptonica installed via vcpkg
- VisualStudio tools are also required for Tesseract
- Cedict dictionary from: https://www.mdbg.net/chinese/dictionary?page=cedict

### ⚙️ Compiling ->

#### Building:
```
mkdir build
cd ./build -
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/devtools/vcpkg/scripts/buildsystems/vcpkg.cmake
```
#### Compiling:
```
cd ./build
cmake --build .

```

## Other ->
### Want to help?
If interested in contribute make an issue or contact me on Discord: https://discord.gg/4bxgEBQKDF

#### ⚖️ Coding norms ->
- Classes are in PascalCase
- Methods, functions and attributes in snake\_case
- Perfomance ideals: 
   - 300< mb of RAM usage during runtime.
   - 2< seconds of app response (For instance, the time it takes the user to get the information of a character they did a programmed action action to get)
- Inclusion of other languages is not welcome for now until further notice
- *Suggestions for optimization are very welcome! As well as Linux or MacOS versions suggestions with code optimized for those platforms*

### 
### 👋 Contact me
For now the only way to contact me is through the app's discord server: https://discord.gg/4bxgEBQKDF.
I will enable mails and other ways later on.

### 🌟Credits
Independent project, special thanks to Tsubaki, Make, Fran and Family.











