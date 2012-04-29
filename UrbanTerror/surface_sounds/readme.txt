Any recent q3map2 can do the custom surfaceparms without using an external utility.

Put the attached custinfoparms.txt file into your scripts directory (same directory as your shaderlist.txt or .shader files will work.) Add the switch -custinfoparms to your BSP compiling command. You're good to go!

For example in GtkRadiant: 
Go to File->Project Settings.
Pick the compile command you normally use and click 'Change'
Copy and paste the 'Menu text' and 'Command' text into a text editor.
Click Cancel.

Click 'Add...'
Paste the 'Menu text' into the text box. Add '-custinfoparms'
bsp_Q3Map2: (test) BSP -meta, -vis, -light -fast -filter  ----> bsp_Q3Map2: (test) BSP -meta, -vis, -light -fast -filter -custinfoparms
This is just so you know this is the right command.

Paste the 'Command' into the text box.
Add '-custinfoparms' to each 'stage' of the compile command, like so:
! "/home/jeff/Apps/GtkRadiant/install/q3map2" -v # -game quake3 -fs_basepath "/home/jeff/Apps/Games/UrbanTerror/" -meta -custinfoparms $ && ! "/home/jeff/Apps/GtkRadiant/install/q3map2" # -game quake3 -fs_basepath "/home/jeff/Apps/Games/UrbanTerror/" -custinfoparms -vis -saveprt $ && ! "/home/jeff/Apps/GtkRadiant/install/q3map2" -v # -game quake3 -fs_basepath "/home/jeff/Apps/Games/UrbanTerror/" -light -fast -filter -custinfoparms  $

Now hit 'Ok' and compile with this command like normal. Ta-da! Custom surfaceparms without the extra, external step.

Never again will you forget to include the custom stepping sounds for your textures!



----


QuaKe[1up] asks: "How do I set the sound for each texture?"

I was thinking I should've included that...
Short answer:
Include a 'surfaceparm TYPE' in the shader for the texture, where TYPE is one of the types in custinfoparms.txt; ie., brick, rock, tin, steel, ... 'surfaceparm brick' (see example shader below.)

Long answer:
[If you already know about shaders, the short answer should suffice. I'm going to be pretty verbose here for anyone that isn't familiar with them or thought they were only used for really complicated special effects.]

If you haven't already, create a file called ut4_MAPNAME.shader; ie., ut4_mario1.shader
Place that file in the ./scripts/ directory of your UrT install (and in the ./scripts/ directory of your pk3, when the time comes.)

The .shader file is a text file that uses a simple language to define 'shaders' for textures in the Q3 engine.
Here is a simple example shader for a texture with no transparency that is affected by the lightmap:
Parts that begin with // are comments, just for illustration, and should not be included in the real file.

textures/mario_blocks/brick_brick      // The name of the texture as it will be referred to in Radiant, no extension
{
    qer_editorimage textures/mario_blocks/brick_brick.tga      // The path to the image that will represent this shader in Radiant
    surfaceparm brick    // A surface parameter: This specifies that this surface will be treated as brick by UrT
    {
        map textures/mario_blocks/brick_brick.tga     // The actual texture that will be drawn for this texture-shader combo
        rgbGen identity     // Don't worry about this, or go read http://toolz.nexuizninjaz.com/shader/
    }
    {
        map $lightmap     // These three lines mean 'apply the lightmap to this texture'
        rgbGen identity
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

So create your .shader file and copy this shader into it. Change the paths to point to your texture, change the surfaceparm to specify the material you want, and recompile your map with the -custinfoparms switch as I showed above. You'll need a shader for every texture you want to apply a custom surface sound to.

Unfortunately the shaders used by the stock UrT textures don't include the necessary surfaceparm as far as I can tell. This means they won't have custom sounds unless you modify their shaders. The good news is you can add the custom sound surface parm to the stock shaders, build your .bsp, and you don't have to include the modified stock shader files because the custom sounds are applied to the .bsp.

In the case of your own textures/shaders you do have to include the .shader file in your pk3.

In case you're wondering why you would want to do it this way:
1) It keeps all the information defining how a texture looks and acts in one place.
2) It doesn't require using a separate utility outside of q3map2 which can be annoying for a whole bunch of reasons.
3) It can be made a part of your bsp compilation process and doesn't require any additional outside steps. I know the 'old way' adding surface sounds was a 'polish' step that I took only when I was ready to package the final product. Forgetting sucks and it's one more thing to test during that final stage (did you set all the textures to the sounds you wanted? etc.).
4) It is a simple way to get introduced to shaders which you'll want for transparency and more advanced effects as you 'level up' in mapping.
