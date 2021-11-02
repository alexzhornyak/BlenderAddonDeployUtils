# Utils for deploying Blender addons
Useful utils for deploying blender addons

## BlenderAddonAutoIncrement.exe
Utility for auto-incrementing Blender addon version
### Command line arguments:
- **-path** - path to `__init__.py` in the root of addon folder
- **-index** - version (0,0,0) index to increment: 0 or 1 or 2 etc.  Default: last index
- **-encoding** - file encoding (UTF-8, SYSTEM). Default: UTF-8
### Examples:
**`__init__.py`**
```py
bl_info = {
    "name": "My Script",    
    "version": (1, 0, 0)
}
```
#### 1. Default (Increment last version index)
* Command:
```posh
BlenderAddonAutoIncrement.exe -path Path\To\__init__.py
```
* Result:
```py
bl_info = {
    "name": "My Script",    
    "version": (1, 0, 1)
}
```
#### 2. Increment major version number
* Command:
```posh
BlenderAddonAutoIncrement.exe -path Path\To\__init__.py -index 0
```
* Result:
```py
bl_info = {
    "name": "My Script",    
    "version": (2, 0, 0)
}
```

## BlenderAddonZip.exe
Utility for deploying addon source as a valid Blender addon zip package
### Program Actions:
* Copies addon src folder to temporary directory, ignoring python cache files etc.
> WARNING!!! Copy algorithm is based on [robocopy](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/robocopy) mirror folder command. Be sure that you've filled target directory correctly!
* Packs temporary addon folder as Blender addon zip package
* Gives version number name prefix
### Command line arguments:
- **-addonDir** - path to addon directory
- **-addonName** - addon name. Default: name of 'addonDir'
- **-tempDir** - temporary directory for creating zip content. Default: User Temp Dir
- **-targetZipDir** - path to output zip dir
- **-targetZipName** - addon zip literal name or variable:  AUTO_GENERATE (Default)
- **-copyParams** - params for using in [robocopy](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/robocopy). Default:  `/MIR /XD .git .svn __pycache* /XF *.log *.md /R:3 /W:3`
- **-onBeforeZipCMD** - path to batch script that will be executed before packing temp folder into zip file
### Examples:
#### 1. Default (use addon version in naming and addon folder name matches addon name)
* Command:
```posh
BlenderAddonZip.exe -addonDir C:\blender\blender-addons\magic_uv -targetZipDir c:\addons\build
```
* Result:
> c:\addons\build\magic_uv_6_5_0.zip
#### 2. Zip from another folder
* Command:
```posh
BlenderAddonZip.exe -addonDir C:\GitHub\MyAddon\Src -addonName MyAddon -targetZipDir C:\GitHub\MyAddon\Build
```
* Result:
> C:\GitHub\MyAddon\Build\MyAddon_1_0_0.zip
#### 3. Exclude custom files and folders
> WARNING! Check target folder because it is based on [robocopy](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/robocopy) mirror function
* Command:
```posh
BlenderAddonZip.exe -addonDir C:\GitHub\MyAddon\Src -copyParams "/MIR /XD .git .svn __pycache* ico /XF *.log *.md *.dll /R:3 /W:3" -targetZipDir C:\GitHub\MyAddon\Build
```
* Result:
Folders: `.git`, `.svn`, `__pycache*`, `ico` and files `*.log`, `*.md`, `*.dll` are excluded
> C:\GitHub\MyAddon\Build\MyAddon_1_0_0.zip
#### 4. Change temp folder contents before packing into zip file
Suppose that you want to change file content in temporary folder before it will be packed into zip folder. In this case you may create batch script and pass its path as parameter **-onBeforeZipCMD**.
* Command:
```posh
BlenderAddonZip.exe -addonDir C:\blender\blender-addons\magic_uv -targetZipDir c:\addons\build -onBeforeZipCMD c:\Scripts\beforeZipCMD.script 
```
* Example: c:\Scripts\beforeZipCMD.script

> First parameter in batch script is the path to temporary folder!
```posh
echo 'Before Zip %1'
set _dir=%~1
powershell -Command "(gc -Path '%_dir%/vlog.py') -replace 'ENABLE_DEBUG = True', 'ENABLE_DEBUG = False' | sc -Path '%_dir%/vlog.py'"
```
This command will replace text `ENABLE_DEBUG = True` by `ENABLE_DEBUG = False` in the file  `Path\To\AddonTempDir\vlog.py`

## Configure in VSCode as POST-BUILD task
Expected: after hit build command `Shift+Ctrl+B`:
- increment addon version number
- pack as zip in build folder
### Actions:
- put **BlenderAddonAutoIncrement.exe**, **BlenderAddonZip.exe** to any system path location or use full path to them
- create **tasks.json** in **.vscode** folder

![image](https://user-images.githubusercontent.com/18611095/124961272-ebd5c800-e025-11eb-9733-fa9dc8771952.png)

```typescript
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Increment Build",
            "command": "BlenderAddonAutoIncrement.exe",
            "args": [
                "-path",
                "${workspaceFolder}/Src/__init__.py"
            ],
            "type": "shell"
        },
        {
            "label": "Deploy Build",
            "command": "BlenderAddonZip.exe",
            "args": [
                "-addonDir",
                "${workspaceFolder}/Src",
                "-addonName",
                "MyAddon",
                "-targetZipDir",
                "${workspaceFolder}/Build"
            ],
            "type": "shell"
        },
        {
            "label": "Build",
            "dependsOrder": "sequence",
            "dependsOn": [
                "Increment Build",
                "Deploy Build"
            ],
            "problemMatcher": [],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```
