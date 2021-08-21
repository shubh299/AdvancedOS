# File Explorer
To run -
1. Compile main.cpp using g++. Run command - `g++ main.cpp -o file_explorer`
2. Run the executable as - `./file_explorer`

There are 2 modes -
* Normal Mode - Simple navigation using arrow keys and display on terminal.
* Command Mode - Using commands to navigate through file system and perform operations.

The following commands/input are supported -
* In normal mode -
    * `Up and down arrow keys` for navigation in display list.
    * `k` and `l` keys for scrolling up and down respectively.
    * `Enter` to go inside a directory or open a file.
    * `Left and right arrow keys` to go back and forward in visited directories.
    * `Backspace` to go one level up in direcory.
    * `h` key to go to home directory, i.e., directory where the program was executed.
    * `:` to go to command mode.
* In command mode - All commands appear in status bar at bottom.
    * `copy <source_file(s)> <destination_directory>`
    * `move <source_file(s)> <destination_directory>`
    * `rename <old_filename> <new_filename>`
    * `create_file <file_name> <destination_path>`
    * `create_dir <dir_name> <destination_path>`
    * `delete_file <file_path>`
    * `delete_dir <directory_path>`
    * `goto <location>`
    * `search <file_name>`
    * `search <directory_name>`
    * Press `Esc` key to exit command mode and return to normal mode.