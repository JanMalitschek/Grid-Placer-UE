# GridPlacer UE
A Plugin designed to alleviate the pain of creating grid-based maps by hand - ported from Unity to Unreal Engine.

This is an overhauled version of the original, which was made for Unity3D and can be found here: https://github.com/JanMalitschek/Grid-Placer.
It offers pretty much the same feature set apart from some changes and improvements here and there.

Despite its name, this is a pretty powerful tool for populating your level with objects in general, so you're not necessarily limited to working on a grid.

# Installation
To install this plugin simply clone the repo and put it into your projects **Plugins** directory (You might have to create that in case it doesn't exist - it goes along side the **Content** directory right into the project root).
Then inside the engine navigate to **Edit > Plugins**, search for **GridPlacer** and enable it (an engine restart might be necessary). **GridPlacer** is now installed.

**Note** that this plugin was made for Unreal Engine 5.3 but it should work perfectly fine with earlier versions of 5 and probably even 4, but I cannot confirm that.

# Usage
**GridPlacer** comes in the form of a new **Editing Mode** which can be selected from a drop down at the top left of the UE editor window (says **Selection Mode** by default).
Now you will be presented with a pretty much empty window with a tool tray at the top. Click the only available tool and the **GridPlacer** interface will appear.

There is a lot of settings but fret not, every single one has a tooltip explaining it's functionality if it's not obvious already.

To start placing, simply drag a **StaticMesh** or a **Blueprint**, that at some point derives from **Actor**, from the **ContentBrowser** into the **ObjectPalette** and tick the little checkmark on each item you want to place.