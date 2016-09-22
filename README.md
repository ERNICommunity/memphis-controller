[![Stories in Ready](https://badge.waffle.io/ERNICommunity/memphis-controller.png?label=ready&title=Ready)](https://waffle.io/ERNICommunity/memphis-controller)
# memphis-controller
Arkathon Project Memphis Controller Software

## How to build
  1. Create a directory where your Eclipse Workspace will be stored and where this project shall be cloned into. E.g. `C:\git\pio-prj`
  2. Clone this repository recursively into the folder you created before, `git clone --recursive git@github.com:ERNICommunity/memphis-controller.git`
  3. Open a command shell in the just cloned project folder, i.e in `C:\git\pio-prj\memphis-controller`
  4. Initialize as a PlatformIO project for the Adafrui ESP6288 Huzzah to be imported into Eclipse: run the command `pio init -b huzzah --ide eclipse`
  5. Build the project: run the command `pio run`
  6. Clean the project: run the command `pio run -t clean`
  7. Dowload to the target board (i.e. on COM2): run the command `pio run -t upload --upload-port COM2`

## Open project in Eclipse CDT
  1. Open Eclipse CDT, as workspace choose the folder you created before, i.e `C:\git\pio-prj`
  2. Import the project with File->Import->General->Existing Projects into Workspace, choose the `memphis-controller` (i.e `C:\git\pio-prj\memphis-controller`)
