# Introduction #

Currently there is no visual configuration of the game. However, it is highly modable via number of configuration files. Some of the settings may be changed during runtime via console (````` for entering the console, `?[ENTER]` for displaying commands available for current scene).

All configuration files obey the following conventions:
  * empty lines and lines beginning with '#' character are ignored
  * lines with unrecognised content are ignored
  * main sections start with `[section name]` line
  * if the section name in unrecognised, all following lines till the next section are ignored. This property is often used in the map files - for skipping models `[skip model]` section is used
  * content of the line that follows list of expected parameters is ignored, but for clarity I precede such comments with '#' character
> ex. ` Lighting 2 # 0 = none, 1 = basic, 2 = full `
  * text parameters are single word by default - you must use quotation marks to enter more words. To insert quotation mark in the text, enter two quotation marks
> ex. ` name "Long name with ""quotation marks"" inside" `
  * if the property is absent, default value is used

# The game #
The `/Data/config.txt` file configures main options that alter game behaviour. This configuration is divided into two sections - `[graphics]` and `[general]`.

## Graphics section ##
| **Property `[values]`** | **Description**                  |
|:------------------------|:---------------------------------|
| `lighting [0/1/2]`    | lighting rendering mode        |
|                       | 0 - no lights                  |
|                       | 1 - one, infinite light        |
|                       | 2 - all lights defined for map |
| `shadows [0/1/2]`     | shadows rendering mode         |
|                       | 0 - no shadows                 |
|                       | 1 - rigid bodies cast shadows  |
|                       | 1 - all objects cast shadows   |
| `shaders [0/1]`       | should the GPU shaders be used |
| `multisampling [0/1/2/...]` | antyaliasing level (will be multiplied x2). Some cards have problems with rendering text when multisampling is on |
| `useVBO [0/1]`        | should VBO extension be used (if exists) - I've met laptop on which it produced errors |
| `shadowMap [int]`     | size of the shadow map texture, currently all the shadows are volumetric, so this value is not used |
| `vSync [0/1]`         | should GPU wait for vertical-sync? |
| `windowX [int]`       | initial game window width          |
| `windowY [int]`       | initial game window height         |
| `fullScreen [0/1]`    | should the game start on full screen (switch with `[F11]`) |
| `fullScreenX [int]`   | full screen horizontal resolution  |
| `fullScreenY [int]`   | full screen vertical resolution    |
|                       |                                    |
| `show_ShadowVolumes [0/1]` | should the outline of shadow volumes be displayed? |
| `show_Skeleton [0/1]`      | should the skeletons be displayed?                 |
| `show_BVH [0/1]`           | should the lowest BVH levels be displayed?         |
| `show_Cameras [0/1]`       | should the symbolic cameras outline be displayed?  |

## General section ##
| **Property `[values]`**    | **Description**                           |
|:---------------------------|:------------------------------------------|
| `console [0/1]`          | disable/enable console                  |
| `scene [menu/game/test]` | initial scene                           |
|                          | menu - begin with menu                  |
|                          | game - begin with game, load map from `/Data/models/level_*.map`, `*` = `level` property                     |
|                          | test - begin with collision test scene (change BVH with `[0]`-`[9]` keys)                                                      |
| `level [int]`            | initial level of game and test scenes   |
| `speed [float]`          | game speed multiplier                   |
| `logging [0/1/2/3]`      | logging level (to `/Data/log.txt` file) |
|                          | 0 - log almost nothing                  |
|                          | 1 - log important errors                |
|                          | 2 - log all errors                      |
|                          | 3 - log all errors and warnings         |
| `3dsTo3dx [0/1]`         | should 3ds models be automatically saved to 3dx files - enabling may result in overwriting existing 3dx files                  |

# Keyboard #
The `/Data/keyboard.txt` file defines mapping of keys into their logical functions. First column contains key name, defined in the `/Data/keys.txt` file (only single word names are allowed). Second column contains numerical function code. Mapping of function codes to function names may be found in the `/Source Files/InputCodes.h` file.

# Cameras #
In the `/Data/cameras.txt` file you may configure views during game time. By default window is divided into four viewports - from players side, from top and from their eyes. Int the `/Data/cameras_*.txt` files you may find alternative camera sets.

Configuration file may contain any number of `[camera human]` or `[camera free]` sections. Each section defines one camera of given type (check [Cameras](Cameras.md) topic for differences between those types).

| **Property `[values]`** | **Description**                                         |
|:------------------------|:--------------------------------------------------------|
| `name [text]`         | name of the camera                                    |
| `eye [x y z]`         | viewers position                                      |
| `center [x y z]`      | target of observation                                 |
| `up [x y z]`          | vector pointing to the top of the screen              |
| `speed [float]`       | camera tracking speed                                 |
| `(viewport)`          | starts subsection that define destination window      |
| `left [0-100]`        | percentage of window width where the viewport starts  |
| `width [0-100]`       | percentage of window width that the viewport takes    |
| `top [0-100]`         | percentage of window height where the viewport starts |
| `height [0-100]`      | percentage of window height that the viewport takes   |
| `(fov)`               | begins subsection that defines projection, sets projection type to orthogonal |
| `angle [degrees]`     | camera's angle of view, sets projection type to perspective |
| `front [float]`       | distance to the near clipping plane                   |
| `back [float]`        | distance to the far clipping plane                    |
| `(track eye)`, `(track center)` | starts subsection that defines automatic tracking of targets by given camera property (eye or center) |
| `mode [text]`         | sets tracking mode                                    |
|                       | nothing - turns the tracking off                      |
|                       | object - tracks given object                          |
|                       | subobject - tracks given subobject                    |
|                       | all\_center - tracks center of all available objects   |
|                       | script name - tracks targets with given script, currently only built in scripts are available: |
|                       | EyeSeeAll\_Center – script for camera eye point – tries to see all available objects (from their side). The view center is mean of objects centers |
|                       | EyeSeeAll\_CenterTop – modification of EyeSeeAll\_Center script that looks from top |
|                       | EyeSeeAll\_Radius – script for camera eye point – tries to see all available objects (from their side). The view center is center of the volume that bounds all of the objects |
| `object [int]`        | index of tracked object                               |
| `subobject [int]`     | index of tracked subobject (for given tracked object) |
| `shift [x y z]`       | shift of the tracking destination point in the coordinate system depending on the script |

# Map list #
In the `/Data/maps.txt` file there is a list of maps shown on the "Select map" screen. It contains number of `[map]` sections. Each section describes one map with following properties:

| **Property `[values]`** | **Description**                      |
|:------------------------|:-------------------------------------|
| `name [text]`         | name of the map                    |
| `file [path]`         | path to the file with map          |
| `img [path]`          | path to the screenshoot of the map |

# Players list #
Players that may be selected in menu are listed in the `/Data/players.txt` file. It's divided into several `[player]` sections. Each section should define following properties:

| **Property `[values]`** | **Description**                         |
|:------------------------|:----------------------------------------|
| `name [text]`         | player name                           |
| `model [path]`        | path to the graphical model           |
| `fastm [path]`        | path to the simplified physical model |
| `customBVH [0/1]`     | should model use custom or automatic Bounding Volumes Hierarchy - automatic behaves better for players |
| `mass [int]`          | mass of the model                     |
| `style [text path]`   | name and path to the file that defines fighting style that may be used by this model (multiple style definitions creates list of available styles) |

Beside abovementioned options, it is possible to use properties of models defined in the map files. However, most of them makes no sense in current context (ex. disabling physics) or will be overwritten by options selected in the menu.

# Fighting styles #
Exemplary fighting style may be found in the `/Data/models/anim/karate.txt` file. Three types of sections should be included in such file – `[auto]`, `[mirror]` and `[action]`.

## Auto movement ##
This section defines list of actions which may be automatically taken by the fighter in order to move to its destination point:

| **Property `[values]`**        | **Description**  |
|:-------------------------------|:-----------------|
| `stop [action]`              | resting action |
| `step [action distance 0/1]` | given action will move player by given distance, 0/1 tells if the action may be stopped when destination is reached before finishing |
| `back [action distance 0/1]` | given action will move player backward by given distance, 0/1 flag tells if the action may be stopped |
| `left [action angle 0/1]`    | given action will rotate player left by given angle (degrees), 0/1 flag tells if the action may be stopped |
| `right [action angle 0/1]`   | given action will rotate player right by given angle (degrees), 0/1 flag tells if the action may be stopped |

## Animation mirroring ##
This section lists bones that form mirroring pairs, ex. left hand with right hand. This data allows using same animation for both sides of the body.
| **Property `[values]`** | **Description**                                  |
|:------------------------|:-------------------------------------------------|
| `bones [int int]`     | adds given pair of bones to the mirroring list |

## Actions and combos ##
Each occurrence of this section defines one action that may be taken by the fighter. It also defines keys that switch between animations (combos). Each time spans are given in miliseconds.
| **Property `[values]`**             | **Description**                      |
|:------------------------------------|:-------------------------------------|
| `name [text]`                     | action name                        |
| `anim [path start_time end_time]` | animation connected with this action. Defining few animations will mix them together |
| `time [int]`                      | duration of the action, 0 for loop |
| `post [action]`                   | action to be taken after finishing current (first action by default) |
| `rotate [0/1]`                    | tells if this action will rotate the fighter (0 by default) |
| `mirror [0/1]`                    | tells if this action will mirror the fighter - following animations will be mirrored (0 by default) |
| `(combo)`                         | starts subsection that defines possible pass to other action |
| `action [action]`                 | destination action                 |
| `key [name]`                      | key that activates combo, possible values are: Forward, Backward, **,**Punch, **Kick,**HandGuard, **LegGuard. Where** = Left / Right |
| `first [int]`                     | moment of opening the time window during which combo may be activated (0 by default) |
| `last [int]`                      | moment of closing the time window during which combo may be activated (0 by default = infinity) |
| `time [int]`                      | moment at which target action is started, if `last` is non zero, then `current-first` value is added to this time (0 by default) |
| `prec [0/1]`                      | if 1, then no `current-first` value will be added to `time` (0 by default) |
| `shift [0/1]`                     | if 1, then player's position matrix will be shifted by the shift of the current root bone position |

# Map #
Exemplary map may be found in the `/Data/models/level3.map` file. It's definition is divided into following sections: `[general]`, `[light]`, `[model]`, `[person]`.

## Main section ##
`[general]` section defines global map settings. It also allows for importing contents of other maps.
| **Property `[values]`** | **Description**                               |
|:------------------------|:----------------------------------------------|
| `import [path]`       | path to the external map that should be merged with this map |
| `skybox [path]`       | path to the 3D model that will simulate sky |
| `skycolor [r g b]`    | color of the sky, each color takes values between 0.0 and 1.0 |
| `spawn1rot [x y z]`   | initial rotation of the first player |
| `spawn1pos [x y z]`   | initial position of the first player |
| `spawn2rot [x y z]`   | initial rotation of the second player |
| `spawn2pos [x y z]`   | initial position of the second player |

## Lighting ##
Each `[light]` section defines one light.
| **Property `[values]`** | **Description**                                            |
|:------------------------|:-----------------------------------------------------------|
| `type [name]`         | type of the light                                        |
|                       | infinite - infinite light                                |
|                       | point – point light                                      |
| `state [1/0]`         | is it on or off                                          |
| `position [x y z]`    | point light position                                     |
| `direction [x y z]`   | infinite light direction                                 |
| `color [r g b]`       | light color, each color takes values between 0.0 and 1.0 |
| `softness [0.0-1.0]`  | softness of the shadows cast by this light               |
| `spot_dir [x y z]`    | direction of the spot light                              |
| `spot_att [int]`      | attenuation of the spot                                  |
| `att_const [int]`     | constant attenuation of the light                        |
| `att_linear [int]`    | linear attenuation of the light                          |
| `att_square [int]`    | square attenuation of the light                          |

## Objects ##
Each `[model]` section define one model from the scene. The only required property is the path to the file with a model.

| **Property `[values]`** | **Description**                         |
|:------------------------|:----------------------------------------|
| `name [text]`         | model name                            |
| `model [path]`        | path to the graphical model           |
| `fastm [path]`        | path to the simplified physical model |
| `customBVH [0/1]`     | should model use custom or automatic Bounding Volumes Hierarchy - manual behaves better for objects |
| `position [x y z]`    | position of the model, multiple `position` entries will be merged together |
| `rotation [x y z]`    | rotation of the model, multiple `rotation` entries will be merged together (order of `position` and `rotation` entries is important) |
| `velocity [x y z]`    | initial velocity of the model, setting `position` or `rotation` after `velocity` will stop the movement |
| `physical [0/1]`      | is the model affected by the gravity? |
| `locked [0/1]`        | is the model stationary (can't move)? |
| `mass [int]`          | mass of the model                     |
| `restitution [float]` | restitution coefficient - how much of the energy will be passed to other objects |
| `restitution_self [float]` | restitution coefficient - how much of the energy will be retained |
| `shadows [0/1]`       | are the shadows cast by the model?    |

In the `[person]` section all the properties for `[model]` section are valid, but there are also few more parameters:

| **Property `[values]`** | **Description**                                       |
|:------------------------|:------------------------------------------------------|
| `animation [path start_time end_time]` | animation that will be played by the model |
| `style [text path]`   | name and path to the file that defines fighting style that may be used by this model (multiple style definitions creates list of available styles) |
| `control [name]`      | method of controlling the model                     |
|                       | camera – player is controlled via cameras           |
|                       | network - player is controlled via network (future) |
|                       | comboard - player is controlled with keyboard       |
|                       | ai - player is controlled by AI                     |
| `enemy [int]`         | player at which this person will concentrate        |